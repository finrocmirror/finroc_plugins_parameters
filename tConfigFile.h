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
/*!\file    plugins/parameters/tConfigFile.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-28
 *
 * \brief   Contains tConfigFile
 *
 * \b tConfigFile
 *
 * Configuration file.
 * Is an xml file consisting of a tree of nodes with values as leafs.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__parameters__tConfigFile_h__
#define __plugins__parameters__tConfigFile_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/serialization/serialization.h"
#include "core/tFrameworkElement.h"

//----------------------------------------------------------------------
// Internal includes with ""
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
// Class declaration
//----------------------------------------------------------------------
//! Configuration file.
/*!
 * Configuration file.
 * Is an xml file consisting of a tree of nodes with values as leafs.
 */
class tConfigFile : public core::tAnnotation
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * \param filename File name of configuration file (loaded if it exists)
   * \param optional Is this an optional config file? (if false and specified file does not exists, prints a warning)
   */
  tConfigFile(const std::string& filename, bool optional = false);

  /*!
   * Create empty config file with no filename (should only be used to deserialize from stream)
   */
  tConfigFile();


  /*!
   * Appends all entries from specified config file to this one
   *
   * \param filename File name of configuration file (loaded if it exists)
   * \throw Throws std::exception if specified file cannot be found or loaded
   */
  void Append(const std::string& filename);

  /*!
   * Find ConfigFile which specified element is configured from
   *
   * \param element Element
   * \return ConfigFile - or null if none could be found
   */
  static tConfigFile* Find(const core::tFrameworkElement& element);

#ifdef _LIB_RRLIB_XML_PRESENT_
  /*!
   * Get entry from configuration file
   *
   * \param entry Entry
   * \param create (Re)create entry node?
   * \return XMLNode representing entry
   *
   * \throw Throws std::runtime_error if entry cannot be found
   */
  rrlib::xml::tNode& GetEntry(const std::string& entry, bool create = false);
#endif

  /*!
   * \return Filename of current config file
   */
  inline std::string GetFilename() const
  {
    return filename;
  }

  /*!
   * Searches given entry in config file and returns its value as string if present.
   * \param entry the entry in the config file to be searched
   * \return string value of entry if present, empty string otherwise
   */
  std::string GetStringEntry(const std::string& entry);

  /*!
   * Does configuration file have the specified entry?
   *
   * \param entry Entry
   * \return Answer
   */
  bool HasEntry(const std::string& entry);

  /*!
   * (Should only be used when Annotatable::getAnnotation() is called manually)
   *
   * \return Is config file active (does it "exist")?
   */
  inline bool IsActive() const
  {
    return active;
  }

  /*!
   * set parameters of all child nodes to current values in tree
   */
  void LoadParameterValues();

  /*!
   * set parameters of all framework element's child nodes to current values in tree
   *
   * \param fe Framework element
   */
  void LoadParameterValues(core::tFrameworkElement& fe);

  /*!
   * Saves configuration file back to HDD
   *
   * \param new_filename Name of file to save to (if empty, preserves current file name)
   */
  void SaveFile(const std::string& new_filename);

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  friend rrlib::serialization::tOutputStream& operator << (rrlib::serialization::tOutputStream& stream, const tConfigFile& file);
  friend rrlib::serialization::tInputStream& operator >> (rrlib::serialization::tInputStream& stream, tConfigFile& file);

#ifdef _LIB_RRLIB_XML_PRESENT_
  /*! (Wrapped) XML document */
  rrlib::xml::tDocument wrapped;
#endif

  /*! File name of configuration file */
  std::string filename;

  /*! Is config file active? (false when config file is deleted via finstruct) */
  bool active;

#ifdef _LIB_RRLIB_XML_PRESENT_
  /*!
   * \param entry Config entry that created node should have
   * \param leaf Should created entry be a leaf node?
   * \return Returns or creates node with the specified config entry - possibly recursively
   */
  rrlib::xml::tNode& CreateEntry(const std::string& entry, bool leaf);

  /*!
   * Implementation of GetEntry() - called recursively
   *
   * \param result List that will contain the result(s) as additional entry/entries after the call
   * \param entry Entry
   * \param node Current node
   * \param entry_string_index Current index in entry string
   */
  void GetEntryImplementation(std::vector<rrlib::xml::tNode*>& result, const std::string& entry, rrlib::xml::tNode& node, size_t entry_string_index);
#endif

};

rrlib::serialization::tOutputStream& operator << (rrlib::serialization::tOutputStream& stream, const tConfigFile& file);
rrlib::serialization::tInputStream& operator >> (rrlib::serialization::tInputStream& stream, tConfigFile& file);

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif
