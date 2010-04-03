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

/* example */
#include <QtLua/State>
#include <QtLua/Function>

							/* anchor 1 */
class MyObject : public QtLua::UserData
{
public:
  QTLUA_REFTYPE(MyObject);

  MyObject(int a)
  : a_(a) {}

private:
  int a_;
};

							/* anchor 2 */
MyObject::ptr my;
							/* anchor end */
int main()
{
  QtLua::State ls;

  /* anchor end */
  try {
							/* anchor 3 */
    QtLua::UserData::ptr ud = QTLUA_REFNEW(QtLua::UserData, );
							/* anchor 4 */
    ls["my_global_var"] = ud;
							/* anchor 5 */
    MyObject::ptr my = QTLUA_REFNEW(MyObject, 42);
							/* anchor end */
 } catch (QtLua::String &e) {
    std::cerr << e.constData() << std::endl;
  }

  return 0;
}

