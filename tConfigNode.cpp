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
/*!\file    plugins/parameters/tConfigNode.cpp
 *
 * \author  Max Reichardt
 *
 * \date    2012-11-29
 *
 */
//----------------------------------------------------------------------
#include "plugins/parameters/tConfigNode.h"

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------
#include "plugins/parameters/tConfigFile.h"
#include "plugins/parameters/internal/tStaticParameterList.h"

//----------------------------------------------------------------------
// Debugging
//----------------------------------------------------------------------
#include <cassert>

//----------------------------------------------------------------------
// Namespace usage
//----------------------------------------------------------------------

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
// Const values
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------

tConfigNode::tConfigNode(const std::string& node) :
  node(node)
{
}

std::string tConfigNode::GetConfigNode(core::tFrameworkElement& fe)
{
  tConfigFile* cf = tConfigFile::Find(fe);
  if (cf == NULL)
  {
    return "";
  }

  std::string result = "";
  core::tFrameworkElement* fe_ptr = &fe;
  while (true)
  {

    tConfigNode* cn = fe_ptr->GetAnnotation<tConfigNode>();
    if (cn)
    {
      result = cn->node + (((*cn->node.rbegin()) == '/') ? "" : "/") + result;
      if (cn->node[0] == '/')
      {
        return result;
      }
    }

    if (fe_ptr == cf->GetAnnotated<core::tFrameworkElement>())
    {
      return result;
    }

    fe_ptr = fe_ptr->GetParent();
  }
}

std::string tConfigNode::GetFullConfigEntry(core::tFrameworkElement& parent, const std::string& config_entry)
{
  if (config_entry[0] == '/')
  {
    return config_entry;
  }
  std::string node = GetConfigNode(parent);
  return node + (((*node.rbegin()) == '/') ? "" : "/") + config_entry;
}

void tConfigNode::SetConfigNode(core::tFrameworkElement& fe, const std::string& node)
{
  rrlib::thread::tLock lock(fe.GetStructureMutex());
  tConfigNode* cn = fe.GetAnnotation<tConfigNode>();
  if (cn != NULL)
  {
    if (cn->node == node)
    {
      return;
    }
    cn->node = node;
  }
  else
  {
    cn = new tConfigNode(node);
    fe.AddAnnotation(*cn);
  }

  // reevaluate static parameters
  internal::tStaticParameterList::DoStaticParameterEvaluation(fe);

  // reload parameters
  if (fe.IsReady())
  {
    tConfigFile* cf = tConfigFile::Find(fe);
    if (cf != NULL)
    {
      cf->LoadParameterValues(fe);
    }
  }
}

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}
