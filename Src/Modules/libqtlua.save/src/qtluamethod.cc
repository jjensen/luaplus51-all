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

#include <cstring>

#include <internal/QObjectWrapper>

#include <internal/Method>

namespace QtLua {

  Method::Method(const QMetaObject *mo, int index)
    : Member(mo, index)
  { 
  }

  Value::List Method::meta_call(State &ls, const Value::List &lua_args)
  {
    if (lua_args.size() < 1)
      throw String("Can't call method without object. (use ':' instead of '.')");

    QObjectWrapper::ptr qow = lua_args[0].to_userdata_null().dynamiccast<QObjectWrapper>();

    if (!qow.valid())
      throw String("Method first argument must be a QObjectWrapper. (use ':' instead of '.')");

    QObject &obj = qow->get_object();

    if (!check_class(obj.metaObject()))
      throw String("Method doesn't belong to passed object type.");

    QMetaMethod mm = _mo->method(_index);

    if (mm.methodType() != QMetaMethod::Slot)
      throw String("Can not call non-slot methods.");

    void *qt_args[11];
    int qt_tid[11];
    int tid, i = 0;

    try {

      // return value
      if (*mm.typeName())
	{
	  tid = qt_tid[0] = QMetaType::type(mm.typeName());

	  if (!tid)
	    throw String("Unsupported method return type, unable to convert % Qt type to lua value.").arg(mm.typeName());

	  qt_args[i++] = QMetaType::construct(tid);
	}
      else
	{
	  qt_args[i++] = 0;
	}

      // parameters
      foreach(const QByteArray &pt, mm.parameterTypes())
	{
	  assert(i < 11);

	  tid = qt_tid[i] = QMetaType::type(pt.constData());

	  if (!tid)
	    throw String("Unsupported method argument type, unable to convert lua value to % Qt type.").arg(String(pt));

	  void *arg = qt_args[i++] = QMetaType::construct(tid);

	  if (lua_args.size() >= i)
	    Member::raw_set_object(tid, arg, lua_args[i - 1]);
	}

      // actual invocation
      if (!obj.qt_metacall(QMetaObject::InvokeMetaMethod, _index, qt_args))
	throw String("Qt method invocation error.");

    } catch (...) {
      for (int j = i - 1; j >= 0; j--)
	if (qt_args[j])
	  QMetaType::destroy(qt_tid[j], qt_args[j]);
      throw;
    }

    Value::List ret_val;

    if (qt_args[0])
      ret_val.push_back(Member::raw_get_object(ls, qt_tid[0], qt_args[0]));

    for (int j = i - 1; j >= 0; j--)
      if (qt_args[j])
	QMetaType::destroy(qt_tid[j], qt_args[j]);

    return ret_val;
  }

  String Method::get_type_name() const
  {
    switch (_mo->method(_index).methodType())
      {
      case QMetaMethod::Signal:
	return Member::get_type_name() + "<signal>";
      case QMetaMethod::Slot:
	return Member::get_type_name() + "<slot>";
      default:
	return Member::get_type_name();
      }
  }

  String Method::get_value_str() const
  {
    QMetaMethod mm = _mo->method(_index);
    const char * t = mm.typeName();

    return String(*t ? t : "void") + " " + _mo->className() + "::" + mm.signature();
  }

  void Method::completion_patch(String &path, String &entry, int &offset)
  {
    switch (_mo->method(_index).methodType())
      {
      case QMetaMethod::Slot:
	// force method invokation operator
	if (!path.isEmpty())
	  path[path.size() - 1] = ':';

	entry += "()";
	offset--;

      default:
	break;

      }
  }

}

