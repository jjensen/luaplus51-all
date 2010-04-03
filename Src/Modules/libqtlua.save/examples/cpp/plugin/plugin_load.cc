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

#include <QtLua/State>
#include <QtLua/Plugin>

int main()
{
    try {
/* anchor 1 */
      QtLua::State state;
      state.openlib(QtLua::QtLuaLib);

      // forge plugin filename
      QtLua::String filename = QtLua::String("plugin") + QtLua::Plugin::get_plugin_ext();
 
      // load the plugin as a QtLua::Plugin userdata value
      state["plugin_userdata"] = QTLUA_REFNEW(QtLua::Plugin, filename);
      // access the QtLua::Plugin object and call the QtLua::Function object
      state.exec_statements("print(plugin_userdata.foo())");

      // load the plugin and convert the QtLua::Plugin object to lua table
      state["plugin_table"] = QtLua::Plugin(filename).to_table(state);
      // access the lua table and call the QtLua::Function object
      state.exec_statements("print(plugin_table.foo())");

/* anchor end */
    } catch (QtLua::String &e) {
      std::cerr << e.constData() << std::endl;
    }
}

