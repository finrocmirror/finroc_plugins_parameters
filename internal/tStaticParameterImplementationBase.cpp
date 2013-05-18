//
// You received this file as part of Finroc
// A Framework for intelligent robot control
//
// Copyright (C) Finroc GbR (finroc.org)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//----------------------------------------------------------------------
/*!\file    plugins/parameters/internal/tStaticParameterImplementationBase.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-30
 *
 */
//----------------------------------------------------------------------
#include "plugins/parameters/internal/tStaticParameterImplementationBase.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "core/tRuntimeEnvironment.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/parameters/tConfigFile.h"
#include "plugins/parameters/tConfigNode.h"
#include "plugins/parameters/internal/tStaticParameterList.h"
#include "plugins/parameters/internal/tParameterInfo.h"

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

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

tStaticParameterImplementationBase::tStaticParameterImplementationBase(const std::string& name, rrlib::rtti::tType type, bool constructor_prototype, bool static_parameter_proxy, const std::string& config_entry) :
  name(name),
  type(type),
  value(),
  last_value(),
  enforce_current_value(false),
  use_value_of(this),
  parent_list(NULL),
  list_index(0),
  command_line_option(),
  outer_parameter_attachment(),
  create_outer_parameter(false),
  config_entry(config_entry),
  config_entry_set_by_finstruct(false),
  static_parameter_proxy(static_parameter_proxy),
  attached_parameters()
{
  if (!constructor_prototype)
  {
    CreateBuffer(type);
  }
}

tStaticParameterImplementationBase::~tStaticParameterImplementationBase()
{
}

void tStaticParameterImplementationBase::AttachTo(tStaticParameterImplementationBase* other)
{
  if (use_value_of != this)
  {
    auto& vec = use_value_of->attached_parameters;
    vec.erase(std::remove(vec.begin(), vec.end(), this), vec.end());
  }
  use_value_of = other == NULL ? this : other;
  if (use_value_of != this)
  {
    use_value_of->attached_parameters.push_back(this);
  }

  tStaticParameterImplementationBase& sp = GetParameterWithBuffer();
  if (sp.type == NULL)
  {
    sp.type = type;
  }
  if (!sp.value)
  {
    CreateBuffer(sp.type);

    if (&sp != this)
    {
      // Swap buffers to have something sensible in it
      std::swap(value, sp.value);
    }
  }
}

void tStaticParameterImplementationBase::CreateBuffer(rrlib::rtti::tType type)
{
  tStaticParameterImplementationBase& sp = GetParameterWithBuffer();
  sp.value.reset(type.CreateInstanceGeneric());
  assert(sp.value);
  assert(type.GetRttiName() != typeid(tStaticParameterList).name());
}

void tStaticParameterImplementationBase::Deserialize(rrlib::serialization::tInputStream& is)
{
  // Skip name and parameter type
  is.ReadString();
  rrlib::rtti::tType dt;
  is >> dt;

  std::string command_line_option_tmp = is.ReadString();
  outer_parameter_attachment = is.ReadString();
  create_outer_parameter = is.ReadBoolean();
  std::string config_entry_tmp = is.ReadString();
  config_entry_set_by_finstruct = is.ReadBoolean();
  enforce_current_value = is.ReadBoolean();
  UpdateOuterParameterAttachment();
  UpdateAndPossiblyLoad(command_line_option_tmp, config_entry_tmp);

  try
  {
    DeserializeValue(is);
  }
  catch (const std::exception& e)
  {
    FINROC_LOG_PRINT(ERROR, e);
  }
}

void tStaticParameterImplementationBase::Deserialize(const rrlib::xml::tNode& node, bool finstruct_context)
{
  rrlib::rtti::tType dt = type;
  if (node.HasAttribute("type"))
  {
    dt = rrlib::rtti::tType::FindType(node.GetStringAttribute("type"));
  }
  enforce_current_value = node.HasAttribute("enforcevalue") && node.GetBoolAttribute("enforcevalue");
  rrlib::rtti::tGenericObject* val = ValuePointer();
  if (val == NULL || val->GetType() != dt)
  {
    CreateBuffer(dt);
    val = ValuePointer();
  }
  val->Deserialize(node);

  std::string command_line_option_tmp;
  if (node.HasAttribute("cmdline"))
  {
    command_line_option_tmp = node.GetStringAttribute("cmdline");
  }
  else
  {
    command_line_option_tmp = "";
  }
  if (node.HasAttribute("attachouter"))
  {
    outer_parameter_attachment = node.GetStringAttribute("attachouter");
    UpdateOuterParameterAttachment();
  }
  else
  {
    outer_parameter_attachment = "";
    UpdateOuterParameterAttachment();
  }
  std::string config_entry_tmp;
  if (node.HasAttribute("config"))
  {
    config_entry_tmp = node.GetStringAttribute("config");
    config_entry_set_by_finstruct = finstruct_context;
  }
  else
  {
    config_entry_tmp = "";
  }

  UpdateAndPossiblyLoad(command_line_option_tmp, config_entry_tmp);
}

