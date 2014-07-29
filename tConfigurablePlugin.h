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
/*!\file    plugins/parameters/tConfigurablePlugin.h
 *
 * \author  Max Reichardt
 *
 * \date    2014-03-11
 *
 * \brief   Contains tConfigurablePlugin
 *
 * \b tConfigurablePlugin
 *
 * Base class for plugins that can be configured
 * (using (static) parameters, config files, and the command line)
 */
//----------------------------------------------------------------------
#ifndef __plugins__parameters__tConfigurablePlugin_h__
#define __plugins__parameters__tConfigurablePlugin_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "core/tPlugin.h"
#include "core/tFrameworkElement.h"

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/parameters/tParameter.h"
#include "plugins/parameters/tStaticParameter.h"

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
//! Configurable Plugin
/*!
 * Base class for plugins that can be configured
 * (using (static) parameters, config files, and the command line)
 */
class tConfigurablePlugin : public core::tPlugin
{
  class tParameterBase;

  template <typename T, class BASE>
  class tParameterImplementation;

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * \param name Unique name of plugin. On Linux platforms, it should be identical with repository and .so file names (e.g. "tcp" for finroc_plugins_tcp and libfinroc_plugins_tcp.so).
   */
  tConfigurablePlugin(const char* name);

  /**
   * Parameter classes to use in plugin.
   * They are non-copyable and should be created as plain member variables of a plugin.
   * Use e.g. unique_ptr to put them in a vector.
   *
   * The first constructor argument must be a pointer to this plugin.
   * Config entries may not be nested.
   * Apart from that, they are used the same way as the plain parameter classes.
   * Constructors take a variadic argument list... just any properties you want to assign to parameter.
   *
   * A string is interpreted as parameter name; Any further string as config entry
   * A framework element pointer is interpreted as parent.
   * tFrameworkElement::tFlags arguments are interpreted as flags.
   * A tQueueSettings argument creates an input queue with the specified settings (not to be used with parameters)
   * tBounds<T> are parameters's bounds.
   * tUnit argument is parameters's unit.
   * const T& is interpreted as parameters's default value.
   * tPortCreationInfo<T> argument is copied. This is only allowed as first argument.
   *
   * This becomes a little tricky when T is a string type. There we have these rules:
   * A String not provided as first argument is interpreted as default value.
   * Any further string is interpreted as config entry.
   */
  template <typename T>
  using tParameter = tParameterImplementation<T, finroc::parameters::tParameter<T>>;

  template <typename T>
  using tStaticParameter = tParameterImplementation<T, finroc::parameters::tStaticParameter<T>>;

  /*!
   * \return True after Init() has been called (true when Init(tNode) is called)
   */
  bool IsInitialized() const
  {
    return initialized;
  }

#ifdef _LIB_RRLIB_XML_PRESENT_
  /*!
   * \return Pointer to configuration file for loading and configuring configurable plugins if one was set and found. nullptr otherwise.
   */
  static rrlib::xml::tNode* GetConfigRootNode();

  /*!
   * Returns XML node to get default parameter value from - if such a node exists
   *
   * \param config_entry Config entry of parameter
   * \return Node for specified config entry if one exists - otherwise nullptr
   */
  rrlib::xml::tNode* GetParameterNode(const std::string& config_entry);

  /*!
   * \returns Return XML node to configure plugin from if one such node exists. nullptr otherwise.
   */
  rrlib::xml::tNode* GetPluginConfigNode();

#endif

  /*!
   * Set configuration file to use for loading and configuring configurable plugins.
   * This must be called before tRuntimeEnvironment::GetInstance() to have an effect
   * (if configurable plugins were already initialized, a warning is displayed).
   *
   * \param file_name File name of config file to use (loaded if it exists)
   * \param root_node Path to node in config file that is the root node
   */
  static void SetConfigFile(const std::string& file_name);

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! True after Init() has been called (true when Init(tNode) is called) */
  bool initialized;

  /*! Contains vector with Finroc elements to be created */
  std::vector<tParameterBase*> elements_to_create;

  /*! Framework element that contains parameters */
  core::tFrameworkElement* parameter_element;


  /*! Framework element to attach parameters to */
  core::tFrameworkElement& GetParameterElement();


  // should not be overriden again (see below)
  virtual void Init() override;

#ifdef _LIB_RRLIB_XML_PRESENT_
  /*!
   * Called on initialization (after parameters have been loaded)
   * Should be overridden instead of plain Init()
   *
   * \param config_node Configuration node if one for this plugin was provided
   */
  virtual void Init(rrlib::xml::tNode* config_node) = 0;
#endif


  /*!
   * Secondary generic base class for parameters (required for deferred creation)
   */
  class tParameterBase : public rrlib::util::tNoncopyable
  {
  public:
    /*! Reference to plugin */
    tConfigurablePlugin& plugin;

    tParameterBase(tConfigurablePlugin& plugin) : plugin(plugin) {}

    virtual void CreateFinrocElement() = 0;
  };

  /*!
   * Generic implementation of plugin parameters (typedef'd later)
   */
  template <typename T, class BASE>
  class tParameterImplementation : public BASE, public tParameterBase
  {
    typedef core::tPortWrapperBase::tConstructorArguments<internal::tParameterCreationInfo<T>> tCreationInfo;

  public:
    template<typename ... ARGS>
    explicit tParameterImplementation(tConfigurablePlugin* plugin, const ARGS&... args)
      : BASE(),
        tParameterBase(*plugin)
    {
      creation_info.reset(new tCreationInfo(args...));
      if (plugin->IsInitialized())
      {
        CreateFinrocElement();
        creation_info.reset();
      }
      else
      {
        plugin->elements_to_create.emplace_back(this);
      }
    }

    void Set(const T& t)
    {
      if (!creation_info)
      {
        BASE::Set(t);
      }
      else
      {
        creation_info->SetDefault(t, true);
      }
    }

  private:

    std::unique_ptr<tCreationInfo> creation_info;

    virtual void CreateFinrocElement() override
    {
      creation_info->parent = &(this->plugin.GetParameterElement());
      static_cast<BASE&>(*this) = BASE(static_cast<tCreationInfo&>(*creation_info));
      creation_info.reset();
#ifdef _LIB_RRLIB_XML_PRESENT_
      rrlib::xml::tNode* node = this->plugin.GetParameterNode(this->GetConfigEntry().length() ? this->GetConfigEntry() : this->GetName());
      if (node)
      {
        T t;
        (*node) >> t;
        Set(t);
      }
#endif
      this->SetConfigEntry("");
    }
  };
};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif
