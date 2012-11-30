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
/*!\file    plugins/parameters/tConfigFile.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-28
 *
 */
//----------------------------------------------------------------------
#include "plugins/parameters/tConfigFile.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/rtti/rtti.h"
#include "rrlib/finroc_core_utils/sFiles.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
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

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Const values
//----------------------------------------------------------------------
/*! Separator entries are divided with */
static const std::string cSEPARATOR("/");

/*! Branch name in XML */
static const std::string cXML_BRANCH_NAME("node");

/*! Leaf name in XML */
static const std::string cXML_LEAF_NAME("value");

/*! Initializes annotation type so that it can be transferred to finstruct */
static rrlib::rtti::tDataType<tConfigFile> cTYPE;

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

tConfigFile::tConfigFile() :
  wrapped(),
  filename(),
  active(true)
{
  wrapped.AddRootNode(cXML_BRANCH_NAME);
}

tConfigFile::tConfigFile(const std::string& filename) :
  wrapped(),
  filename(filename),
  active(true)
{
  if (util::sFiles::FinrocFileExists(filename))
  {
    try
    {
      wrapped = util::sFiles::GetFinrocXMLDocument(filename, false); // false = do not validate with dtd
      return;
    }
    catch (const std::exception& e)
    {
      FINROC_LOG_PRINT(ERROR, e);
    }
  }
  wrapped = rrlib::xml::tDocument();
  wrapped.AddRootNode(cXML_BRANCH_NAME);
}

tConfigFile* tConfigFile::Find(const core::tFrameworkElement& element)
{
  tConfigFile* config_file = element.GetAnnotation<tConfigFile>();
  if (config_file && config_file->active == true)
  {
    return config_file;
  }
  core::tFrameworkElement* parent = element.GetParent();
  if (parent)
  {
    return Find(*parent);
  }
  return NULL;
}

rrlib::xml::tNode& tConfigFile::GetEntry(const std::string& entry, bool create)
{
  std::vector<std::string> nodes;
  boost::split(nodes, entry, boost::is_any_of(cSEPARATOR));
  size_t idx = (nodes.size() > 0 && nodes[0].length() == 0) ? 1 : 0; // if entry starts with '/', skip first empty string
  rrlib::xml::tNode::iterator current = &wrapped.RootNode();
  rrlib::xml::tNode::iterator parent = current;
  bool created = false;
  while (idx < nodes.size())
  {
    if (nodes[idx].length() == 0)
    {
      FINROC_LOG_PRINT(WARNING, "Entry '", entry, "' is not clean. Skipping empty string now, but please fix this!");
      idx++;
      continue;
    }
    bool found = false;
    for (rrlib::xml::tNode::iterator child = current->ChildrenBegin(); child != current->ChildrenEnd(); ++child)
    {
      if (boost::equals(cXML_BRANCH_NAME, child->Name()) || boost::equals(cXML_LEAF_NAME, child->Name()))
      {
        try
        {
          if (boost::equals(nodes[idx], child->GetStringAttribute("name")))
          {
            idx++;
            parent = current;
            current = child;
            found = true;
            break;
          }
        }
        catch (const std::exception& e)
        {
          FINROC_LOG_PRINT(WARNING, "tree node without name");
        }
      }
    }
    if (!found)
    {
      if (create)
      {
        parent = current;
        current = &(current->AddChildNode((idx == nodes.size() - 1) ? cXML_LEAF_NAME : cXML_BRANCH_NAME));
        created = true;
        current->SetAttribute("name", nodes[idx]);
        idx++;
      }
      else
      {
        throw util::tRuntimeException(std::string("Node not found: ") + entry, CODE_LOCATION_MACRO);
      }
    }
  }
  if (!boost::equals(cXML_LEAF_NAME, current->Name()))
  {
    throw util::tRuntimeException("Node no leaf", CODE_LOCATION_MACRO);
  }

  // Recreate node?
  if (create && (!created))
  {
    parent->RemoveChildNode(*current);
    current = &(parent->AddChildNode(cXML_LEAF_NAME));
    current->SetAttribute("name", nodes[nodes.size() - 1]);
  }

  return *current;
}

std::string tConfigFile::GetStringEntry(const std::string& entry)
{
  if (this->HasEntry(entry))
  {
    try
    {
      return GetEntry(entry, false).GetTextContent();
    }
    catch (const std::exception& e)
    {
      return "";
    }
  }
  else
  {
    return "";
  }
}

