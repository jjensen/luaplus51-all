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

// This example show how to use a QHashProxy object to access a QMap
// object from lua script.

#include <iostream>

#include <QMap>

#include <QtLua/State>
#include <QtLua/QHashProxy>
 
int main()
{
  try {
							/* anchor 1 */
    typedef QMap<QtLua::String, QtLua::String> Container;

    // QMap we want to access from lua
    Container map;

    // Map proxy which provides access to our QMap from lua
    QtLua::QHashProxy<Container> proxy(map);
							/* anchor 2 */

    QtLua::State state;
    state.openlib(QtLua::QtLuaLib);

    // Declare a lua global variable using our map proxy
    state["map"] = proxy;

    // Insert a value in QMap
    map["key1"] = "value";

    // Read/Write in QMap from lua using the proxy object
    state.exec_statements("map.key2 = map.key1");

							/* anchor 3 */
    // Read back value in QMap inserted from lua script
    std::cout << map["key2"].constData() << std::endl;

    // Remove key2 entry from lua script
    state.exec_statements("map.key2 = nil");

    // Iterate through QMap from lua script
    state.exec_statements("for key, value in each(map) do print(key, value) end");
							/* anchor end */
  } catch (QtLua::String &e) {
    std::cerr << e.constData() << std::endl;
  }

}
