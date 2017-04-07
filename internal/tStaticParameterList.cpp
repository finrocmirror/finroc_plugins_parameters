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
/*!\file    plugins/parameters/internal/tStaticParameterList.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-30
 *
 */
//----------------------------------------------------------------------
#include "plugins/parameters/internal/tStaticParameterList.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/rtti/rtti.h"
#include <map>

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/parameters/internal/tStaticParameterImplementationBase.h"

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

/*! Initializes annotation type so that it can be transferred to finstruct */
static rrlib::rtti::tDataType<tStaticParameterList> cTYPE;

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

tStaticParameterList::tStaticParameterList() :
  parameters(),
  create_action(-1)
{}

tStaticParameterList::~tStaticParameterList()
{
  Clear();
}

void tStaticParameterList::Add(tStaticParameterImplementationBase& param)
{
  param.list_index = parameters.size();
  param.parent_list = this;
  parameters.push_back(&param);
}

void tStaticParameterList::Clear()
{
  for (int i = parameters.size() - 1; i >= 0; i--)
  {
    delete parameters[i];
  }
  parameters.clear();
}

#ifdef _LIB_RRLIB_XML_PRESENT_
void tStaticParameterList::Deserialize(const rrlib::xml::tNode& node, bool finstruct_context)
{
  size_t number_of_children = std::distance(node.ChildrenBegin(), node.ChildrenEnd());
  bool print_loading_messages = false;
  if (number_of_children != Size())
  {
    FINROC_LOG_PRINT(WARNING, "Number of parameters in XML file differs from expected number of parameters.");
    print_loading_messages = true;
  }

  std::map<size_t, const rrlib::xml::tNode*> parameter_index_to_xml_node_map; // xml parameter index is index; value is matched static parameter in list; NULL if no match could be found
  size_t xml_index = 0;
  for (rrlib::xml::tNode::const_iterator child = node.ChildrenBegin(); child != node.ChildrenEnd(); ++child, ++xml_index)
  {
    if (child->Name() != "parameter")
    {
      FINROC_LOG_PRINT(WARNING, "Found entry with tag '", child->Name(), "' instead of 'parameter'. Ignoring.");
      continue;
    }

    try
    {
      std::string xml_name = child->GetStringAttribute("name");
      tStaticParameterImplementationBase* found = NULL;
      for (size_t i = 0; i < this->Size(); i++)
      {
        if (xml_name == this->Get(i).GetName() || xml_name == "Par " + this->Get(i).GetName()) // Support legacy files where "Par " prefix was not removed
        {
          found = &(this->Get(i));
          if (xml_index != i && (!print_loading_messages))
          {
            FINROC_LOG_PRINT(WARNING, "Parameter with name '", xml_name, "' found in XML file (expected: '", this->Get(xml_index).GetName(), "')");
            print_loading_messages = true;
          }
          parameter_index_to_xml_node_map[i] = &(*child);
          break;
        }
      }
      if ((!found) && (!print_loading_messages))
      {
        FINROC_LOG_PRINT(WARNING, "Parameter with name '", xml_name, "' found in XML file (expected: '", this->Get(xml_index).GetName(), "')");
        print_loading_messages = true;
      }
    }
    catch (const rrlib::xml::tException& ex)
    {
      FINROC_LOG_PRINT(WARNING, "Found parameter without a name in XML file. Ignoring.");
    }
  }

  if (print_loading_messages)
  {
    FINROC_LOG_PRINT(WARNING, "Loading parameters as follows:");
  }
  for (size_t i = 0; i < this->Size(); i++)
  {
    tStaticParameterImplementationBase& param = Get(i);
    auto xml_node = parameter_index_to_xml_node_map.find(i);
    bool apply_default = (xml_node == parameter_index_to_xml_node_map.end());
    if (!apply_default)
    {
      if (print_loading_messages)
      {
        FINROC_LOG_PRINT(WARNING, "- ", param.GetName(), ": from XML parameter '", xml_node->second->GetStringAttribute("name"), "'");
      }
      try
      {
        param.Deserialize(*(xml_node->second), finstruct_context);
      }
      catch (const std::exception& e)
      {
        FINROC_LOG_PRINT(WARNING, "Could not deserialize parameter '", param.GetName(), "' from XML. Reason: ", e);
        apply_default = true;
      }
    }
    if (apply_default)
    {
      FINROC_LOG_PRINT(WARNING, "- ", param.GetName(), ": not modifying current value");
    }
  }
}
#endif

