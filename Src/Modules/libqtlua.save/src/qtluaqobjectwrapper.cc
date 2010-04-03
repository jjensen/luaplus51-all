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

#include <QDebug>
#include <QObject>
#include <QMetaObject>
#include <QWidget>

#include <internal/QObjectWrapper>

#include <internal/Member>
#include <internal/Method>
#include <internal/MetaCache>
#include <internal/QObjectIterator>

#define assert_do(x) { bool res_ = (x); assert (((void)#x, res_)); }

namespace QtLua {

  static const int destroyindex = QObject::staticMetaObject.indexOfSignal("destroyed()");

  QObjectWrapper::QObjectWrapper(State &ls, QObject *obj)
    : _ls(ls),
      _obj(obj),
      _lua_next_slot(1),
      _reparent(true),
      _delete(obj && obj->parent() && get_wrapper(ls, obj->parent())->_delete)
  {
#ifdef QTLUA_QOBJECTWRAPPER_DEBUG
    qDebug() << "wrapper object created" << _obj;
#endif

    if (_obj)
      {
	assert_do(QMetaObject::connect(obj, destroyindex, this, metaObject()->methodCount() + 0));

	ls._whash.insert(obj, this);
	// increment reference count since we are binded to a qobject
	_inc();
      }
  }

  Ref<QObjectWrapper> QObjectWrapper::get_wrapper(State &ls, QObject *obj)
  {
#ifdef QTLUA_QOBJECTWRAPPER_DEBUG
    qDebug() << "wrapper object get" << obj;
#endif

    if (obj)
      {
	wrapper_hash_t::iterator i = ls._whash.find(obj);

	if (i != ls._whash.end())
	  return *i.value();
      }

    QObjectWrapper::ptr qow = QTLUA_REFNEW(QObjectWrapper, ls, obj);

    return qow;
  }

  Ref<QObjectWrapper> QObjectWrapper::get_wrapper(State &ls, QObject *obj, bool reparent, bool delete_)
  {
    QObjectWrapper::ptr qow = get_wrapper(ls, obj);

    qow->_reparent = reparent;
    qow->_delete = delete_;

    return qow;
  }

  void QObjectWrapper::obj_destroyed()
  {
#ifdef QTLUA_QOBJECTWRAPPER_DEBUG
    qDebug() << "wrapped object has been destroyed" << _obj;
#endif
    assert(_obj = sender());

    assert_do(_ls._whash.remove(_obj));
    _obj = 0;
    _drop();
  }

  QObjectWrapper::~QObjectWrapper()
  {
#ifdef QTLUA_QOBJECTWRAPPER_DEBUG
    qDebug() << "wrapper object detructor" << _obj;
#endif

    if (_obj)
      {
	assert_do(_ls._whash.remove(_obj));

	assert_do(QMetaObject::disconnect(_obj, destroyindex, this, metaObject()->methodCount() + 0));

	_lua_disconnect_all();

	if (_delete)
	  {
#ifdef QTLUA_QOBJECTWRAPPER_DEBUG
	    qDebug() << "wrapped object delete" << _obj;
#endif
	    delete _obj;
	  }
      }
  }

  void QObjectWrapper::ref_drop(int count)
  {
#ifdef QTLUA_QOBJECTWRAPPER_DEBUG
    qDebug() << "wrapper refdrop" << count << _delete << _obj;
#endif

    if (_obj && count == 1 && !_obj->parent() && _delete)
      _drop();
  }

  int QObjectWrapper::qt_metacall(QMetaObject::Call c, int id, void **qt_args)
  {
    id = QObject::qt_metacall(c, id, qt_args);

    if (id < 0 || c != QMetaObject::InvokeMetaMethod)
      return id;

    if (id == 0)
      {
	// slot 0 is reserved for object.destroyed() signal
	obj_destroyed();
	return -1;
      }

    lua_slots_hash_t::iterator i = _lua_slots.find(id);
    assert(i != _lua_slots.end());

    Value::List lua_args;

    // first arg is sender object
    if (_obj)
      {
	assert(_obj == sender());
	lua_args.push_back(Value(_ls, QObjectWrapper::get_wrapper(_ls, _obj)));
      }
    else
      lua_args.push_back(Value(_ls));

    // push more args from parameter type informations
    QMetaMethod mm = _obj->metaObject()->method(id >> 16);

    foreach(const QByteArray &pt, mm.parameterTypes())
      {
	qt_args++;
	lua_args.push_back(Member::raw_get_object(_ls, QMetaType::type(pt.constData()), *qt_args));
      }

    try {
      i.value().call(lua_args);
    } catch (const String &err) {
      qDebug() << "Error executing lua slot:" << err;
    }

    return -1;
  }

