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

#ifndef QTLUATABLEITERATOR_HH_
#define QTLUATABLEITERATOR_HH_

#include <QtLua/qtluaiterator.hh>

namespace QtLua {

/**
 * @short Lua table iterator class (internal)
 * @header internal/TableIterator
 * @module {Base}
 * @internal
 *
 * This internal class implements the iterator used to
 * iterate over lua tables and userdata values.
 *
 * @see Iterator
 */

class TableIterator : public Iterator
{
public:
  QTLUA_REFTYPE(TableIterator);

  TableIterator(lua_State *st, const Value &table);
  ~TableIterator();

private:
  bool more() const;
  void next();
  void fetch();
  Value get_key() const;
  Value get_value() const;
  ValueRef get_value_ref();

  lua_State *_st;
  Value _key;
  Value _value;
  bool _more;
};

}

#endif