bool tConfigFile::HasEntry(const std::string& entry)
{
  try
  {
    GetEntry(entry, false);
  }
  catch (const std::exception& e)
  {
    return false;
  }
  return true;
}

void tConfigFile::LoadParameterValues()
{
  LoadParameterValues(*GetAnnotated<core::tFrameworkElement>());
}

void tConfigFile::LoadParameterValues(core::tFrameworkElement& fe)
{
  rrlib::thread::tLock lock(fe.GetStructureMutex());  // nothing should change while we're doing this
  for (auto it = fe.SubElementsBegin(true); it != fe.SubElementsEnd(); ++it)
  {
    if (it->IsPort() && it->IsReady() && Find(*it) == this)    // Does element belong to this configuration file?
    {
      internal::tParameterInfo* pi = it->GetAnnotation<internal::tParameterInfo>();
      if (pi)
      {
        try
        {
          pi->LoadValue();
        }
        catch (const std::exception& e)
        {
          FINROC_LOG_PRINT_STATIC(ERROR, e);
        }
      }
    }
  }
}

void tConfigFile::SaveFile()
{
  // first: update tree
  core::tFrameworkElement* ann = GetAnnotated<core::tFrameworkElement>();
  assert(ann);
  {
    rrlib::thread::tLock lock(ann->GetStructureMutex()); // nothing should change while we're doing this
    for (auto it = ann->SubElementsBegin(true); it != ann->SubElementsEnd(); ++it)
    {
      if (it->IsPort() && it->IsReady() && Find(*it) == this)    // Does element belong to this configuration file?
      {
        internal::tParameterInfo* pi = it->GetAnnotation<internal::tParameterInfo>();
        if (pi)
        {
          try
          {
            pi->SaveValue();
          }
          catch (const std::exception& e)
          {
            FINROC_LOG_PRINT_STATIC(ERROR, e);
          }
        }
      }
    }
  }

  try
  {
    std::string save_to = util::sFiles::GetFinrocFileToSaveTo(filename);
    if (save_to.length() == 0)
    {
      std::string save_to_alt = util::sFiles::GetFinrocFileToSaveTo(boost::replace_all_copy(filename, "/", "_"));
      FINROC_LOG_PRINT(ERROR, "There does not seem to be any suitable location for: '", filename, "' . For now, using '", save_to_alt, "'.");
      save_to = save_to_alt;
    }

    // write new tree to file
    wrapped.WriteToFile(save_to);
  }
  catch (const std::exception& e)
  {
    FINROC_LOG_PRINT(ERROR, e);
  }
}

rrlib::serialization::tOutputStream& operator << (rrlib::serialization::tOutputStream& stream, const tConfigFile& config_file)
{
  stream.WriteBoolean(config_file.IsActive());
  stream.WriteString(config_file.GetFilename());

  try
  {
    stream.WriteString(config_file.wrapped.RootNode().GetXMLDump());
  }
  catch (const std::exception& e)
  {
    FINROC_LOG_PRINT(ERROR, e); // Should never occur
  }
  return stream;
}

rrlib::serialization::tInputStream& operator >> (rrlib::serialization::tInputStream& stream, tConfigFile& config_file)
{
  config_file.active = stream.ReadBoolean();
  std::string file = stream.ReadString();
  std::string content = stream.ReadString();

  if (config_file.active && file.length() > 0 && content.length() == 0 && (!boost::equals(file, config_file.filename)))
  {
    // load file
    if (util::sFiles::FinrocFileExists(file))
    {
      try
      {
        config_file.wrapped = util::sFiles::GetFinrocXMLDocument(file, false);
      }
      catch (const std::exception& e)
      {
        FINROC_LOG_PRINT(ERROR, e);
        config_file.wrapped = rrlib::xml::tDocument();
        try
        {
          config_file.wrapped.AddRootNode(cXML_BRANCH_NAME);
        }
        catch (const std::exception& e1)
        {
          FINROC_LOG_PRINT(ERROR, e1);
        }
      }
    }
    config_file.filename = file;
  }
  else if (config_file.active && content.length() > 0)
  {
    if (file.length() > 0)
    {
      config_file.filename = file;
    }

    try
    {
      config_file.wrapped = rrlib::xml::tDocument(content.c_str(), content.length() + 1);
    }
    catch (const std::exception& e)
    {
      FINROC_LOG_PRINT(ERROR, e);
    }
  }
  return stream;
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
