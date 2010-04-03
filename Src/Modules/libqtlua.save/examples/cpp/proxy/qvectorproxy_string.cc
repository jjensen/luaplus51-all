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

// This example show how to use a QVectorProxy object to access a QVector
// object from lua script.

#include <iostream>

#include <QVector>

#include <QtLua/State>
#include <QtLua/QVectorProxy>

int main()
{
  try {
							/* anchor 1 */
    typedef QVector<QtLua::String> Container;

    // QVector we want to access from lua
    Container vector(1);

    // Vector proxy which provides access to our QVector from lua.
    // Our proxy is allowed to resize the QVector.
    QtLua::QVectorProxy<Container, true> proxy(vector);
							/* anchor 2 */

    QtLua::State state;
    state.openlib(QtLua::QtLuaLib);

    // Declare a lua global variable using our QVector proxy
    state["vector"] = proxy;

    // Set a value in QVector
    vector[0] = "foo";

    // Read/Write/Resize QVector from lua using the proxy object
    state.exec_statements("vector[2] = vector[1]..\"bar\" ");

							/* anchor 3 */
    // Read back value in QVector modified from lua script
    std::cout << vector[1].constData() << std::endl;

    // Remove entry 0 sizing down the QVector
    state.exec_statements("vector[1] = nil");

    // Iterate through QVector from lua script
    state.exec_statements("for key, value in each(vector) do print(key, value) end");
							/* anchor end */

  } catch (QtLua::String &e) {
    std::cerr << e.constData() << std::endl;
  }

}

