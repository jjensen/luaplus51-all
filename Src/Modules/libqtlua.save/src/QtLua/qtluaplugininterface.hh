/*
    This file is part of LibQtLua.

    LibQtLua is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    LibQtLua is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with LibQtLua.  If not, see <http://www.gnu.org/licenses/>.

    Copyright (C) 2008, Alexandre Becoulet <alexandre.becoulet@free.fr>

*/


#ifndef QTLUAPLUGINSINTERFACE_HH_
#define QTLUAPLUGINSINTERFACE_HH_

#include <QtPlugin>

#include "qtluastring.hh"
#include "qtluaitem.hh"

namespace QtLua {

  class Plugin;

  /**
   * @short QtLua plugin interface
   * @header QtLua/PluginInterface
   * @module {Functions export}
   * @see Plugin
   *
   * This class describes the interface which must be implemented
   * to write plugins compatible with the @ref Plugin class.
   */

  class PluginInterface
  {
  public:
    virtual ~PluginInterface() { }

    /** Get plugin name */
    virtual const String & get_name() const = 0;
    /** Get plugin description */
    virtual const String & get_description() const = 0;
    /** Register all plugin members, called on @ref Plugin initialization */
    virtual void register_members(Plugin &plugin) = 0;
  };

}

Q_DECLARE_INTERFACE(QtLua::PluginInterface, "QtLua.PluginInterface/1.0")

#endif