void tStaticParameterImplementationBase::DeserializeValue(rrlib::serialization::tInputStream& is)
{
  if (is.ReadBoolean())
  {
    rrlib::rtti::tType dt;
    is >> dt;
    rrlib::rtti::tGenericObject* val = ValuePointer();
    if (val->GetType() != dt)
    {
      CreateBuffer(dt);
      val = ValuePointer();
    }
    rrlib::serialization::Deserialize(is, *val, rrlib::serialization::tDataEncoding::XML);
  }
}

void tStaticParameterImplementationBase::GetAllAttachedParameters(std::vector<tStaticParameterImplementationBase*>& result)
{
  result.clear();
  result.push_back(this);

  for (size_t i = 0; i < result.size(); i++)
  {
    tStaticParameterImplementationBase* param = result[i];
    if (param->use_value_of != NULL && param->use_value_of != this && (/* result does not contain param */std::find(result.begin(), result.end(), param) == result.end()))
    {
      result.push_back(param->use_value_of);
    }
    for (size_t j = 0; j < param->attached_parameters.size(); j++)
    {
      tStaticParameterImplementationBase* at = param->attached_parameters[j];
      if (at != this && (/* result does not contain param */std::find(result.begin(), result.end(), at) == result.end()))
      {
        result.push_back(at);
      }
    }
  }
}

bool tStaticParameterImplementationBase::HasChanged()
{
  tStaticParameterImplementationBase& sp = GetParameterWithBuffer();
  if (sp.value.get() == last_value.get())
  {
    return false;
  }
  if ((!sp.value) || (!last_value))
  {
    return true;
  }
  return !sp.value->Equals(*last_value);
}

void tStaticParameterImplementationBase::LoadValue()
{
  core::tFrameworkElement* parent = parent_list->GetAnnotated();

  if (use_value_of == this && (!enforce_current_value))
  {
    // command line
    core::tFrameworkElement* fg = parent->GetParentWithFlags(core::tFrameworkElement::tFlag::FINSTRUCTABLE_GROUP);
    if (command_line_option.length() > 0 && (fg == NULL || fg->GetParent() == &core::tRuntimeEnvironment::GetInstance()))
    {
      // outermost group?
      std::string arg = core::tRuntimeEnvironment::GetInstance().GetCommandLineArgument(command_line_option);
      if (arg.length() > 0)
      {
        try
        {
          Set(arg);
          return;
        }
        catch (std::exception& e)
        {
          FINROC_LOG_PRINT(ERROR, "Failed to load parameter '", GetName(), "' from command line argument '", arg, "': ", e);
        }
      }
    }

    // config entry
    if (config_entry.length() > 0)
    {
      if (config_entry_set_by_finstruct)
      {
        if (fg == NULL || (!tParameterInfo::IsFinstructableGroupResponsibleForConfigFileConnections(*fg, *parent)))
        {
          return;
        }
      }
      tConfigFile* cf = tConfigFile::Find(*parent);
      std::string full_config_entry = tConfigNode::GetFullConfigEntry(*parent, config_entry);
      if (cf != NULL)
      {
        if (cf->HasEntry(full_config_entry))
        {
          rrlib::xml::tNode& node = cf->GetEntry(full_config_entry, false);
          try
          {
            value->Deserialize(node);
          }
          catch (std::exception& e)
          {
            FINROC_LOG_PRINT(ERROR, "Failed to load parameter '", GetName(), "' from config entry '", full_config_entry, "': ", e);
          }
        }
      }
    }
  }
}

void tStaticParameterImplementationBase::ResetChanged()
{
  tStaticParameterImplementationBase& sp = GetParameterWithBuffer();

  assert(sp.value);
  if ((!last_value) || last_value->GetType() != sp.value->GetType())
  {
    last_value.reset(sp.value->GetType().CreateInstanceGeneric());
  }
  assert(last_value);

  FINROC_LOG_PRINT(DEBUG_VERBOSE_2, "Resetting change for buffers of type ", sp.value->GetType().GetName());
  //rrlib::serialization::sSerialization::DeepCopy(*sp->value, *last_value);
  last_value->DeepCopyFrom(*sp.value.get());
  assert(!HasChanged());
}

