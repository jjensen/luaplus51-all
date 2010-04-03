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

#include <cstring>

#include <internal/QObjectWrapper>
#include <internal/Enum>
#include <internal/EnumIterator>

namespace QtLua {

  Enum::Enum(const QMetaObject *mo, int index)
    : Member(mo, index)
  { 
  }

  Value Enum::meta_index(State &ls, const Value &key)
  {
    return Value(ls, _mo->enumerator(_index).keyToValue(key.to_string().constData()));
  }

  Ref<Iterator> Enum::new_iterator(State &ls)
  {
    return QTLUA_REFNEW(EnumIterator, ls, _mo->enumerator(_index));
  }

  String Enum::get_value_str() const
  {
    QMetaEnum me = _mo->enumerator(_index);

    return String(me.scope()) + "::" + me.name();
  }

  void Enum::completion_patch(String &path, String &entry, int &offset)
  {
    entry += ".";
  }

}

