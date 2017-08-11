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
/*!\file    plugins/parameters/internal/tParameterInfo.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-28
 *
 */
//----------------------------------------------------------------------
#include "plugins/parameters/internal/tParameterInfo.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/rtti/rtti.h"
#include "core/port/tAbstractPort.h"
#include "core/tRuntimeEnvironment.h"
#include "plugins/data_ports/tGenericPort.h"

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
namespace internal
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------

/* Initializes parameter info annotation type */
static rrlib::rtti::tDataType<tParameterInfo> cTYPE;

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

tParameterInfo::tParameterInfo() :
  config_entry(),
  entry_set_from_finstruct(false),
  command_line_option(),
  finstruct_default()
{}

#ifdef _LIB_RRLIB_XML_PRESENT_
void tParameterInfo::Deserialize(const rrlib::xml::tNode& node, bool finstruct_context, bool include_commmand_line)
{
  if (node.HasAttribute("config"))
  {
    config_entry = node.GetStringAttribute("config");
    entry_set_from_finstruct = finstruct_context;
  }
  else
  {
    config_entry = "";
  }
  if (include_commmand_line)
  {
    if (node.HasAttribute("cmdline"))
    {
      command_line_option = node.GetStringAttribute("cmdline");
    }
    else
    {
      command_line_option = "";
    }
  }
  if (node.HasAttribute("default"))
  {
    finstruct_default = node.GetStringAttribute("default");
  }
  else
  {
    finstruct_default = "";
  }
}
#endif

bool tParameterInfo::IsFinstructableGroupResponsibleForConfigFileConnections(const core::tFrameworkElement& finstructable_group, const core::tFrameworkElement& ap)
{
  tConfigFile* cf = tConfigFile::Find(ap);
  if (cf == NULL)
  {
    return finstructable_group.GetParentWithFlags(core::tFrameworkElement::tFlag::FINSTRUCTABLE_GROUP) == NULL;
  }
  core::tFrameworkElement* config_element = cf->GetAnnotated<core::tFrameworkElement>();
  const core::tFrameworkElement* responsible = config_element->GetFlag(core::tFrameworkElement::tFlag::FINSTRUCTABLE_GROUP) ?
      config_element : config_element->GetParentWithFlags(core::tFrameworkElement::tFlag::FINSTRUCTABLE_GROUP);
  if (!responsible)
  {
    // ok, config file is probably attached to runtime. Choose outer-most finstructable group.
    responsible = &finstructable_group;
    const core::tFrameworkElement* tmp;
    while ((tmp = responsible->GetParentWithFlags(core::tFrameworkElement::tFlag::FINSTRUCTABLE_GROUP)) != NULL)
    {
      responsible = tmp;
    }
  }
  return responsible == &finstructable_group;
}

void tParameterInfo::LoadValue(bool ignore_ready)
{
  core::tAbstractPort* ann = this->GetAnnotated<core::tAbstractPort>();
  {
    rrlib::thread::tLock lock(ann->GetStructureMutex());
    if (ann && (ignore_ready || ann->IsReady()))
    {
      // command line option
      if (command_line_option.length() > 0)
      {
        std::string arg = core::tRuntimeEnvironment::GetInstance().GetCommandLineArgument(command_line_option);
        if (arg.length() > 0)
        {
          if (data_ports::IsDataFlowType(ann->GetDataType()))
          {
            rrlib::serialization::tStringInputStream sis(arg);
            data_ports::tGenericPort port = data_ports::tGenericPort::Wrap(*ann);
            data_ports::tPortDataPointer<rrlib::rtti::tGenericObject> buffer = port.GetUnusedBuffer();
            try
            {
              buffer->Deserialize(sis);
              std::string error = port.BrowserPublish(buffer);
              if (error.size() > 0)
              {
                FINROC_LOG_PRINT(WARNING, "Failed to load parameter '", ann, "' from command line argument '", arg, "': ", error);
              }
              return;
            }
            catch (const std::exception& e)
            {
              FINROC_LOG_PRINT(ERROR, "Failed to load parameter '", ann, "' from command line argument '", arg, "': ", e);
            }
          }
          else
          {
            throw std::runtime_error("Port Type not supported as a parameter");
          }
        }
      }

      // config file entry
      tConfigFile* cf = tConfigFile::Find(*ann);
      if (cf != NULL && config_entry.length() > 0)
      {
        std::string full_config_entry = tConfigNode::GetFullConfigEntry(*ann, config_entry);
        if (cf->HasEntry(full_config_entry))
        {
#ifdef _LIB_RRLIB_XML_PRESENT_
          rrlib::xml::tNode& node = cf->GetEntry(full_config_entry, false);
          if (data_ports::IsDataFlowType(ann->GetDataType()))
          {
            data_ports::tGenericPort port = data_ports::tGenericPort::Wrap(*ann);
            data_ports::tPortDataPointer<rrlib::rtti::tGenericObject> buffer = port.GetUnusedBuffer();

            try
            {
              buffer->Deserialize(node);
              std::string error = port.BrowserPublish(buffer);
              if (error.size() > 0)
              {
                FINROC_LOG_PRINT(WARNING, "Failed to load parameter '", ann, "' from config entry '", full_config_entry, "': ", error);
              }
              return;
            }
            catch (const std::exception& e)
            {
              FINROC_LOG_PRINT(ERROR, "Failed to load parameter '", ann, "' from config entry '", full_config_entry, "': ", e);
            }
          }
          else
          {
            throw std::runtime_error("Port Type not supported as a parameter");
          }
#endif
        }
      }

      // finstruct default
      if (finstruct_default.length() > 0)
      {
        if (data_ports::IsDataFlowType(ann->GetDataType()))
        {
          rrlib::serialization::tStringInputStream sis(finstruct_default);
          data_ports::tGenericPort port = data_ports::tGenericPort::Wrap(*ann);
          data_ports::tPortDataPointer<rrlib::rtti::tGenericObject> buffer = port.GetUnusedBuffer();

          try
          {
            buffer->Deserialize(sis);
            std::string error = port.BrowserPublish(buffer);
            if (error.size() > 0)
            {
              FINROC_LOG_PRINT(WARNING, "Failed to load parameter '", ann, "' from finstruct default '", finstruct_default, "': ", error);
            }
            return;
          }
          catch (const std::exception& e)
          {
            FINROC_LOG_PRINT(ERROR, "Failed to load parameter '", ann, "' from finstruct default '", finstruct_default, "': ", e);
          }
        }
        else
        {
          throw std::runtime_error("Port Type not supported as a parameter");
        }
      }
    }
  }
}

