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


#ifndef QTLUAPLUGIN_HH_
#define QTLUAPLUGIN_HH_

#include <QPluginLoader>
#include <QMap>

#include "qtluaqhashproxy.hh"
#include "qtluastring.hh"
#include "qtluauserdata.hh"

namespace QtLua {

  class Function;

  /** @internal */
  typedef QMap<String, UserData::ptr> plugin_map_t;

  /**
   * @short QtLua plugin class
   * @header QtLua/Plugin
   * @module {Functions export}
   * @see PluginInterface
   *
   * This class allows easy development and loading of Qt plugins
   * which can be manipulated from lua scripts.
   *
   * These plugins must use the @ref PluginInterface interface. It may
   * have additional Qt plugin interfaces which can be queried with
   * the @ref api function from C++ code.
   *
   * These plugins may to contains @ref Function objects which can
   * be invoked from lua.
   *
   * @ref Function objects contained in plugin library must be
   * registered on the @ref Plugin object. This is done on @ref Plugin
   * creation by the @ref PluginInterface::register_members function
   * which must be implemented in the plugin to call the
   * @ref Function::__register_2__ function for each member to register.
   *
   * An internal @ref Plugin::Loader {plugin loader} object is
   * allocated and referenced by registered member @ref Function
   * objects to prevent plugin unloading until all registered members
   * get collected, even if the associated @ref Plugin object is
   * destroyed.
   *
   * The @ref Plugin object may be exposed to lua as 
   * a read only table like userdata object which contains registered
   * members; it also exposes the plugin name and description.
   * 
   * The @ref Plugin object can also be used to just load the plugin
   * and get a lua table containing all registered @ref Function
   * objects by using the @ref to_table function in C++ or the @tt -
   * operator in lua. In this case there is no need to expose or keep
   * the @ref Plugin object once the plugin has been loaded.
   *
   * The @ref QtLuaLib lua library provides a @tt{plugin()} lua
   * function which returns a @ref Plugin userdata object for a given plugin
   * file name. The platform dependent plugin file name extension will
   * be appended automatically. The returned @ref Plugin object may be
   * converted directly to a lua table using the @tt - lua operator.
   *
   * @section {Example}
   * Here is the code of an example plugin. The header file implements the Qt plugin interface:
   * @example examples/cpp/plugin/plugin.hh:1
   *
   * The plugin source file registers a @ref Function object:
   * @example examples/cpp/plugin/plugin.cc:1
   *
   * Here is a C++ example of plugin use:
   * @example examples/cpp/plugin/plugin_load.cc:1
   * @end section
   */

  class Plugin : public QHashProxyRo<plugin_map_t>
  {
    friend class Function;
  public:

    QTLUA_REFTYPE(Plugin);

    /** Load a new plugin */
    Plugin(const String &filename);

    /** Get plugin name */
    const String & get_name() const;

    /** Get plugin description */
    const String & get_description() const;

    /** Convert @ref Plugin object to lua table */
    Value to_table(State &ls) const;

    /** Get instance of the requested interface type. Return 0 if no
	such interface is available in the plugin. */
    template <class interface>
    inline interface *api() const;

    /** Get platform dependent plugin file name suffix */
    static const String & get_plugin_ext();

  private:

    /**
     * @short Ref counted plugin loader object (internal)
     * @internal 
     */
    struct Loader : public QPluginLoader, public UserData
    {
      QTLUA_REFTYPE(Loader);
      Loader(const String &filename);
      ~Loader();
    };

    String get_value_str() const;
    void completion_patch(String &path, String &entry, int &offset);

    plugin_map_t	_map;
    Loader::ptr		_loader;
  };
}

#endif

