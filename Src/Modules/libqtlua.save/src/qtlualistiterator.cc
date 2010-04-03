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

#include <QtLua/Value>
#include <QtLua/Item>
#include <QtLua/Iterator>
#include <QtLua/ListItem>

#include <internal/ListIterator>

namespace QtLua {

  ListIterator::ListIterator(State &ls, ListItem::ptr list)
    : _ls(ls),
      _list(list),
      _it(_list->get_list().begin())
  {
  }

  bool ListIterator::more() const
  {
    return _it != _list->get_list().end();
  }

  void ListIterator::next()
  {
    _it++;
  }

  Value ListIterator::get_key() const
  {
    return Value(_ls, (*_it)->get_name());
  }

  Value ListIterator::get_value() const
  {
    return Value(_ls, *_it);
  }

  ValueRef ListIterator::get_value_ref()
  {
    return ValueRef(Value(_ls, _list),
		    Value(_ls, (*_it)->get_name()));
  }

}

