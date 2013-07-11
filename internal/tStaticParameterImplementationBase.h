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
/*!\file    plugins/parameters/internal/tStaticParameterImplementationBase.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-30
 *
 * \brief   Contains tStaticParameterImplementationBase
 *
 * \b tStaticParameterImplementationBase
 *
 * Base class (without template parameter) for all static parameters.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__parameters__internal__tStaticParameterImplementationBase_h__
#define __plugins__parameters__internal__tStaticParameterImplementationBase_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/rtti/rtti.h"

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
class tStaticParameterList;

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Base class for all static parameters.
/*!
 * Base class (without template parameter) for all static parameters.
 */
class tStaticParameterImplementationBase : private rrlib::util::tNoncopyable
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * \param name Name of parameter
   * \param type DataType of parameter
   * \param constructor_prototype Is this a CreteModuleActionPrototype (no buffer will be allocated)
   */
  tStaticParameterImplementationBase(const std::string& name, rrlib::rtti::tType type, bool constructor_prototype, bool static_parameter_proxy = false, const std::string& config_entry = "");

  virtual ~tStaticParameterImplementationBase();

  /*!
   * Attach this static parameter to another one.
   * They will share the same value/buffer.
   *
   * \param other Other parameter to attach this one to. Use 'NULL' or 'this' to detach.
   */
  void AttachTo(tStaticParameterImplementationBase* other);

  /*!
   * (should be overridden by subclasses)
   * \return Deep copy of parameter (without value)
   */
  virtual tStaticParameterImplementationBase* DeepCopy()
  {
    return new tStaticParameterImplementationBase(name, type, false, false, config_entry);
  }

  void Deserialize(rrlib::serialization::tInputStream& is);

  void Deserialize(const rrlib::xml::tNode& node, bool finstruct_context);

  /*!
   * Deserializes value from stream
   *
   * \param is Input stream
   */
  void DeserializeValue(rrlib::serialization::tInputStream& is);

  /*!
   * \param result Result buffer for all attached parameters (including those from parameters this parameter is possibly (indirectly) attached to)
   */
  void GetAllAttachedParameters(std::vector<tStaticParameterImplementationBase*>& result);

  /*!
   * \return Place in Configuration tree, this parameter is configured from
   */
  std::string GetConfigEntry()
  {
    return config_entry;
  }

  const char* GetLogDescription()
  {
    return name.c_str();
  }

  /*!
   * \return Name of parameter
   */
  inline std::string GetName()
  {
    return name;
  }

  /*!
   * \return List that this structure parameter is member of
   */
  tStaticParameterList* GetParentList()
  {
    return parent_list;
  }

  /*!
   * \return DataType of parameter
   */
  inline rrlib::rtti::tType GetType()
  {
    return type;
  }

  /*!
   * \return Has parameter changed since last call to "ResetChanged" (or creation).
   */
  bool HasChanged();

  /*!
   * \return Is this a proxy for other static parameters? (only used in finstructable groups)
   */
  bool IsStaticParameterProxy() const
  {
    return static_parameter_proxy;
  }

  /*!
   * Load value (from any config file entries or command line or whereever)
   */
  void LoadValue();

  /*!
   * Reset "changed flag".
   * The current value will now be the one any new value is compared with when
   * checking whether value has changed.
   */
  void ResetChanged();

  void Serialize(rrlib::serialization::tOutputStream& os) const;

  void Serialize(rrlib::xml::tNode& node, bool finstruct_context) const;

  /*!
   * \param s Serialized as string
   */
  void Set(const std::string& s);

  /*!
   * \param config_entry Place in Configuration tree, this parameter is configured from.
   * Immediately loads this value when parent module is initialized.
   */
  void SetConfigEntry(const std::string& config_entry);

  /*!
   * \param outer_parameter_attachment Name of outer parameter of finstructable group to configure parameter with.
   * (set by finstructable group containing module with this parameter)
   * \param create_outer Create outer parameter if it does not exist yet?
   */
  void SetOuterParameterAttachment(const std::string& outer_parameter_attachment, bool create_outer);

  /*!
   * (Internal helper function to make expressions shorter)
   *
   * \return Value buffer to use
   */
  inline rrlib::rtti::tGenericObject* ValuePointer() const
  {
    return GetParameterWithBuffer().value.get();
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  friend class tStaticParameterList;

  /*! Name of parameter */
  std::string name;

  /*! DataType of parameter */
  rrlib::rtti::tType type;

  /*! Current parameter value (in CreateModuleAction-prototypes this is null) */
  std::unique_ptr<rrlib::rtti::tGenericObject> value;

  /*! Last parameter value (to detect whether value has changed) */
  std::unique_ptr<rrlib::rtti::tGenericObject> last_value;

  /*! Is current value enforced (typically hard-coded)? In this case, any config file entries or command line parameters are ignored */
  bool enforce_current_value;

  /*!
   * StaticParameterBase whose value buffer to use.
   * Typically, this is set to this.
   * However, it is possible to attach this parameter to another (outer) parameter.
   * In this case they share the same buffer: This parameter uses useValueOf.valPointer(), too.
   */
  tStaticParameterImplementationBase* use_value_of;

  /** List that this structure parameter is member of */
  tStaticParameterList* parent_list;

  /*! Index in parameter list */
  int list_index;

  /*!
   * Command line option to set this parameter
   * (set by finstructable group containing module with this parameter)
   */
  std::string command_line_option;

  /*!
   * Name of outer parameter if parameter is configured by static parameter of finstructable group
   * (usually set by finstructable group containing module with this parameter)
   */
  std::string outer_parameter_attachment;

  /*! Create outer parameter if it does not exist yet? (Otherwise an error message is displayed. Only true, when edited with finstruct.) */
  bool create_outer_parameter;

  /*!
   * Place in Configuration tree, this parameter is configured from (nodes are separated with '/')
   * (usually set by finstructable group containing module with this parameter)
   * (starting with '/' => absolute link - otherwise relative)
   */
  std::string config_entry;

  /*! Was configEntry set by finstruct? */
  bool config_entry_set_by_finstruct;

  /*! Is this a proxy for other static parameters? (as used in finstructable groups) */
  bool static_parameter_proxy;

  /*! List of attached parameters */
  std::vector<tStaticParameterImplementationBase*> attached_parameters;


  /*!
   * Create buffer of specified type
   * (and delete old buffer)
   *
   * \param type Type
   */
  void CreateBuffer(rrlib::rtti::tType type);

  /*!
   * Internal helper method to get parameter containing buffer we are using/sharing.
   *
   * \return Parameter containing buffer we are using/sharing.
   */
  tStaticParameterImplementationBase& GetParameterWithBuffer()
  {
    if (use_value_of == this)
    {
      return *this;
    }
    return use_value_of->GetParameterWithBuffer();
  }

  /*!
   * Internal helper method to get parameter containing buffer we are using/sharing.
   *
   * \return Parameter containing buffer we are using/sharing.
   */
  const tStaticParameterImplementationBase& GetParameterWithBuffer() const
  {
    if (use_value_of == this)
    {
      return *this;
    }
    return use_value_of->GetParameterWithBuffer();
  }

  /*!
   * Set commandLineOption and configEntry.
   * Check if they changed and possibly load value.
   */
  void UpdateAndPossiblyLoad(const std::string& command_line_option_tmp, const std::string& config_entry_tmp);

  /*!
   * Check whether change to outerParameterAttachment occured and perform any
   * changes required.
   *
   * \param outer_attachement_string_changed Has outer_parameter_attachment string changed?
   */
  void UpdateOuterParameterAttachment(bool outer_attachement_string_changed);
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif
