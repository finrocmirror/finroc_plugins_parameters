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
/*!\file    plugins/parameters/definitions.h
 *
 * \author  Max Reichardt
 *
 * \date    2013-09-09
 *
 * Various definitions for parameters plugin
 */
//----------------------------------------------------------------------
#ifndef __plugins__parameters__definitions_h__
#define __plugins__parameters__definitions_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

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

/*!
 * Can be passed to constructor of static parameters.
 * Determines when the callback "OnStaticParameterChange()" is invoked.
 */
enum class tChangeCallback
{
  /*!
   * Default. Parameters are checked for changes (and the callback is possibly invoked)
   * when the parent component is initialized or its "CheckStaticParameters()" method is called.
   */
  ON_CHECK_ONLY,

  /*!
   * Parameters are checked for changes (and the callback is possibly invoked) immediately
   * when the parameter's value is set.
   */
  ON_SET
};

//----------------------------------------------------------------------
// Function declarations
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif
