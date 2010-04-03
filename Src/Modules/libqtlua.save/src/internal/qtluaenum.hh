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


#ifndef QTLUAENUM_HH_
#define QTLUAENUM_HH_

#include <QMetaObject>
#include <QMetaEnum>

#include <internal/qtluamember.hh>

namespace QtLua {

/**
 * @short Qt enum wrapper class (internal)
 * @header internal/Enum
 * @module {QObject wrapping}
 * @internal
 *
 * This internal class implements the wrapper which give
 * access to enums of @ref QObject objects from lua.
 */

  class Enum : public Member
  {
  public:
    QTLUA_REFTYPE(Enum);

    Enum(const QMetaObject *mo, int index);

  private:
    Value meta_index(State &ls, const Value &key);
    Ref<Iterator> new_iterator(State &ls);
    String get_value_str() const;
    void completion_patch(String &path, String &entry, int &offset);
  };

}

#endif

