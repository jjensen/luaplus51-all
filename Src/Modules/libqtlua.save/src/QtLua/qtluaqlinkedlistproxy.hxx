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

#ifndef QTLUAQLINKEDLISTPROXY_HXX_
#define QTLUAQLINKEDLISTPROXY_HXX_

#include "qtluauserdata.hxx"
#include "qtluaiterator.hxx"

namespace QtLua {

  template <class Container>
  QLinkedListProxy<Container>::QLinkedListProxy()
    : _linkedlist(0)
  {
  }

  template <class Container>
  QLinkedListProxy<Container>::QLinkedListProxy(Container &list)
    : _linkedlist(&list)
  {
  }

  template <class Container>
  void QLinkedListProxy<Container>::set_container(Container *list)
  {
    _linkedlist = list;
  }

  template <class Container>
  Ref<Iterator> QLinkedListProxy<Container>::new_iterator(State &ls)
  {
    if (!_linkedlist)
      throw String("Can not iterate on null container.");

    return QTLUA_REFNEW(ProxyIterator, ls, *this);
  }

  template <class Container>
  QLinkedListProxy<Container>::ProxyIterator::ProxyIterator(State &ls, QLinkedListProxy::ptr proxy)
    : _ls(ls),
      _proxy(proxy),
      _it(_proxy->_linkedlist->begin()),
      _i(0)
  {
  }

  template <class Container>
  bool QLinkedListProxy<Container>::ProxyIterator::more() const
  {
    return _proxy->_linkedlist && _it != _proxy->_linkedlist->end();
  }

  template <class Container>
  void QLinkedListProxy<Container>::ProxyIterator::next()
  {
    _it++;
    _i++;
  }

  template <class Container>
  Value QLinkedListProxy<Container>::ProxyIterator::get_key() const
  {
    return Value(_ls, (int)_i);
  }

  template <class Container>
  Value QLinkedListProxy<Container>::ProxyIterator::get_value() const
  {
    return Value(_ls, *_it);
  }

  template <class Container>
  ValueRef QLinkedListProxy<Container>::ProxyIterator::get_value_ref()
  {
    return ValueRef(Value(_ls, _proxy), Value(_ls, (double)_i));
  }

}

#endif

