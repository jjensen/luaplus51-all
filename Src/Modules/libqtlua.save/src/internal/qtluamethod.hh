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


#ifndef QTLUAMETHOD_HH_
#define QTLUAMETHOD_HH_

#include <QMetaObject>
#include <QMetaMethod>

#include <internal/qtluamember.hh>

namespace QtLua {

/**
 * @short Qt method wrapper class (internal)
 * @header internal/Property
 * @module {QObject wrapping}
 * @internal
 *
 * This internal class implements the wrapper which enables invocation
 * of methods of @ref QObject objects from lua.
 */
  class Method : public Member
  {
  public:
    QTLUA_REFTYPE(Method);

    Method(const QMetaObject *mo, int index);

  private:
    Value::List meta_call(State &ls, const Value::List &args);
    String get_type_name() const;
    String get_value_str() const;
    void completion_patch(String &path, String &entry, int &offset);
  };

}

#endif

