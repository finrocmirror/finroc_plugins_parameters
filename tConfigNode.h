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
/*!\file    plugins/parameters/tConfigNode.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-29
 *
 * \brief   Contains tConfigNode
 *
 * \b tConfigNode
 *
 * Using this annotation, a common parent config file node for all a module's/group's
 * parameter config entries can be specified.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__parameters__tConfigNode_h__
#define __plugins__parameters__tConfigNode_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
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
//! Specifies config file nodes as root nodes for framework elements
/*!
 * Using this annotation, a common parent config file node for all a module's/group's
 * parameter config entries can be specified.
 */
class tConfigNode : public core::tAnnotation
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * Get config file node to use for the specified framework element.
   * It searches in parent framework elements for any entries to
   * determine which one to use.
   *
   * \param fe Framework element
   */
  static std::string GetConfigNode(core::tFrameworkElement& fe);

  /*!
   * Get full config entry for specified parent - taking any common config file node
   * stored in parents into account
   *
   * \param parent Parent framework element
   * \param config_entry Config entry (possibly relative to parent config file node - if not starting with '/')
   * \return Config entry to use
   */
  static std::string GetFullConfigEntry(core::tFrameworkElement& parent, const std::string& config_entry);

  /*!
   * Set config file node for the specified framework element.
   *
   * \param fe Framework element
   * \param node Common parent config file node for all child parameter config entries (starting with '/' => absolute link - otherwise relative).
   */
  static void SetConfigNode(core::tFrameworkElement& fe, const std::string& node);

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! Config file entry for node (starting with '/' => absolute link - otherwise relative) */
  tString node;


  tConfigNode(const std::string& node = "");
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif
