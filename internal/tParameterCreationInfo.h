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
/*!\file    plugins/parameters/internal/tParameterCreationInfo.h
 *
 * \author  Max Reichardt
 *
 * \date    2013-09-09
 *
 * \brief   Contains tParameterCreationInfo
 *
 * \b tParameterCreationInfo
 *
 * This class bundles various parameters for the creation of parameters.
 *
 * Instead of providing suitable constructors for all types of sensible
 * combinations of the numerous (often optional) construction parameters,
 * there is only one constructor taking a single argument of this class.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__parameters__internal__tParameterCreationInfo_h__
#define __plugins__parameters__internal__tParameterCreationInfo_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "plugins/data_ports/tPortCreationInfo.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/parameters/definitions.h"

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
//! Bundle of parameter creation parameters
/*!
 * This class bundles various parameters for the creation of parameters.
 *
 * Instead of providing suitable constructors for all types of sensible
 * combinations of the numerous (often optional) construction parameters,
 * there is only one constructor taking a single argument of this class.
 */
template <typename T>
class tParameterCreationInfo : public data_ports::tPortCreationInfo<T>
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  typedef data_ports::tPortCreationInfo<T> tBase;

  /*! Change callback setting */
  tChangeCallback change_callback;

  tParameterCreationInfo() :
    change_callback(tChangeCallback::ON_CHECK_ONLY)
  {}

  /*! Set methods for parameter-specific properties */
  void Set(const tParameterCreationInfo& other)
  {
    *this = other;
  }

  void Set(const tChangeCallback& change_callback)
  {
    this->change_callback = change_callback;
  }
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif
