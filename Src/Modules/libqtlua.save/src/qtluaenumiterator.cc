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

#include <cstdlib>

#include <QtLua/Value>
#include <QtLua/Iterator>

#include <internal/EnumIterator>

namespace QtLua {

  EnumIterator::EnumIterator(State &ls, QMetaEnum me)
    : _ls(ls),
      _me(me),
      _index(0)
  {
  }

  bool EnumIterator::more() const
  {
    return _index < _me.keyCount();
  }

  void EnumIterator::next()
  {
    assert(_index < _me.keyCount());
    _index++;
  }

  Value EnumIterator::get_key() const
  {
    return Value(_ls, _me.key(_index));
  }

  Value EnumIterator::get_value() const
  {
    return Value(_ls, _me.value(_index));
  }

  ValueRef EnumIterator::get_value_ref()
  {
    abort();
    /* jj - This is most certainly not correct. */
    Value table(_ls, _me.value(_index));
    Value key(_ls, _me.value(_index));
    return ValueRef(table, key);
  }

}

