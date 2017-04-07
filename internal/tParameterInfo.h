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
/*!\file    plugins/parameters/internal/tParameterInfo.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-28
 *
 * \brief   Contains tParameterInfo
 *
 * \b tParameterInfo
 *
 * Annotates ports that are a parameter
 * and provides parameter-specific functionality.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__parameters__internal__tParameterInfo_h__
#define __plugins__parameters__internal__tParameterInfo_h__

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
namespace internal
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Parameter information
/*!
 * Annotates ports that are a parameter
 * and provides parameter-specific functionality.
 */
class tParameterInfo : public core::tAnnotation
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  tParameterInfo();

#ifdef _LIB_RRLIB_XML_PRESENT_
  void Deserialize(const rrlib::xml::tNode& node, bool finstruct_context, bool include_commmand_line);
#endif

  /*!
   * \return Command line option to set this parameter
   * (set by outer-most finstructable group)
   */
  std::string GetCommandLineOption() const
  {
    return command_line_option;
  }

  /*!
   * \return Place in Configuration tree, this parameter is configured from (nodes are separated with dots)
   */
  inline std::string GetConfigEntry() const
  {
    return config_entry;
  }

  /*!
   * \return Default value set in finstruct (optional)
   * (set by finstructable group responsible for connecting this parameter to attribute tree)
   */
  std::string GetFinstructDefault() const
  {
    return finstruct_default;
  }

  /*!
   * \return Does parameter have any non-default info relevant for finstructed group?
   */
  bool HasNonDefaultFinstructInfo()
  {
    return (config_entry.length() > 0 && entry_set_from_finstruct) || command_line_option.length() > 0 || finstruct_default.length() > 0;
  }

  /*!
   * \return Is config entry set from finstruct/xml?
   */
  bool IsConfigEntrySetFromFinstruct() const
  {
    return entry_set_from_finstruct;
  }

  /*!
   * Is finstructable group the one responsible for saving parameter's config entry?
   *
   * \param finstructable_group Finstructable group to check
   * \param ap Framework element to check this for (usually parameter port)
   * \return Answer.
   */
  static bool IsFinstructableGroupResponsibleForConfigFileConnections(const core::tFrameworkElement& finstructable_group, const core::tFrameworkElement& ap);

  /*!
   * load value from configuration file
   */
  inline void LoadValue()
  {
    LoadValue(false);
  }

  /*!
   * load value from configuration file
   *
   * \param ignore ready flag?
   */
  void LoadValue(bool ignore_ready);

  /*!
   * save value to configuration file
   * (if value equals default value and entry does not exist, no entry is written to file)
   */
  void SaveValue();

#ifdef _LIB_RRLIB_XML_PRESENT_
  void Serialize(rrlib::xml::tNode& node, bool finstruct_context, bool include_command_line) const;
#endif

  /*!
   * \param commandLineOption Command line option to set this parameter
   * (set by outer-most finstructable group)
   */
  void SetCommandLineOption(const std::string& command_line_option)
  {
    this->command_line_option = command_line_option;
  }

  /*!
   * (loads value from configuration file, if is exists
   *
   * \param config_entry New Place in Configuration tree, this parameter is configured from (nodes are separated with dots)
   * \param finstruct_set Is config entry set from finstruct?
   */
  void SetConfigEntry(const std::string& config_entry, bool finstruct_set = false);

  /*!
   * \param finstructDefault Default value set in finstruct.
   * (set by finstructable group responsible for connecting this parameter to attribute tree)
   */
  void SetFinstructDefault(const std::string& finstruct_default)
  {
    this->finstruct_default = finstruct_default;
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  friend rrlib::serialization::tOutputStream& operator << (rrlib::serialization::tOutputStream& stream, const tParameterInfo& parameter_info);
  friend rrlib::serialization::tInputStream& operator >> (rrlib::serialization::tInputStream& stream, tParameterInfo& parameter_info);

  /*!
   * Place in Configuration tree, this parameter is configured from (nodes are separated with '/')
   * (starting with '/' => absolute link - otherwise relative)
   */
  std::string config_entry;

  /*! Was config entry set from finstruct? */
  bool entry_set_from_finstruct;

  /*!
   * Command line option to set this parameter
   * (set by outer-most finstructable group)
   */
  std::string command_line_option;

  /*!
   * Default value set in finstruct (optional)
   * (set by finstructable group responsible for connecting this parameter to attribute tree)
   */
  std::string finstruct_default;


  virtual void OnInitialization() override;
};

rrlib::serialization::tOutputStream& operator << (rrlib::serialization::tOutputStream& stream, const tParameterInfo& parameter_info);
rrlib::serialization::tInputStream& operator >> (rrlib::serialization::tInputStream& stream, tParameterInfo& parameter_info);

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif
