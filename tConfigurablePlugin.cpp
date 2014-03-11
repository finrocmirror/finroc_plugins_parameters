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

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/parameters/tConfigFile.h"
#include "plugins/parameters/tConfigNode.h"

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
tConfigurablePlugin::tConfigurablePlugin() :
  initialized(false),
  elements_to_create(),
  parameter_element(nullptr)
{}

core::tFrameworkElement& tConfigurablePlugin::GetParameterElement()
{
  if (!parameter_element)
  {
    parameter_element = new core::tFrameworkElement(&core::tRuntimeEnvironment::GetInstance().GetElement(core::tSpecialRuntimeElement::SETTINGS), GetId());
    tConfigNode::SetConfigNode(*parameter_element, std::string("/Runtime/Plugins/") + GetId());
    parameter_element->Init();
  }
  return *parameter_element;
}

void tConfigurablePlugin::Init()
{
  // Create parameters
  for (auto it = elements_to_create.begin(); it != elements_to_create.end(); ++it)
  {
    it->first->CreateFinrocElement(*it->second);
  }
  elements_to_create.clear();
  elements_to_create.shrink_to_fit(); // vector will not be used anymore

  // Init parameters
  LoadParameterValues();
  initialized = true;

  // Call Init
  tConfigFile* config_file = tConfigFile::Find(GetParameterElement());
  if (config_file)
  {
    std::string config_entry = tConfigNode::GetConfigNode(*parameter_element);
    if (config_file->HasEntry(config_entry))
    {
      Init(&config_file->GetEntry(config_entry));
    }
    else
    {
      Init(nullptr);
    }
  }
}

void tConfigurablePlugin::LoadParameterValues()
{
  GetParameterElement().Init();
  internal::tStaticParameterList::DoStaticParameterEvaluation(GetParameterElement());
}



//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
