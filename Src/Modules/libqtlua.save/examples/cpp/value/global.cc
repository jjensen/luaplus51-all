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
							/* anchor i1 */
    QtLua::State state;
							/* anchor i2 */
    QtLua::State const & const_state = state;
							/* anchor end */

							/* anchor 1 */
    // Access global table from lua
    state.exec_statements("foo = 5");

    // Access global table from C++
    int foo = const_state["foo"];
							/* anchor end */
    std::cout << foo << std::endl;
							/* anchor 2 */
    // Access global table from lua
    state.exec_statements("foo = 5");

    // Access global table from C++
    state["bar"] = 5;
							/* anchor end */

  } catch (QtLua::String &e) {
    std::cerr << e.constData() << std::endl;
  }

}

