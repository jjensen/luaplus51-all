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

#ifndef QTLUAENUMITERATOR_HH_
#define QTLUAENUMITERATOR_HH_

#include <QtLua/qtluaiterator.hh>

#include <QMetaEnum>

namespace QtLua {

/**
 * @short Qt enum iterator class (internal)
 * @header internal/EnumIterator
 * @module {QObject wrapping}
 * @internal
 *
 * This internal class implements the iterator used to
 * iterate over values of a Qt meta enum.
 *
 * @see Iterator
 * @see Enum
 */

class EnumIterator : public Iterator
{
public:
  QTLUA_REFTYPE(EnumIterator);

  EnumIterator(State &ls, QMetaEnum me);

private:
  bool more() const;
  void next();
  Value get_key() const;
  Value get_value() const;
  ValueRef get_value_ref();

  State &_ls;
  QMetaEnum _me;
  int _index;
};

}

#endif

