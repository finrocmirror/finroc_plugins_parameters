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
#include "plugins/data_ports/tPort.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/parameters/internal/tParameterCreationInfo.h"
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

  static tStaticParameterImplementation* CreateInstance(const internal::tParameterCreationInfo<T>& creation_info, bool constructor_prototype)
  {
    return new tStaticParameterImplementation(creation_info, constructor_prototype);
  }

  T& Get()
  {
    return ValuePointer()->template GetData<T>();
  }

  inline void Set(const T& new_value)
  {
    ValuePointer()->template GetData<T>() = new_value;
    tStaticParameterImplementationBase::NotifyChange();
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  tStaticParameterImplementation(const internal::tParameterCreationInfo<T>& creation_info, bool constructor_prototype) :
    tStaticParameterImplementationBase(creation_info.name, rrlib::rtti::tDataType<T>(), constructor_prototype, false, creation_info.config_entry)
  {
    if (creation_info.DefaultValueSet())
    {
      rrlib::serialization::tInputStream is(creation_info.GetDefaultGeneric());
      ValuePointer()->Deserialize(is);
    }
  }

  virtual tStaticParameterImplementationBase* DeepCopy() // TODO: mark with override when we use gcc 4.7
  {
    internal::tParameterCreationInfo<T> creation_info;
    creation_info.name = GetName();
    return new tStaticParameterImplementation(creation_info, false);
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

  static tStaticParameterImplementation* CreateInstance(const internal::tParameterCreationInfo<T>& creation_info, bool constructor_prototype);

  T& Get()
  {
    current_value_temp = tPortImplementation::ToValue(ValuePointer()->template GetData<data_ports::numeric::tNumber>(), unit);
    return current_value_temp;
  }

  virtual void Set(T new_value)
  {
    tPortImplementation::Assign(ValuePointer()->template GetData<data_ports::numeric::tNumber>(), new_value, unit);
    tStaticParameterImplementationBase::NotifyChange();
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
protected:

  /*! Unit of parameter */
  data_ports::tUnit unit;

  /*! Temporary storage for current value - so that we can return reference */
  T current_value_temp;


  tStaticParameterImplementation(const internal::tParameterCreationInfo<T>& creation_info, bool constructor_prototype) :
    tStaticParameterImplementationBase(creation_info.name, rrlib::rtti::tDataType<data_ports::numeric::tNumber>(), constructor_prototype, false, creation_info.config_entry),
    unit(creation_info.unit)
  {
    if (creation_info.DefaultValueSet())
    {
      Set(creation_info.GetDefault());
    }
  }

  virtual tStaticParameterImplementationBase* DeepCopy() // TODO: mark with override when we use gcc 4.7
  {
    return new tStaticParameterImplementation(
             core::tPortWrapperBase::tConstructorArguments<internal::tParameterCreationInfo<T>>(GetName(), unit), false);
  }
};

template <typename T>
class tBoundedNumericStaticParameterImplementation : public tStaticParameterImplementation<T, true>
{
  typedef tStaticParameterImplementation<T, true> tBase;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  tBoundedNumericStaticParameterImplementation(const internal::tParameterCreationInfo<T>& creation_info, bool constructor_prototype) :
    tBase(creation_info, constructor_prototype),
    bounds(creation_info.GetBounds())
  {
  }

  virtual void Set(T new_value) // TODO: mark with override when we use gcc 4.7
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
    tBase::Set(new_value);
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! Bounds of this parameter */
  data_ports::tBounds<T> bounds;

  virtual tStaticParameterImplementationBase* DeepCopy() // TODO: mark with override when we use gcc 4.7
  {
    return new tBoundedNumericStaticParameterImplementation(
             core::tPortWrapperBase::tConstructorArguments<internal::tParameterCreationInfo<T>>(this->GetName(), this->unit, bounds), false);
  }
};


template <typename T>
tStaticParameterImplementation<T, true>* tStaticParameterImplementation<T, true>::CreateInstance(
  const internal::tParameterCreationInfo<T>& creation_info, bool constructor_prototype)
{
  if (creation_info.BoundsSet())
  {
    return new tBoundedNumericStaticParameterImplementation<T>(creation_info, constructor_prototype);
  }
  return new tStaticParameterImplementation<T, true>(creation_info, constructor_prototype);
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif
