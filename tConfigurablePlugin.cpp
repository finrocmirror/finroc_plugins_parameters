//
// You received this file as part of Finroc
// A framework for intelligent robot control
//
// Copyright (C) Finroc GbR (finroc.org)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
//----------------------------------------------------------------------
/*!\file    plugins/parameters/tConfigurablePlugin.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2014-03-11
 *
 */
//----------------------------------------------------------------------
#include "plugins/parameters/tConfigurablePlugin.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "core/tRuntimeEnvironment.h"
#include "core/file_lookup.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Debugging
//----------------------------------------------------------------------
#include <cassert>

//----------------------------------------------------------------------
// Namespace usage
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Namespace declaration
//----------------------------------------------------------------------
namespace finroc
{
namespace parameters
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

/*! Stores config file name if set */
static std::string config_file_name;

static bool first_plugin_initialized = false;

tConfigurablePlugin::tConfigurablePlugin(const char* name) :
  tPlugin(name),
  initialized(false),
  elements_to_create(),
  parameter_element(nullptr)
{}

#ifdef _LIB_RRLIB_XML_PRESENT_
rrlib::xml::tNode* tConfigurablePlugin::GetConfigRootNode()
{
  /*! Pointer to config file if one was set and found - lazily initialized with GetConfigFile() */
  static std::unique_ptr<rrlib::xml::tDocument> config_file;
  static rrlib::xml::tNode* root_node = nullptr;

  // load config file?
  if (config_file_name.length())
  {
    if (!core::FinrocFileExists(config_file_name))
    {
      FINROC_LOG_PRINT(WARNING, "Configuration file '", config_file_name, "' does not exist. Plugins are initialized with defaults. No additional plugins are loaded.");
    }
    else
    {
      config_file.reset(new rrlib::xml::tDocument(core::GetFinrocFile(config_file_name), false));
      for (auto it = config_file->RootNode().ChildrenBegin(); it != config_file->RootNode().ChildrenEnd(); ++it)
      {
        if (it->Name() == "runtime")
        {
          root_node = &(*it);
          break;
        }
      }
    }
    config_file_name = "";
  }

  return root_node;
}
#endif

core::tFrameworkElement& tConfigurablePlugin::GetParameterElement()
{
  if (!parameter_element)
  {
    parameter_element = new core::tFrameworkElement(&core::tRuntimeEnvironment::GetInstance().GetElement(core::tSpecialRuntimeElement::SETTINGS), GetName());
    parameter_element->Init();
  }
  return *parameter_element;
}

#ifdef _LIB_RRLIB_XML_PRESENT_
rrlib::xml::tNode* tConfigurablePlugin::GetParameterNode(const std::string& config_entry)
{
  rrlib::xml::tNode* plugin_node = GetPluginConfigNode();
  if (plugin_node)
  {
    for (auto it = plugin_node->ChildrenBegin(); it != plugin_node->ChildrenEnd(); ++it)
    {
      try
      {
        if (it->Name() == "value" && it->GetStringAttribute("name") == config_entry)
        {
          return &(*it);
        }
      }
      catch (const rrlib::xml::tException& e) {}
    }
  }
  return nullptr;
}

rrlib::xml::tNode* tConfigurablePlugin::GetPluginConfigNode()
{
  rrlib::xml::tNode* root_node = GetConfigRootNode();
  if (root_node)
  {
    for (auto it = root_node->ChildrenBegin(); it != root_node->ChildrenEnd(); ++it)
    {
      try
      {
        if (it->Name() == "plugin" && it->GetStringAttribute("name") == GetName())
        {
          return &(*it);
        }
      }
      catch (const rrlib::xml::tException& e) {}
    }
  }
  return nullptr;
}

#endif

void tConfigurablePlugin::Init()
{
  // Create parameters
  for (auto it = elements_to_create.begin(); it != elements_to_create.end(); ++it)
  {
    (*it)->CreateFinrocElement();
  }
  elements_to_create.clear();
  elements_to_create.shrink_to_fit(); // vector will not be used anymore

  // Init parameters
  GetParameterElement();
  initialized = true;
  first_plugin_initialized = true;

  // Call Init
#ifdef _LIB_RRLIB_XML_PRESENT_
  Init(GetPluginConfigNode());
#endif
}

void tConfigurablePlugin::SetConfigFile(const std::string& file_name)
{
  if (first_plugin_initialized)
  {
    FINROC_LOG_PRINT(WARNING, "Some configurable plugins were already initialized. Setting a config file now has no effect on them.");
  }
  config_file_name = file_name;
}



//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