void tParameterInfo::OnInitialization()
{
  try
  {
    LoadValue(true);
  }
  catch (const std::exception& e)
  {
    FINROC_LOG_PRINT(ERROR, e);
  }
}

void tParameterInfo::SaveValue()
{
  core::tAbstractPort* ann = GetAnnotated<core::tAbstractPort>();
  if (ann == NULL || (!ann->IsReady()))
  {
    return;
  }
  tConfigFile* cf = tConfigFile::Find(*ann);
  bool has_entry = cf->HasEntry(config_entry);
  if (data_ports::IsDataFlowType(ann->GetDataType()))
  {
    data_ports::tGenericPort port = data_ports::tGenericPort::Wrap(*ann);
    if (has_entry)
    {
      // Does port contain default value?
      const rrlib::rtti::tGenericObject* default_value = port.GetDefaultValue();
      bool is_default = false;
      if (default_value)
      {
        std::unique_ptr<rrlib::rtti::tGenericObject> current_value(default_value->GetType().CreateGenericObject());
        port.Get(*current_value);
        if (current_value->Equals(*default_value))
        {
          is_default = true;
        }
      }

      if (!is_default)
      {
#ifdef _LIB_RRLIB_XML_PRESENT_
        rrlib::xml::tNode& node = cf->GetEntry(config_entry, true);
        std::unique_ptr<rrlib::rtti::tGenericObject> current_value(port.GetDataType().CreateGenericObject());
        port.Get(*current_value);
        current_value->Serialize(node);
#endif
      }
    }
  }
  else
  {
    throw std::runtime_error("Port Type not supported as a parameter");
  }
}

#ifdef _LIB_RRLIB_XML_PRESENT_
void tParameterInfo::Serialize(rrlib::xml::tNode& node, bool finstruct_context, bool include_command_line) const
{
  assert(!(node.HasAttribute("default") || node.HasAttribute("cmdline") || node.HasAttribute("config")));
  if (config_entry.length() > 0 && (entry_set_from_finstruct || (!finstruct_context)))
  {
    node.SetAttribute("config", config_entry);
  }
  if (include_command_line)
  {
    if (command_line_option.length() > 0)
    {
      node.SetAttribute("cmdline", command_line_option);
    }
  }
  if (finstruct_default.length() > 0)
  {
    node.SetAttribute("default", finstruct_default);
  }
}
#endif

void tParameterInfo::SetConfigEntry(const std::string& config_entry, bool finstruct_set)
{
  if (this->config_entry.compare(config_entry) != 0)
  {
    this->config_entry = config_entry;
    this->entry_set_from_finstruct = finstruct_set;
    try
    {
      LoadValue();
    }
    catch (const std::exception& e)
    {
      FINROC_LOG_PRINT(ERROR, e);
    }
  }
}


rrlib::serialization::tOutputStream& operator << (rrlib::serialization::tOutputStream& stream, const tParameterInfo& parameter_info)
{
  stream.WriteBoolean(parameter_info.IsConfigEntrySetFromFinstruct());
  stream.WriteString(parameter_info.GetConfigEntry());
  stream.WriteString(parameter_info.GetCommandLineOption());
  stream.WriteString(parameter_info.GetFinstructDefault());
  return stream;
}

rrlib::serialization::tInputStream& operator >> (rrlib::serialization::tInputStream& stream, tParameterInfo& parameter_info)
{
  parameter_info.entry_set_from_finstruct = stream.ReadBoolean();
  std::string config_entry_tmp = stream.ReadString();
  std::string command_line_option_tmp = stream.ReadString();
  std::string finstruct_default_tmp = stream.ReadString();
  bool same = config_entry_tmp.compare(parameter_info.GetConfigEntry()) == 0 &&
              command_line_option_tmp.compare(parameter_info.GetCommandLineOption()) == 0 &&
              finstruct_default_tmp.compare(parameter_info.GetFinstructDefault()) == 0;
  parameter_info.config_entry = config_entry_tmp;
  parameter_info.command_line_option = command_line_option_tmp;
  parameter_info.finstruct_default = finstruct_default_tmp;

  if (!same)
  {
    try
    {
      parameter_info.LoadValue();
    }
    catch (std::exception& e)
    {
      FINROC_LOG_PRINT_STATIC(ERROR, e);
    }
  }
  return stream;
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}
