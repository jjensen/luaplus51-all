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
#include <QtLua/Function>

int main()
{
  try {

    /* anchor 1 */
    static class : public QtLua::Function
    {
      /* anchor 6 */
      QtLua::Value::List meta_call(QtLua::State &ls, const QtLua::Value::List &args)
      {
	/* anchor 5 */
	QtLua::String a = get_arg<QtLua::String>(args, 0);
	int           b = get_arg<int>(args, 1, 42);

	/* anchor 6 */
	return QtLua::Value(ls, "foo");
      }
      /* anchor 2 */
      QtLua::String get_description() const
      {
	return "This function just returns \"foo\"";
      }

      QtLua::String get_help() const
      {
	return ("usage: foo()");
      }
      /* anchor 3 */
    } foo_function;

    /* anchor 4 */
    QtLua::State state;

    foo_function.register_(state, "bar.foo");
    /* anchor end */

  } catch (QtLua::String &e) {
    std::cerr << e.constData() << std::endl;
  }

  return 0;
}

