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

#ifndef QTLUAQOBJECTITERATOR_HH_
#define QTLUAQOBJECTITERATOR_HH_

#include <QtLua/qtluaiterator.hh>
#include <QtLua/Ref>

#include <internal/MetaCache>

namespace QtLua {

  class State;
  class QObjectWrapper;

/**
 * @short Lua QObject children and meta members iterator class (internal)
 * @header internal/QObjectIterator
 * @module {QObject wrapping}
 * @internal
 *
 * This internal class implements the iterator used to
 * iterate over @ref QObject children and methods, properties, enums
 * exposed by the associated @ref QMetaObject.
 *
 * @see Iterator
 */

class QObjectIterator : public Iterator
{
public:
  QTLUA_REFTYPE(QObjectIterator);

  QObjectIterator(State &ls, Ref<QObjectWrapper> qow);
  QObjectIterator(State &ls, const QMetaObject *mo);

private:
  bool more() const;
  void next();
  void update();
  Value get_key() const;
  Value get_value() const;
  ValueRef get_value_ref();

  enum Current
    {
      CurChildren,
      CurMember,
      CurEnd,
    };

  State &_ls;
  Ref<QObjectWrapper> _qow;
  MetaCache *_mc;
  Current _cur;
  member_cache_t::const_iterator _it;
  int _child_id;
};

}

#endif

