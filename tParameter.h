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
/*!\file    plugins/parameters/tParameter.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-28
 *
 * \brief   Contains tParameter
 *
 * \b tParameter
 *
 * Parameter that can be changed at application runtime.
 * To deal with issues of concurrency, it is based on tPort.
 * Parameter values can be set in code, loaded from configuration
 * files or specified via the command line if set up accordingly.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__parameters__tParameter_h__
#define __plugins__parameters__tParameter_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/parameters/internal/tParameterImplementation.h"
#include "plugins/parameters/internal/tParameterInfo.h"

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
//! Parameter
/*!
 * Parameter that can be changed at application runtime.
 * To deal with issues of concurrency, it is based on tPort.
 * Parameter values can be set in code, loaded from configuration
 * files or specified via the command line if set up accordingly.
 */
template <typename T>
class tParameter
{
  typedef internal::tParameterImplementation < T, data_ports::tIsNumeric<T>::value || std::is_same<bool, T>::value > tImplementation;

  typedef data_ports::api::tPortImplementation<T, data_ports::api::tPortImplementationTypeTrait<T>::type> tPortImplementation;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*! Should methods passing buffers by-value be available? */
  enum { cPASS_BY_VALUE = data_ports::tIsCheaplyCopiedType<T>::value };

  /*!
   * Constructor takes variadic argument list... just any properties you want to assign to parameter.
   *
   * The first string is interpreted as parameter name, the second possibly as config entry.
   * A framework element pointer is interpreted as parent.
   * tFrameworkElement::tFlags arguments are interpreted as flags.
   * int argument is interpreted as queue length (ignored)
   * tBounds<T> are parameter's bounds.
   * tUnit argument is parameter's's unit.
   * int16/short argument is interpreted as minimum network update interval (ignored)
   * const T& is interpreted as port's default value.
   * tPortCreationInfo<T> argument is copied. This is only allowed as first argument.
   *
   * This becomes a little tricky when parameter has numeric or string type.
   * There we have these rules:
   *
   * string type: The second string argument is interpreted as default_value. The third as config entry.
   * numeric type: The first numeric argument is interpreted as default_value.
   */
  template<typename ... ARGS>
  tParameter(const ARGS&... args) : implementation(data_ports::tPortCreationInfo<T>(args...))
  {
    data_ports::tPortCreationInfo<T> creation_info(args...);
    this->wrapped->AddAnnotation(*(new internal::tParameterInfo()));
    SetConfigEntry(creation_info.config_entry);
  }

  /*!
   * Gets Port's current value.
   * (only available for 'cheaply copied' types)
   *
   * \param v unused dummy parameter for std::enable_if technique
   * \return Parameter's current value by-value.
   */
  template <bool AVAILABLE = cPASS_BY_VALUE>
  inline T Get(typename std::enable_if<AVAILABLE, void>::type* v = NULL) const
  {
    return implementation.Get();
  }

  /*!
   * \return Place in configuration file this parameter is configured from (nodes are separated with '/')
   */
  std::string GetConfigEntry()
  {
    internal::tParameterInfo* parameter_info = implementation.GetWrapped()->template GetAnnotation<internal::tParameterInfo>();
    return parameter_info->GetConfigEntry();
  }

  /*!
   * Gets Port's current value in buffer
   *
   * \return Buffer with port's current value with read lock.
   */
  inline data_ports::tPortDataPointer<const T> GetPointer() const
  {
    return implementation.GetPointer();
  }

  /*!
   * \param new_value New value of parameter
   */
  void Set(const T& new_value)
  {
    tPortImplementation::BrowserPublish(*implementation.GetWrapped(), new_value, rrlib::time::cNO_TIME);
  }

  /*!
   * \param config_entry New place in configuration file this parameter is configured from (nodes are separated with '/')
   */
  void SetConfigEntry(const std::string& config_entry)
  {
    internal::tParameterInfo* parameter_info = implementation.GetWrapped()->template GetAnnotation<internal::tParameterInfo>();
    parameter_info->SetConfigEntry(config_entry);
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! Parameter implementation */
  tImplementation implementation;
};

extern template class tParameter<int>;
extern template class tParameter<long long int>;
extern template class tParameter<float>;
extern template class tParameter<double>;
extern template class tParameter<std::string>;
extern template class tParameter<bool>;

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif
