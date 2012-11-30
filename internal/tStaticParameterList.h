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
/*!\file    plugins/parameters/internal/tStaticParameterList.h
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-30
 *
 * \brief   Contains tStaticParameterList
 *
 * \b tStaticParameterList
 *
 * List of static parameters that is attached to framework element
 * the static parameters belong to.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__parameters__internal__tStaticParameterList_h__
#define __plugins__parameters__internal__tStaticParameterList_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------
#include "rrlib/serialization/serialization.h"
#include "core/tFrameworkElement.h"

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
class tStaticParameterImplementationBase;

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! List of static parameters
/*!
 * List of static parameters that is attached to framework element
 * the static parameters belong to.
 */
class tStaticParameterList : public core::tAnnotation
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  tStaticParameterList();

  virtual ~tStaticParameterList();


  /*!
   * Add parameter to list
   *
   * \param param Parameter
   */
  void Add(tStaticParameterImplementationBase& param);

  virtual void AnnotatedObjectInitialized(); // TODO: mark as override in gcc 4.7

  /*!
   * XML Deserialization implementation.
   */
  void Deserialize(const rrlib::xml::tNode& node, bool finstruct_context);

  /*!
   * Trigger evaluation of static parameters in this framework element and all of its children.
   * (This must never be called when thread in surrounding thread container is running.)
   *
   * \param fe Framework element of interest
   */
  static void DoStaticParameterEvaluation(core::tFrameworkElement& fe);

  /*!
   * \param i Index
   * \return Parameter with specified index
   */
  inline tStaticParameterImplementationBase& Get(size_t i) const
  {
    return *parameters[i];
  }

  core::tFrameworkElement* GetAnnotated()
  {
    return tAnnotation::GetAnnotated<core::tFrameworkElement>();
  }

  /*!
   * \return Index of CreateModuleAction that was used to create framework element
   */
  inline int GetCreateAction() const
  {
    return create_action;
  }

  /*!
   * Get or create StaticParameterList for Framework element
   *
   * \param fe Framework element
   * \return StaticParameterList
   */
  static tStaticParameterList& GetOrCreate(core::tFrameworkElement& fe);

  /*!
   * XML Serialization implementation.
   */
  void Serialize(rrlib::xml::tNode& node, bool finstruct_context) const;

  /*!
   * \param create_action CreateModuleAction that was used to create framework element
   */
  inline void SetCreateAction(int create_action)
  {
    assert((this->create_action == -1));
    this->create_action = create_action;
  }

  /*!
   * \return size of list
   */
  inline size_t Size() const
  {
    return parameters.size();
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*! List of parameters */
  std::vector<tStaticParameterImplementationBase*> parameters;

  /*!
   * Index of CreateModuleAction that was used to create framework element
   * (typically only set when created with finstruct)
   */
  int create_action;


  /*! Clear list (deletes parameters) */
  void Clear();
};

rrlib::serialization::tOutputStream& operator << (rrlib::serialization::tOutputStream& stream, const tStaticParameterList& list);
rrlib::serialization::tInputStream& operator >> (rrlib::serialization::tInputStream& stream, tStaticParameterList& list);
rrlib::xml::tNode& operator << (rrlib::xml::tNode& node, const tStaticParameterList& list);
const rrlib::xml::tNode& operator >> (const rrlib::xml::tNode& node, tStaticParameterList& list);

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
}


#endif
