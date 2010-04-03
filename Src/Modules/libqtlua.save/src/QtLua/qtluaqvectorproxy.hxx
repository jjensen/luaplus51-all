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

#ifndef QTLUAQVECTORPROXY_HXX_
#define QTLUAQVECTORPROXY_HXX_

#include "qtluauserdata.hxx"
#include "qtluaiterator.hxx"

namespace QtLua {

  template <class Container, bool resize>
  QVectorProxyRo<Container, resize>::QVectorProxyRo()
    : _vector(0)
  {
  }

  template <class Container, bool resize>
  QVectorProxyRo<Container, resize>::QVectorProxyRo(Container &vector)
    : _vector(&vector)
  {
  }

  template <class Container, bool resize>
  QVectorProxy<Container, resize>::QVectorProxy()
    : QVectorProxyRo<Container, resize>()
  {
  }

  template <class Container, bool resize>
  QVectorProxy<Container, resize>::QVectorProxy(Container &vector)
    : QVectorProxyRo<Container, resize>(vector)
  {
  }

  template <class Container, bool resize>
  void QVectorProxyRo<Container, resize>::set_container(Container *vector)
  {
    _vector = vector;
  }

  template <class Container, bool resize>
  Value QVectorProxyRo<Container, resize>::meta_index(State &ls, const Value &key)
  { 
    if (!_vector)
      return Value(ls);

    int index = (unsigned int)key.to_number() - 1;

    if (index >= 0 && index < _vector->size())
      return Value(ls, _vector->at(index));
    else
      return Value(ls);
  }

  template <class Container, bool resize>
  Value QVectorProxyRo<Container, resize>::meta_operation(State &ls, Operation op, const Value &a, const Value &b)
  {
    switch (op)
      {
      case OpLen:
	return Value(ls, _vector ? _vector->size() : 0);
      case OpUnm:
	return _vector ? Value(ls, *_vector) : Value(ls);
      default:
	return UserData::meta_operation(ls, op, a, b);
      }
  }

  template <class Container, bool resize>
  void QVectorProxy<Container, resize>::meta_newindex(State &ls, const Value &key, const Value &value)
  {
    if (!_vector)
      throw String("Can not write to null container.");

    int index = (unsigned int)key.to_number() - 1;

    if (index < 0)
      throw String("QVector index is out of bounds.");

    if (resize && value.type() == Value::TNil)
      {
	if (index < _vector->size())
	  _vector->remove(index);
      }
    else
      {
	if (index >= _vector->size())
	  {
	    if (resize)
	      _vector->resize(index + 1);
	    else
	      throw String("QVector index is out of bounds.");
	  }
	(*_vector)[index] = value;
      }
  }

  template <class Container, bool resize>
  Ref<Iterator> QVectorProxyRo<Container, resize>::new_iterator(State &ls)
  {
    if (!_vector)
      throw String("Can not iterate on null container.");

    return QTLUA_REFNEW(ProxyIterator, ls, *this);
  }

  template <class Container, bool resize>
  QVectorProxyRo<Container, resize>::ProxyIterator::ProxyIterator(State &ls, QVectorProxyRo::ptr proxy)
    : _ls(ls),
      _proxy(proxy),
      _it(0)
  {
  }

  template <class Container, bool resize>
  bool QVectorProxyRo<Container, resize>::ProxyIterator::more() const
  {
    return _proxy->_vector && _it < (unsigned int)_proxy->_vector->size();
  }

  template <class Container, bool resize>
  void QVectorProxyRo<Container, resize>::ProxyIterator::next()
  {
    _it++;
  }

  template <class Container, bool resize>
  Value QVectorProxyRo<Container, resize>::ProxyIterator::get_key() const
  {
    return Value(_ls, (int)_it);
  }

  template <class Container, bool resize>
  Value QVectorProxyRo<Container, resize>::ProxyIterator::get_value() const
  {
    return Value(_ls, _proxy->_vector->at(_it));
  }

  template <class Container, bool resize>
  ValueRef QVectorProxyRo<Container, resize>::ProxyIterator::get_value_ref()
  {
    return ValueRef(Value(_ls, _proxy), Value(_ls, (double)_it));
  }

}

#endif

