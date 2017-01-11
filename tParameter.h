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
#include "plugins/parameters/internal/tParameterCreationInfo.h"
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
  typedef internal::tParameterImplementation < T, data_ports::IsNumeric<T>::value || std::is_same<bool, T>::value > tImplementation;

  typedef data_ports::api::tPortImplementation<T, data_ports::api::tPortImplementationTypeTrait<T>::type> tPortImplementation;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*! Should methods passing buffers by-value be available? */
  enum { cPASS_BY_VALUE = data_ports::tIsCheaplyCopiedType<T>::value };

  /*! Type T */
  typedef T tDataType;

  /*! Bundles all possible constructor parameters of tParameter */
  typedef internal::tParameterCreationInfo<T> tConstructorParameters;

  /*! Creates no wrapped parameter */
  tParameter() : implementation()
  {}

  /*!
   * Constructor takes variadic argument list... just any properties you want to assign to parameter.
   *
   * The first string is interpreted as parameter name, the second possibly as config entry.
   * A framework element pointer is interpreted as parent.
   * tFrameworkElement::tFlags arguments are interpreted as flags.
   * tBounds<T> are parameter's bounds.
   * tUnit argument is parameter's's unit.
   * const T& is interpreted as port's default value.
   * tParameterCreationInfo<T> argument is copied.
   *
   * This becomes a little tricky when parameter has numeric or string type.
   * There we have these rules:
   *
   * string type: The second string argument is interpreted as default_value. The third as config entry.
   * numeric type: The first numeric argument is interpreted as default_value.
   */
  template<typename ... ARGS>
  tParameter(const ARGS&... args) : implementation()
  {
    core::tPortWrapperBase::tConstructorArguments<data_ports::tPortCreationInfo<T>> creation_info(args...);
    if (!(creation_info.flags.Raw() & core::tFrameworkElementFlags(core::tFrameworkElementFlag::DELETED).Raw())) // do not create parameter, if deleted flag is set
    {
      implementation = tImplementation(creation_info);
      implementation.GetWrapped()->AddAnnotation(*(new internal::tParameterInfo()));
      SetConfigEntry(creation_info.config_entry);
    }
  }

  /*!
   * \param listener Listener to add (see tInputPort.h)
   */
  template <typename TListener>
  void AddListener(TListener& listener)
  {
    implementation.AddPortListener(listener);
  }
  template <typename TListener>
  void AddListenerSimple(TListener& listener)
  {
    implementation.AddPortListenerSimple(listener);
  }

  /*!
   * Gets parameter's current value.
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
   * Gets parameter's current value
   *
   * (Using this Get()-variant is efficient when using 'cheaply copied' types,
   * but can be extremely costly with large data types)
   *
   * \param result Buffer to (deep) copy parameter's current value to
   */
  inline void Get(T& result) const
  {
    return implementation.Get(result);
  }

  /*!
   * (throws a std::runtime_error if parameter is not bounded)
   *
   * \return Bounds as they are currently set
   */
  template <bool AVAILABLE = data_ports::IsBoundable<T>::value>
  inline typename std::enable_if<AVAILABLE, data_ports::tBounds<T>>::type GetBounds() const
  {
    return implementation.GetBounds();
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
   * \return Name of wrapped framework element (see tFrameworkElement::GetName())
   */
  inline const tString& GetName() const
  {
    return implementation.GetName();
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
   * \return Wrapped port. For rare case that someone really needs to access ports.
   */
  inline typename tImplementation::tPortBackend* GetWrapped() const
  {
    return implementation.GetWrapped();
  }

  /*!
   * \return Has parameter changed since last changed-flag-reset?
   */
  inline bool HasChanged() const
  {
    return implementation.HasChanged();
  }

  /*!
   * Initialize this parameter.
   * This must be called prior to using parameter.
   *
   * For parameters created in e.g. component constructor, this is done automatically.
   * For parameters created dynamically, this usually needs to be called.
   */
  inline void Init()
  {
    implementation.Init();
  }

  /*!
   * Reset changed flag.
   */
  inline void ResetChanged()
  {
    implementation.ResetChanged();
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
