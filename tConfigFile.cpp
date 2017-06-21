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
#ifdef _LIB_RRLIB_XML_PRESENT_
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
#endif

/*! Initializes annotation type so that it can be transferred to finstruct */
static rrlib::rtti::tDataType<tConfigFile> cTYPE;

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

tConfigFile::tConfigFile() :
#ifdef _LIB_RRLIB_XML_PRESENT_
  wrapped(),
#endif
  filename(),
  active(true)
{
#ifdef _LIB_RRLIB_XML_PRESENT_
  wrapped.AddRootNode(cXML_BRANCH_NAME);
#endif
}

tConfigFile::tConfigFile(const std::string& filename, bool optional) :
#ifdef _LIB_RRLIB_XML_PRESENT_
  wrapped(),
#endif
  filename(filename),
  active(true)
{
#ifdef _LIB_RRLIB_XML_PRESENT_
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
  else if (!optional)
  {
    FINROC_LOG_PRINT(WARNING, "Specified config file not found: ", filename);
  }
  wrapped = rrlib::xml::tDocument();
  wrapped.AddRootNode(cXML_BRANCH_NAME);
#endif
}

void tConfigFile::Append(const std::string& filename)
{
#ifdef _LIB_RRLIB_XML_PRESENT_
  if (core::FinrocFileExists(filename))
  {
    // merge entries into first document
    auto document = core::GetFinrocXMLDocument(filename, false); // false = do not validate with dtd
    auto& root_node = document.RootNode();
    for (auto it = root_node.ChildrenBegin(); it != root_node.ChildrenEnd(); ++it)
    {
      this->wrapped.RootNode().AddChildNode(*it, true); // not using copy resulted in erroneous behavior
    }
  }
  else
  {
    throw std::runtime_error("Specified config file not found: " + filename);
  }
#endif
}

#ifdef _LIB_RRLIB_XML_PRESENT_
rrlib::xml::tNode& tConfigFile::CreateEntry(const std::string& entry, bool leaf)
{
  if (!leaf)
  {
    std::vector<rrlib::xml::tNode*> result;
    GetEntryImplementation(result, entry, wrapped.RootNode(), 0);
    if (result.size() > 0)
    {
      // do we want to warn if node is a leaf node? - I currently do not think so
      return *result[0];
    }
  }

  size_t slash_index = entry.rfind('/');
  tXMLNode& parent = (slash_index == std::string::npos || slash_index == 0) ? wrapped.RootNode() : CreateEntry(entry.substr(0, slash_index), false);
  tXMLNode& created = parent.AddChildNode(leaf ? cXML_LEAF_NAME : cXML_BRANCH_NAME);
  created.SetAttribute("name", (slash_index == std::string::npos) ? entry : entry.substr(slash_index + 1));
  return created;
}
#endif

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

#ifdef _LIB_RRLIB_XML_PRESENT_
rrlib::xml::tNode& tConfigFile::GetEntry(const std::string& entry, bool create)
{
  std::vector<rrlib::xml::tNode*> result;
  GetEntryImplementation(result, entry, wrapped.RootNode(), 0);
  if (result.size() > 1)
  {
    FINROC_LOG_PRINT(WARNING, "There are ", result.size(), " entries in config file with the qualified name '", entry, "'. Using the first one.");
  }

  if (!create)
  {
    if (result.size() == 0)
    {
      throw std::runtime_error("Config node not found: " + entry);
    }
    if (result[0]->Name() != cXML_LEAF_NAME)
    {
      throw std::runtime_error("Config node is no leaf: " + entry);
    }
    return *result[0];
  }

  // create node...
  if (result.size() > 0)
  {
    // recreate existing node
    std::string name = result[0]->GetStringAttribute("name");
    tXMLNode& parent = result[0]->Parent();
    parent.RemoveChildNode(*result[0]);
    tXMLNode& new_node = parent.AddChildNode(cXML_LEAF_NAME);
    new_node.SetAttribute("name", name);
    return new_node;
  }
  else
  {
    return CreateEntry(entry, true);
  }
}

void tConfigFile::GetEntryImplementation(std::vector<rrlib::xml::tNode*>& result, const std::string& entry, rrlib::xml::tNode& node, size_t entry_string_index)
{
  if (entry_string_index >= entry.length())
  {
    return;
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
              GetEntryImplementation(result, entry, *child, new_entry_string_index);
            }
          }
          else
          {
            result.push_back(&(*child));
          }
        }
      }
      catch (const std::exception& e)
      {
        FINROC_LOG_PRINT(WARNING, "Encountered tree node without name");
      }
    }
  }

  // Okay, we did not find any more
}
#endif

std::string tConfigFile::GetStringEntry(const std::string& entry)
{
#ifdef _LIB_RRLIB_XML_PRESENT_
  try
  {
    return GetEntry(entry).GetTextContent();
  }
  catch (const std::exception& e)
  {
    return "";
  }
#else
  return "";
#endif
}

bool tConfigFile::HasEntry(const std::string& entry)
{
#ifdef _LIB_RRLIB_XML_PRESENT_
  // TODO: could be implemented more efficiently
  try
  {
    GetEntry(entry);
    return true;
  }
  catch (const std::exception& e)
  {
    return false;
  }
#else
  return false;
#endif
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
#ifdef _LIB_RRLIB_XML_PRESENT_
  // first: update tree
  core::tFrameworkElement* ann = GetAnnotated<core::tFrameworkElement>();
  if (ann)
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
#endif
}

rrlib::serialization::tOutputStream& operator << (rrlib::serialization::tOutputStream& stream, const tConfigFile& config_file)
{
#ifdef _LIB_RRLIB_XML_PRESENT_
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
#endif
  return stream;
}

rrlib::serialization::tInputStream& operator >> (rrlib::serialization::tInputStream& stream, tConfigFile& config_file)
{
#ifdef _LIB_RRLIB_XML_PRESENT_
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
#endif
  return stream;
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
