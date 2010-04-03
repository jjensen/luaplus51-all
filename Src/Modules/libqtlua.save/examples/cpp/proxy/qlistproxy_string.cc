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

// This example show how to use a QListProxy object to access a QList
// object from lua script.

#include <iostream>

#include <QList>

#include <QtLua/State>
#include <QtLua/QListProxy>

int main()
{
  try {
							/* anchor 1 */
    typedef QList<QtLua::String> Container;

    // QList we want to access from lua
    Container list;

    // List proxy which provides access to our QList from lua
    QtLua::QListProxy<Container> proxy(list);
							/* anchor 2 */

    QtLua::State state;
    state.openlib(QtLua::QtLuaLib);

    // Declare a lua global variable using our QList proxy
    state["list"] = proxy;

    // Add values in QList
    list << "foo" << "bar" << "FOO";

    // Append an element to the QList from lua using the proxy object
    state.exec_statements("list[4] = \"BAR\" ");
    state.exec_statements("list[5] = \"TEST\" ");

    // Read/Write element from lua
    state.exec_statements("list[1] = list[1]..list[2]");

    // Delete element from lua
    state.exec_statements("list[3] = nil");

							/* anchor 3 */
    // Read back values in modified QList
    foreach(const QtLua::String &s, list)
      std::cout << s.constData() << std::endl;

    // Iterate through QList from lua script too
    state.exec_statements("for key, value in each(list) do print(key, value) end");
							/* anchor end */

  } catch (QtLua::String &e) {
    std::cerr << e.constData() << std::endl;
  }

}