void tStaticParameterList::DoStaticParameterEvaluation(core::tFrameworkElement& fe)
{
  rrlib::thread::tLock lock2(fe.GetStructureMutex());

  // all parameters attached to any of the module's parameters
  std::vector<tStaticParameterImplementationBase*> attached_parameters;
  std::vector<tStaticParameterImplementationBase*> attached_parameters_tmp;

  tStaticParameterList* spl = fe.GetAnnotation<tStaticParameterList>();
  if (spl)
  {

    // Reevaluate parameters and check whether they have changed
    bool changed = false;
    for (size_t i = 0; i < spl->Size(); i++)
    {
      spl->Get(i).LoadValue();
      changed |= spl->Get(i).HasChanged();
      spl->Get(i).GetAllAttachedParameters(attached_parameters_tmp);
      attached_parameters.insert(attached_parameters.end(), attached_parameters_tmp.begin(), attached_parameters_tmp.end());
    }

    if (changed)
    {
      fe.OnStaticParameterChange();

      // Reset change flags for all parameters
      for (size_t i = 0; i < spl->Size(); i++)
      {
        spl->Get(i).ResetChanged();
      }

      // initialize any new child elements
      if (fe.IsReady())
      {
        fe.Init();
      }
    }
  }

  // evaluate children's static parameters
  for (auto it = fe.ChildrenBegin(); it != fe.ChildrenEnd(); ++it)
  {
    // follow only primary links
    if ((it->GetParent() == &fe) && (!it->IsDeleted()))
    {
      DoStaticParameterEvaluation(*it);
    }
  }

  // evaluate any attached parameters that have changed, too
  for (size_t i = 0; i < attached_parameters.size(); i++)
  {
    if (attached_parameters[i]->HasChanged())
    {
      DoStaticParameterEvaluation(*attached_parameters[i]->GetParentList()->GetAnnotated());
    }
  }
}

tStaticParameterList& tStaticParameterList::GetOrCreate(core::tFrameworkElement& fe)
{
  tStaticParameterList* result = fe.GetAnnotation<tStaticParameterList>();
  if (result == NULL)
  {
    result = new tStaticParameterList();
    fe.AddAnnotation(*result);
  }
  return *result;
}

void tStaticParameterList::OnInitialization()
{
  DoStaticParameterEvaluation(*GetAnnotated());
}

std::ostream& operator << (std::ostream& output, const tStaticParameterList& list)
{
  core::tFrameworkElement* annotated = list.GetAnnotated();
  if (annotated)
  {
    output << "Static Parameter List of " << (*annotated);
  }
  else
  {
    output << "Static Parameter List (not attached)";
  }
  return output;
}

#ifdef _LIB_RRLIB_XML_PRESENT_
void tStaticParameterList::Serialize(rrlib::xml::tNode& node, bool finstruct_context) const
{
  for (size_t i = 0u; i < Size(); i++)
  {
    rrlib::xml::tNode& child = node.AddChildNode("parameter");
    tStaticParameterImplementationBase& param = Get(i);
    child.SetAttribute("name", param.GetName());
    param.Serialize(child, finstruct_context);
  }
}
#endif

rrlib::serialization::tOutputStream& operator << (rrlib::serialization::tOutputStream& stream, const tStaticParameterList& list)
{
  stream.WriteInt(list.GetCreateAction());
  stream.WriteInt(static_cast<int>(list.Size()));
  for (size_t i = 0; i < list.Size(); i++)
  {
    list.Get(i).Serialize(stream);
  }
  return stream;
}

rrlib::serialization::tInputStream& operator >> (rrlib::serialization::tInputStream& stream, tStaticParameterList& list)
{
  if (list.GetAnnotated() == NULL)
  {
    throw std::runtime_error("List needs to be attached to framework element before being deserialized.");
  }
  else    // attached to module - only update parameter values
  {
    int read_action = stream.ReadInt();
    if (list.GetCreateAction() != read_action || list.Size() != static_cast<size_t>(stream.ReadInt()))
    {
      throw std::runtime_error("Invalid action id or parameter number");
    }
    core::tFrameworkElement* ann = list.GetAnnotated();
    for (size_t i = 0; i < list.Size(); i++)
    {
      list.Get(i).Deserialize(stream);
    }
    tStaticParameterList::DoStaticParameterEvaluation(*ann);
  }
  return stream;
}

#ifdef _LIB_RRLIB_XML_PRESENT_
rrlib::xml::tNode& operator << (rrlib::xml::tNode& node, const tStaticParameterList& list)
{
  list.Serialize(node, false);
  return node;
}

const rrlib::xml::tNode& operator >> (const rrlib::xml::tNode& node, tStaticParameterList& list)
{
  list.Deserialize(node, false);
  return node;
}
#endif

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}