  void QObjectWrapper::_lua_connect(int sigindex, const Value &value)
  {
    switch (value.type())
      {
      case Value::TUserData:
      case Value::TFunction: {
	int slot_id = _lua_next_slot++ | (sigindex << 16);

	if (_lua_next_slot <= 65536 &&
	    QMetaObject::connect(_obj, sigindex, this, metaObject()->methodCount() + slot_id))
	  {
	    _lua_slots.insert(slot_id, value);
	    return;
	  }

	throw String("Unable to connect Qt signal.");
      }

      default:
	throw String("Can not connect lua::% type value to Qt signal.").arg(value.type_name());
      }
  }

  bool QObjectWrapper::_lua_disconnect(int sigindex, const Value &value)
  {
    lua_slots_hash_t::iterator i;

    for (i = _lua_slots.begin(); i != _lua_slots.end(); )
      {
	if (((i.key() >> 16) == sigindex) && (value == i.value()))
	  {
	    bool ok = QMetaObject::disconnect(_obj, sigindex, this, metaObject()->methodCount() + i.key());
	    assert(ok);
	    i = _lua_slots.erase(i);
	    return true;
	  }
	else
	  ++i;
      }

    return false;
  }

  void QObjectWrapper::_lua_disconnect_all(int sigindex)
  {
    lua_slots_hash_t::iterator i;

    for (i = _lua_slots.begin(); i != _lua_slots.end(); )
      {
	if ((i.key() >> 16) == sigindex)
	  {
	    bool ok = QMetaObject::disconnect(_obj, sigindex, this, metaObject()->methodCount() + i.key());
	    assert(ok);
	    i = _lua_slots.erase(i);
	  }
	else
	  ++i;
      }
  }

  void QObjectWrapper::_lua_disconnect_all()
  {
    lua_slots_hash_t::iterator i;

    for (i = _lua_slots.begin(); i != _lua_slots.end(); )
      {
	bool ok = QMetaObject::disconnect(_obj, i.key() >> 16, this, metaObject()->methodCount() + i.key());
	assert(ok);
	++i;
      }
    _lua_slots.clear();
  }

  QObject * QObjectWrapper::get_child(QObject &obj, const String &name)
  {
    foreach (QObject *child, obj.children())
      if (QObjectWrapper::qobject_name(*child) == name)
	return child;
    return 0;
  }

  Value QObjectWrapper::meta_index(State &ls, const Value &key)
  {
    QObject &obj = get_object();
    String skey = key.to_string();

    // handle children access
    if (QObject *child = get_child(obj, skey))
      return Value(ls, QObjectWrapper::get_wrapper(ls, child));

    // fallback to member read access
    Member::ptr m = MetaCache::get_meta(obj).get_member(skey);

    return m.valid() ? m->access(*this) : Value(ls);
  }

  void QObjectWrapper::reparent(QObject *parent)
  {
    assert(_obj);

    if (!_reparent)
      throw String("Parent change not allowed for '%' QObject.").arg(QObjectWrapper::qobject_name(*_obj));

    // FIXME handle non-widget reparent
    if (!_obj->isWidgetType() || (parent && !parent->isWidgetType()))
      throw String("Reparent of non QWidget objects not supported yet.");

    qobject_cast<QWidget*>(_obj)->setParent(qobject_cast<QWidget*>(parent));
  }

  void QObjectWrapper::meta_newindex(State &ls, const Value &key, const Value &value)
  {
    QObject &obj = get_object();
    String skey = key.to_string();

    // handle existing children access
    if (QObject *child = get_child(obj, skey))
      {
	QObjectWrapper::get_wrapper(ls, child)->reparent(0);
      }
    else
      {
	// fallback to member write access
	Member::ptr m = MetaCache::get_meta(obj).get_member(skey);

	if (m.valid())
	  {
	    m->assign(*this, value);
	    return;
	  }
      }

    // fallback to child insertion
    if (value.type() != Value::TNil)
      {
	QObjectWrapper::ptr qow = value.to_userdata_cast<QObjectWrapper>();
	QObject &child = qow->get_object();
	child.setObjectName(skey);
	qow->reparent(&obj);
      }
  }

  Ref<Iterator> QObjectWrapper::new_iterator(State &ls)
  {
    return QTLUA_REFNEW(QObjectIterator, ls, *this);
  }

  String QObjectWrapper::get_type_name() const
  {
    String res(UserData::get_type_name());

    if (_obj)
      res += "<" + String(_obj->metaObject()->className()) + ">";

    return res;
  }

  String QObjectWrapper::get_value_str() const
  {
    if (!_obj)
      return "(deleted)";
    QString addr;
    addr.sprintf("%p", _obj);
    return addr;
  }

  void QObjectWrapper::completion_patch(String &path, String &entry, int &offset)
  {
    entry += ".";
  }

  String QObjectWrapper::qobject_name(QObject &obj)
  {
    if (obj.objectName().isEmpty())
      {
	QString name;

	name.sprintf("%s_%lx", obj.metaObject()->className(), (unsigned long)&obj);
	obj.setObjectName(name.toLower());
      }

    return obj.objectName();
  }

}

