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
#include "core/file_lookup.h"

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
typedef rrlib::xml::tNode tXMLNode;

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
  if (core::FinrocFileExists(filename))
  {
    try
    {
      wrapped = core::GetFinrocXMLDocument(filename, false); // false = do not validate with dtd
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

rrlib::xml::tNode& tConfigFile::CreateEntry(const std::string& entry, bool leaf)
{
  if (!leaf)
  {
    std::pair<tXMLNode*, tXMLNode*> found = GetEntryImplementation(entry, wrapped.RootNode(), 0);
    if (found.first)
    {
      // do we want to warn if node is a leaf node? - I currently do not think so
      return *found.first;
    }
  }

  size_t slash_index = entry.rfind('/');
  tXMLNode& parent = (slash_index == std::string::npos || slash_index == 0) ? wrapped.RootNode() : CreateEntry(entry.substr(0, slash_index), false);
  tXMLNode& created = parent.AddChildNode(leaf ? cXML_LEAF_NAME : cXML_BRANCH_NAME);
  created.SetAttribute("name", (slash_index == std::string::npos) ? entry : entry.substr(slash_index + 1));
  return created;
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
  std::pair<tXMLNode*, tXMLNode*> found = GetEntryImplementation(entry, wrapped.RootNode(), 0);
  if (!create)
  {
    if (!found.first)
    {
      throw std::runtime_error("Config node not found: " + entry);
    }
    if (found.first->Name() != cXML_LEAF_NAME)
    {
      throw std::runtime_error("Config node is no leaf: " + entry);
    }
    return *found.first;
  }

  // create node...
  if (found.first)
  {
    // recreate existing node
    std::string name = found.first->GetStringAttribute("name");
    found.second->RemoveChildNode(*found.first);
    found.first = &(found.second->AddChildNode(cXML_LEAF_NAME));
    found.first->SetAttribute("name", name);
    return *found.first;
  }
  else
  {
    return CreateEntry(entry, true);
  }
}

std::pair<tXMLNode*, tXMLNode*> tConfigFile::GetEntryImplementation(const std::string& entry, rrlib::xml::tNode& node, size_t entry_string_index)
{
  typedef std::pair<tXMLNode*, tXMLNode*> tResult;

  if (entry_string_index >= entry.length())
  {
    return tResult(NULL, NULL);
  }

  // Check for slash at beginning
  if (entry[entry_string_index] == '/')
  {
    if (entry_string_index > 0)
    {
      FINROC_LOG_PRINT(WARNING, "Entry '", entry, "' seems to be not clean (sequential slashes). Skipping one slash now, as this is typically intended. Please fix this!");
    }
    entry_string_index++;
  }

  // Search child nodes
  for (rrlib::xml::tNode::iterator child = node.ChildrenBegin(); child != node.ChildrenEnd(); ++child)
  {
    if (child->Name() == cXML_BRANCH_NAME || child->Name() == cXML_LEAF_NAME)
    {
      try
      {
        std::string name_attribute = child->GetStringAttribute("name");
        if (entry.compare(entry_string_index, name_attribute.length(), name_attribute) == 0) // starts_with name attribute?
        {
          size_t new_entry_string_index = entry_string_index + name_attribute.length();
          if (new_entry_string_index != entry.length())
          {
            if (entry[new_entry_string_index] == '/')
            {
              new_entry_string_index++;
              tResult result = GetEntryImplementation(entry, *child, new_entry_string_index);
              if (result.first)
              {
                return result;
              }
            }
          }
          else
          {
            return tResult(&(*child), &node);
          }
        }
      }
      catch (const std::exception& e)
      {
        FINROC_LOG_PRINT(WARNING, "Encountered tree node without name");
      }
    }
  }

  // Okay, we did not find one
  return tResult(NULL, NULL);
}

std::string tConfigFile::GetStringEntry(const std::string& entry)
{
  auto result = GetEntryImplementation(entry, wrapped.RootNode(), 0);
  if (result.first)
  {
    try
    {
      return result.first->GetTextContent();
    }
    catch (const std::exception& e)
    {
      return "";
    }
  }
  return "";
}

bool tConfigFile::HasEntry(const std::string& entry)
{
  auto result = GetEntryImplementation(entry, wrapped.RootNode(), 0);
  return result.first && (result.first->Name() == cXML_LEAF_NAME);
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

void tConfigFile::SaveFile(const std::string& new_filename)
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
    if (new_filename.length() > 0)
    {
      this->filename = new_filename;
    }
    std::string save_to = core::GetFinrocFileToSaveTo(this->filename);
    if (save_to.length() == 0)
    {
      std::string save_to_alt = save_to;
      std::replace(save_to_alt.begin(), save_to_alt.end(), '/', '_'); // Replace '/' characters with '_'
      save_to_alt = core::GetFinrocFileToSaveTo(save_to_alt);
      FINROC_LOG_PRINT(ERROR, "There does not seem to be any suitable location for: '", this->filename, "' . For now, using '", save_to_alt, "'.");
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

  if (config_file.active && file.length() > 0 && content.length() == 0 && (file != config_file.filename))
  {
    // load file
    if (core::FinrocFileExists(file))
    {
      try
      {
        config_file.wrapped = core::GetFinrocXMLDocument(file, false);
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
