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
/*!\file    plugins/parameters/tStaticParameter.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-30
 *
 * \brief   Contains tStaticParameter
 *
 * \b tStaticParameter
 *
 * Unlike "normal" parameters, static parameters cannot be changed while
 * a Finroc application is executing
 * (as this is not required, changing them is not thread-safe)
 * Thus, static paratemers are more or less construction parameters
 * of modules and groups.
 * They often influence the port structure of these modules and groups.
 */
//----------------------------------------------------------------------
#ifndef __plugins__parameters__tStaticParameter_h__
#define __plugins__parameters__tStaticParameter_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/parameters/internal/tStaticParameterImplementation.h"
#include "plugins/parameters/internal/tStaticParameterList.h"

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
//! Static Parameter.
/*!
 * Unlike "normal" parameters, static parameters cannot be changed while
 * a Finroc application is executing
 * (as this is not required, changing them is not thread-safe)
 * Thus, static paratemers are more or less construction parameters
 * of modules and groups.
 * They often influence the port structure of these modules and groups.
 */
template <typename T>
class tStaticParameter
{
#ifdef _LIB_RRLIB_XML_PRESENT_
  static_assert(rrlib::serialization::IsXMLSerializable<T>::value, "T has to be serializable to XML.");
#endif

protected:

  /*! Class that contains actual implementation of most functionality */
  typedef internal::tStaticParameterImplementation < T, data_ports::IsNumeric<T>::value && (!definitions::cSINGLE_THREADED) > tImplementation;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*! Type T */
  typedef T tDataType;

  /*! Bundles all possible constructor parameters of tStaticParameter */
  typedef internal::tParameterCreationInfo<T> tConstructorParameters;

  /*!
   * Constructor takes variadic argument list... just any properties you want to assign to parameter.
   *
   * The first string is interpreted as parameter name, the second possibly as config entry.
   * A framework element pointer is interpreted as parent.
   * tBounds<T> are parameter's bounds.
   * tUnit argument is parameter's unit.
   * const T& is interpreted as parameter's default value.
   * tChangeCallback can be specified - e.g. for immediate callback on parameter value change
   *
   * This becomes a little tricky when parameter has numeric or string type.
   * There we have these rules:
   *
   * string type: The second string argument is interpreted as default value. The third as config entry.
   * numeric type: The first numeric argument is interpreted as default value.
   */
  template <typename ... ARGS>
  tStaticParameter(const ARGS&... args) :
    implementation(NULL)
  {
    core::tPortWrapperBase::tConstructorArguments<internal::tParameterCreationInfo<T>> creation_info(args...);
    implementation = tImplementation::CreateInstance(creation_info, false);
    implementation->SetChangeCallbackMode(creation_info.change_callback);
    assert(creation_info.parent != NULL);
    internal::tStaticParameterList::GetOrCreate(*creation_info.parent).Add(*implementation);
  }

  /*!
   * Attach this static parameter to another one.
   * They will share the same value/buffer.
   *
   * \param other Other parameter to attach this one to. Use null or this to detach.
   */
  void AttachTo(tStaticParameter<T>& other)
  {
    implementation->AttachTo(other.implementation);
  }

  /*!
   * Attach to parameter in outer framework element (e.g. group).
   *
   * \param outer_parameter_attachment Name of outer parameter of finstructable group to configure parameter with.
   * (set by finstructable group containing module with this parameter)
   * \param create_outer Create outer parameter if it does not exist yet?
   */
  void AttachToOuterParameter(const std::string& outer_parameter_attachment = "", bool create_outer = true)
  {
    implementation->SetOuterParameterAttachment(outer_parameter_attachment.length() > 0 ? outer_parameter_attachment : implementation->GetName(), create_outer);
  }

  /*!
   * \return Current parameter value as reference
   * (value is deleted, when parameter is - which doesn't happen while a module is running)
   */
  T& Get() const
  {
    return implementation->Get();
  }

  /*!
   * \return Place in configuration file this parameter is configured from
   */
  std::string GetConfigEntry()
  {
    return implementation->GetConfigEntry();
  }

  /*!
   * \return Name of parameter
   */
  std::string GetName()
  {
    return implementation->GetName();
  }

  /*!
   * \return Has parameter changed since last call to "ResetChanged" (or creation).
   */
  inline bool HasChanged()
  {
    return implementation->HasChanged();
  }

  /*!
   * Reset "changed flag".
   * The current value will now be the one any new value is compared with when
   * checking whether value has changed.
   */
  inline void ResetChanged()
  {
    implementation->ResetChanged();
  }

  /*!
   * (same as SetValue)
   *
   * \param new_value New value
   */
  inline void Set(const T& new_value)
  {
    implementation->Set(new_value);
  }

  /*!
   * \param config_entry New place in configuration file this parameter is configured from (nodes are separated with '/')
   */
  void SetConfigEntry(const std::string& config_entry)
  {
    implementation->SetConfigEntry(config_entry);
  }

//----------------------------------------------------------------------
// Protected functions
//----------------------------------------------------------------------
protected:

  tStaticParameter() : implementation(NULL) {}

  /*!
   * \param Implementation to use (for any subclasses)
   */
  void SetImplementation(tImplementation* implementation)
  {
    this->implementation = implementation;
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! Static Parameter Implementation */
  tImplementation* implementation;

};

extern template class tStaticParameter<int>;
extern template class tStaticParameter<long long int>;
extern template class tStaticParameter<float>;
extern template class tStaticParameter<double>;
extern template class tStaticParameter<std::string>;
extern template class tStaticParameter<bool>;

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif
