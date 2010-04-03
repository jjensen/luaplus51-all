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
#include <internal/QObjectWrapper>

#include <internal/MetaCache>
#include <internal/QObjectIterator>

namespace QtLua {

  QObjectIterator::QObjectIterator(State &ls, const QMetaObject *mo)
    : _ls(ls)
  {
    _cur = CurMember;
    _mc = &MetaCache::get_meta(mo);
    _it = _mc->get_member_table().begin();

    update();
  }

  QObjectIterator::QObjectIterator(State &ls, QObjectWrapper::ptr qow)
    : _ls(ls),
      _qow(qow)
  {
    _cur = CurChildren;
    _child_id = 0;

    QObject &obj = _qow->get_object();

    _mc = &MetaCache::get_meta(obj.metaObject());
    _it = _mc->get_member_table().begin();

    update();
  }

  bool QObjectIterator::more() const
  {
    return _cur != CurEnd;
  }

  void QObjectIterator::next()
  {
    switch (_cur)
      {
      case CurChildren:
	_child_id++;
	break;
      case CurMember:
	_it++;
	break;
      case CurEnd:
	std::abort();
      }

    update();
  }

  void QObjectIterator::update()
  {
    switch (_cur)
      {
      case CurChildren:
	if (_qow->_obj && _child_id < _qow->_obj->children().size())
	  return;

	_cur = CurMember;

      case CurMember:
	while (_it == _mc->get_member_table().end())
	  {
	    const QMetaObject *super = _mc->get_meta_object()->superClass();

	    if (!super)
	      {
		_cur = CurEnd;
		break;
	      }

	    _mc = &MetaCache::get_meta(super);
	    _it = _mc->get_member_table().begin();
	  }

      case CurEnd:
	break;
      }
  }

  Value QObjectIterator::get_key() const
  {
    switch (_cur)
      {
      case CurChildren:
	if (!_qow->_obj)
	  return Value(_ls);
	else
	  return Value(_ls, QObjectWrapper::qobject_name(*_qow->_obj->children().at(_child_id)));

      case CurMember:
	return Value(_ls, _it.key());

      default:
	std::abort();
      }
  }

  Value QObjectIterator::get_value() const
  {
    switch (_cur)
      {
      case CurChildren:
	if (!_qow->_obj)
	  return Value(_ls);
	else
	  return Value(_ls, QObjectWrapper::get_wrapper(_ls, _qow->_obj->children().at(_child_id)));

      case CurMember:
	return Value(_ls, _it.value());

      default:
	std::abort();
      }
  }

  ValueRef QObjectIterator::get_value_ref()
  {
    // Not used from lua script
    std::abort();
    /* jj - This is most certainly not correct. */
    Value table(_ls, _it.value());
    Value key(_ls, _it.value());
    return ValueRef(table, key);
  }

}

