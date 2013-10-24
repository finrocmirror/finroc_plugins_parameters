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
/*!\file    plugins/parameters/internal/tParameterImplementation.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-28
 *
 * \brief   Contains tParameterImplementation
 *
 * \b tParameterImplementation
 *
 * Implementation of different types of parameters.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__parameters__internal__tParameterImplementation_h__
#define __plugins__parameters__internal__tParameterImplementation_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "plugins/data_ports/tInputPort.h"

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
//! Parameter implementation
/*!
 * Implementation of different types of parameters.
 */
template <typename T, bool CACHE>
class tParameterImplementation : public data_ports::tInputPort<T>
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  tParameterImplementation() {}

  tParameterImplementation(data_ports::tPortCreationInfo<T> creation_info) :
    data_ports::tInputPort<T>(creation_info)
  {}

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

};

/*!
 * Caches numeric value of parameter port (optimization, since values hardly ever change)
 */
template <typename T, bool FLOATING_POINT>
class tValueCache : public core::tAnnotation
{
public:

  tValueCache() : current_value(0) {}

  T Get() const
  {
    return current_value.load();
  }

  void Set(T value)
  {
    current_value = value;
  }

  void OnPortChange(const T& value, data_ports::tChangeContext& change_context)
  {
    Set(value);
  }

private:

  /*! Cached current value (we will much more often read than it will be changed) */
  std::atomic<T> current_value;
};

// float implementation (there's no std::atomic<float> in gcc 4.6)
template <typename T>
struct tValueCache<T, true> : public core::tAnnotation
{
  typedef typename std::conditional<std::is_same<T, double>::value, uint64_t, uint32_t>::type tStorage;
  static_assert(sizeof(T) == sizeof(tStorage), "Storage size should be identical");

public:

  tValueCache() : current_value(0) {}

  T Get() const
  {
    union
    {
      T original_value;
      tStorage stored_value;
    };
    stored_value = current_value.load();
    return original_value;
  }

  void Set(T value)
  {
    union
    {
      T original_value;
      tStorage stored_value;
    };
    original_value = value;
    current_value = stored_value;
  }

  void OnPortChange(const T& value, data_ports::tChangeContext& change_context)
  {
    Set(value);
  }

private:

  /*! Cached current value (we will much more often read than it will be changed) */
  std::atomic<tStorage> current_value;

};

template <typename T>
class tParameterImplementation<T, true> : public data_ports::tInputPort<T>
{
  typedef tValueCache<T, std::is_floating_point<T>::value> tCache;
  typedef data_ports::tInputPort<T> tBase;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  tParameterImplementation() {}

  tParameterImplementation(data_ports::tPortCreationInfo<T> creation_info) :
    data_ports::tInputPort<T>(creation_info),
    cache(new tCache())
  {
    this->AddAnnotation(*cache);
    this->AddPortListener(*cache);
    cache->Set(tBase::Get());
  }

  T Get() const
  {
    return cache->Get();
  }

  void Get(T& result) const
  {
    result = cache->Get();
  }

  inline data_ports::tPortDataPointer<const T> GetPointer() const
  {
    return data_ports::tPortDataPointer<const T>(data_ports::api::tPortDataPointerImplementation<T, true>(Get(), rrlib::time::cNO_TIME));
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! cache instance used for this parameter */
  tCache* cache;

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif
