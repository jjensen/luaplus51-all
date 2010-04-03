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

#ifndef QTLUALISTITERATOR_HH_
#define QTLUALISTITERATOR_HH_

#include <QtLua/qtluaiterator.hh>
#include <QtLua/qtlualistitem.hh>

namespace QtLua {

/**
 * @short Qt Model/View list item iterator class (internal)
 * @header internal/ListIterator
 * @module {Model/View}
 * @internal
 *
 * This internal class implements the iterator used to
 * iterate over @ref ListItem objects.
 *
 * @see Iterator
 */

class ListIterator : public Iterator
{
public:
  QTLUA_REFTYPE(ListIterator);

  ListIterator(State &ls, ListItem::ptr list);

private:
  bool more() const;
  void next();
  Value get_key() const;
  Value get_value() const;
  ValueRef get_value_ref();

  State &_ls;
  ListItem::ptr _list;
  QVector<Item::ptr>::const_iterator _it;
};

}

#endif

