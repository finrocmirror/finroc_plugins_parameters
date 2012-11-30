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
/*!\file    plugins/parameters/internal/tStaticParameterImplementation.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-30
 *
 * \brief   Contains tStaticParameterImplementation
 *
 * \b tStaticParameterImplementation
 *
 * Implementations of tStaticParameter class for different types T.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__parameters__internal__tStaticParameterImplementation_h__
#define __plugins__parameters__internal__tStaticParameterImplementation_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/parameters/internal/tStaticParameterImplementationBase.h"

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
//! tStaticParameter implementations
/*!
 * Implementations of tStaticParameter class for different types T.
 */
template <typename T, bool NUMERIC>
class tStaticParameterImplementation : public tStaticParameterImplementationBase
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  tStaticParameterImplementation(const data_ports::tPortCreationInfo<T>& creation_info) :
    tStaticParameterImplementationBase(creation_info.name, rrlib::rtti::tDataType<T>(), false, false, creation_info.config_entry)
  {
    if (creation_info.DefaultValueSet())
    {
      rrlib::serialization::tInputStream is(&creation_info.GetDefaultGeneric());
      is >> *(ValuePointer());
    }
  }

  T& Get()
  {
    return ValuePointer()->template GetData<T>();
  }

  inline void Set(const T& new_value)
  {
    ValuePointer()->template GetData<T>() = new_value;
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  virtual tStaticParameterImplementationBase* DeepCopy() // TODO: mark with override when we use gcc 4.7
  {
    return new tStaticParameterImplementation(data_ports::tPortCreationInfo<T>(GetName()));
  }
};


template <typename T>
class tStaticParameterImplementation<T, true> : public tStaticParameterImplementationBase
{

  /*! Class that contains port implementation for type T */
  typedef data_ports::api::tPortImplementation<T, data_ports::api::tPortImplementationTypeTrait<T>::type> tPortImplementation;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  tStaticParameterImplementation(const data_ports::tPortCreationInfo<T>& creation_info) :
    tStaticParameterImplementationBase(creation_info.name, rrlib::rtti::tDataType<data_ports::numeric::tNumber>(), false, false, creation_info.config_entry),
    unit(creation_info.unit),
    bounds(creation_info.BoundsSet() ? creation_info.GetBounds() : data_ports::tBounds<T>())
  {
    if (creation_info.DefaultValueSet())
    {
      Set(creation_info.GetDefault());
    }
  }

  T& Get()
  {
    current_value_temp = tPortImplementation::ToValue(ValuePointer()->template GetData<data_ports::numeric::tNumber>(), unit);
    return current_value_temp;
  }

  inline void Set(T new_value)
  {
    if (!bounds.InBounds(new_value))
    {
      if (bounds.GetOutOfBoundsAction() == data_ports::tOutOfBoundsAction::DISCARD)
      {
        return;
      }
      else if (bounds.GetOutOfBoundsAction() == data_ports::tOutOfBoundsAction::ADJUST_TO_RANGE)
      {
        new_value = bounds.ToBounds(new_value);
      }
      else if (bounds.GetOutOfBoundsAction() == data_ports::tOutOfBoundsAction::APPLY_DEFAULT)
      {
        new_value = bounds.GetOutOfBoundsDefault();
      }
    }
    tPortImplementation::Assign(ValuePointer()->template GetData<data_ports::numeric::tNumber>(), new_value, unit);
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! Unit of parameter */
  data_ports::tUnit unit;

  /*! Bounds of this parameter */
  data_ports::tBounds<T> bounds;

  /*! Temporary storage for current value - so that we can return reference */
  T current_value_temp;


  virtual tStaticParameterImplementationBase* DeepCopy() // TODO: mark with override when we use gcc 4.7
  {
    return new tStaticParameterImplementation(data_ports::tPortCreationInfo<T>(GetName(), unit, bounds));
  }
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif
