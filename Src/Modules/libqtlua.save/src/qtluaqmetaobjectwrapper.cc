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

#include <QObject>
#include <QMetaObject>

#include <internal/QMetaObjectWrapper>
#include <internal/QObjectIterator>
#include <internal/MetaCache>
#include <internal/Member>

namespace QtLua {

  QMetaObjectWrapper::QMetaObjectWrapper(const QMetaObject *mo)
    : _mo(mo)
  {
  }

  Value QMetaObjectWrapper::meta_index(State &ls, const Value &key)
  {
    Member::ptr m = MetaCache::get_meta(_mo).get_member(key.to_string());

    return m.valid() ? Value(ls, m) : Value(ls);
  }

  Ref<Iterator> QMetaObjectWrapper::new_iterator(State &ls)
  {
    return QTLUA_REFNEW(QObjectIterator, ls, _mo);
  }

  void QMetaObjectWrapper::completion_patch(String &path, String &entry, int &offset)
  {
    entry += ".";
  }

  String QMetaObjectWrapper::get_value_str() const
  {
    String res(_mo->className());

    if (_mo->superClass())
      res += String(" : public ") + _mo->superClass()->className();

    return res;
  }

};