void tStaticParameterImplementationBase::Serialize(rrlib::serialization::tOutputStream& os) const
{
  os.WriteString(name);
  os << type;
  os.WriteString(command_line_option);
  os.WriteString(outer_parameter_attachment);
  os.WriteBoolean(create_outer_parameter);
  os.WriteString(config_entry);
  os.WriteBoolean(config_entry_set_by_finstruct);
  os.WriteBoolean(enforce_current_value);

  rrlib::rtti::tGenericObject* val = ValuePointer();
  os.WriteBoolean(val != NULL);
  if (val != NULL)
  {
    os << val->GetType();
    rrlib::serialization::Serialize(os, *val, rrlib::serialization::tDataEncoding::XML);
  }
}

void tStaticParameterImplementationBase::Serialize(rrlib::xml::tNode& node, bool finstruct_context) const
{
  assert(!(node.HasAttribute("type") || node.HasAttribute("cmdline") || node.HasAttribute("config") || node.HasAttribute("attachouter")));
  rrlib::rtti::tGenericObject* val = ValuePointer();
  if (val->GetType() != type || static_parameter_proxy)
  {
    node.SetAttribute("type", val->GetType().GetName());
  }
  if (enforce_current_value)
  {
    node.SetAttribute("enforcevalue", true);
  }
  val->Serialize(node);

  if (command_line_option.length() > 0)
  {
    node.SetAttribute("cmdline", command_line_option);
  }
  if (outer_parameter_attachment.length() > 0)
  {
    node.SetAttribute("attachouter", outer_parameter_attachment);
  }
  if (config_entry.length() > 0 && (config_entry_set_by_finstruct || (!finstruct_context)))
  {
    node.SetAttribute("config", config_entry);
  }
}

void tStaticParameterImplementationBase::Set(const std::string& s)
{
  assert(type != NULL);
  //rrlib::rtti::tType dt = sSerializationHelper::GetTypedStringDataType(type, s);
  rrlib::rtti::tGenericObject* val = ValuePointer();
  if (val->GetType() != type)
  {
    CreateBuffer(type);
    val = ValuePointer();
  }

  rrlib::serialization::tStringInputStream sis(s);
  val->Deserialize(sis);
}

void tStaticParameterImplementationBase::SetConfigEntry(const std::string& config_entry)
{
  config_entry_set_by_finstruct = false;
  if (config_entry.compare(this->config_entry) != 0)
  {
    this->config_entry = config_entry;
    if (GetParentList() && GetParentList()->GetAnnotated() && GetParentList()->GetAnnotated()->IsReady())
    {
      LoadValue();
    }
  }
}

void tStaticParameterImplementationBase::SetOuterParameterAttachment(const std::string& outer_parameter_attachment, bool create_outer)
{
  this->outer_parameter_attachment = outer_parameter_attachment;
  create_outer_parameter = create_outer;

  UpdateOuterParameterAttachment();
}

void tStaticParameterImplementationBase::UpdateAndPossiblyLoad(const std::string& command_line_option_tmp, const std::string& config_entry_tmp)
{
  bool cmdline_changed = command_line_option.compare(command_line_option_tmp) != 0;
  bool config_entry_changed = config_entry.compare(config_entry_tmp) != 0;
  command_line_option = command_line_option_tmp;
  config_entry = config_entry_tmp;

  if (use_value_of == this && (cmdline_changed || config_entry_changed))
  {
    LoadValue();
  }
}

void tStaticParameterImplementationBase::UpdateOuterParameterAttachment()
{
  if (parent_list == NULL)
  {
    return;
  }
  if (outer_parameter_attachment.length() == 0)
  {
    if (use_value_of != this)
    {
      AttachTo(this);
    }
  }
  else
  {
    tStaticParameterImplementationBase* sp = &GetParameterWithBuffer();
    bool name_differs = sp->GetName().compare(outer_parameter_attachment) != 0;
    if (name_differs || (sp == this))
    {

      // find parameter to attach to
      core::tFrameworkElement* fg = parent_list->GetAnnotated()->GetParentWithFlags(core::tFrameworkElement::tFlag::FINSTRUCTABLE_GROUP);
      if (fg == NULL)
      {
        FINROC_LOG_PRINT(ERROR, "No parent finstructable group. Ignoring...");
        return;
      }

      tStaticParameterList& spl = tStaticParameterList::GetOrCreate(*fg);
      for (size_t i = 0; i < spl.Size(); i++)
      {
        sp = &spl.Get(i);
        if (sp->GetName().compare(outer_parameter_attachment) == 0) // equals?
        {
          AttachTo(sp);
          return;
        }
      }

      if (create_outer_parameter)
      {
        sp = new tStaticParameterImplementationBase(outer_parameter_attachment, type, false, true);
        AttachTo(sp);
        spl.Add(*sp);
        FINROC_LOG_PRINT(DEBUG, "Creating proxy parameter '", outer_parameter_attachment, "' in '", fg->GetQualifiedName() + "'.");
      }
      else
      {
        FINROC_LOG_PRINT(ERROR, "No parameter named '", outer_parameter_attachment, "' found in parent group.");
      }
    }
  }
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}
