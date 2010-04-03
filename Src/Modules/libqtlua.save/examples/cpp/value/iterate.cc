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

#include <iostream>

#include <QDebug>

#include <QtLua/State>
#include <QtLua/Value>

int main()
{
  try {
							/* anchor 1 */
    QtLua::State state;

    // New lua table value
    state.exec_statements("table = { a = 1, b = 2, c = 3 }");

    QtLua::Value table = state["table"];

    // Iterate over lua table from C++ code
    for (QtLua::Value::const_iterator i = table.begin(); i != table.end(); i++)
      qDebug() << i.key().to_string_p()
	       << i.value().to_string_p();
							/* anchor end */

							/* anchor 3 */
    state.openlib(QtLua::QtLuaLib);

    // Iterate from lua code
    state.exec_statements("for key, value in each(table) do print(key, value) end");
							/* anchor end */

							/* anchor 2 */
    // Modify lua table from C++ code
    for (QtLua::Value::iterator i = table.begin(); i != table.end(); i++)
      i.value() = QtLua::Value(state, "foo");
							/* anchor end */

    for (QtLua::Value::const_iterator i = table.begin(); i != table.end(); i++)
      qDebug() << i.key().to_string_p()
	       << i.value().to_string_p();

  } catch (QtLua::String &e) {
    std::cerr << e.constData() << std::endl;
  }

}

