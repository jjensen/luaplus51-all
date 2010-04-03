
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

#include <QSize>
#include <QSizeF>
#include <QRect>
#include <QRectF>
#include <QPoint>
#include <QPointF>
#include <QMetaObject>
#include <QMetaType>
#include <QWidget>

#include <QtLua/String>
#include <internal/QObjectWrapper>

#include <internal/Member>

namespace QtLua {

  void Member::assign(QObjectWrapper &obj, const Value &value)
  {
    throw String("Can not assign value to '%' member").arg(get_type_name());
  }

  Value Member::access(QObjectWrapper &qow)
  {
    return Value(qow.get_state(), *this);
  }

  bool Member::check_class(const QMetaObject *mo) const
  {
    const QMetaObject *m = mo;

    while (m)
      {
	if (!strcmp(_mo->className(), m->className()))
	  return true;
	m = m->superClass();
      }

    return false;
  }

  static int ud_ref_type = qRegisterMetaType<Ref<UserData> >("Ref<UserData>");

  Value Member::raw_get_object(State &ls, int type, const void *data)
  {
    switch (type)
      {
      case QMetaType::Bool:
	return Value(ls, (Value::Bool)*(bool*)data);
      case QMetaType::Int:
	return Value(ls, (double)*(int*)data);
      case QMetaType::UInt:
	return Value(ls, (double)*(unsigned int*)data);
      case QMetaType::Long:
	return Value(ls, (double)*(long*)data);
      case QMetaType::LongLong:
	return Value(ls, (double)*(long long*)data);
      case QMetaType::Short:
	return Value(ls, (double)*(short*)data);
      case QMetaType::Char:
	return Value(ls, (double)*(char*)data);
      case QMetaType::ULong:
	return Value(ls, (double)*(unsigned long*)data);
      case QMetaType::ULongLong:
	return Value(ls, (double)*(unsigned long long*)data);
      case QMetaType::UShort:
	return Value(ls, (double)*(unsigned short*)data);
      case QMetaType::UChar:
	return Value(ls, (double)*(unsigned char*)data);
      case QMetaType::Double:
	return Value(ls, *(double*)data);
      case QMetaType::Float:
	return Value(ls, *(float*)data);
      case QMetaType::QChar:
	return Value(ls, (double)reinterpret_cast<const QChar*>(data)->unicode());
      case QMetaType::QString:
	return Value(ls, String(*reinterpret_cast<const QString*>(data)));
      case QMetaType::QStringList: {
	Value value(ls, Value::TTable);
	const QStringList *qsl = reinterpret_cast<const QStringList*>(data);
	for (int i = 0; i < qsl->size(); i++)
	  value[i+1] = String(qsl->at(i));
	return value;
      }
      case QMetaType::QByteArray:
	return Value(ls, String(*reinterpret_cast<const QByteArray*>(data)));
      case QMetaType::QObjectStar:
	return Value(ls, QObjectWrapper::get_wrapper(ls, *(QObject**)data));
      case QMetaType::QWidgetStar:
	return Value(ls, QObjectWrapper::get_wrapper(ls, *(QWidget**)data));
      case QMetaType::QSize: {
	Value value(ls, Value::TTable);
	const QSize *size = reinterpret_cast<const QSize*>(data);
	value[1] = size->width();
	value[2] = size->height();
	return value;
      }
      case QMetaType::QSizeF: {
	Value value(ls, Value::TTable);
	const QSizeF *size = reinterpret_cast<const QSizeF*>(data);
	value[1] = size->width();
	value[2] = size->height();
	return value;
      }
      case QMetaType::QRect: {
	Value value(ls, Value::TTable);
	const QRect *rect = reinterpret_cast<const QRect*>(data);
	value[1] = rect->x();
	value[2] = rect->y();
	value[3] = rect->width();
	value[4] = rect->height();
	return value;
      }
      case QMetaType::QRectF: {
	Value value(ls, Value::TTable);
	const QRectF *rect = reinterpret_cast<const QRectF*>(data);
	value[1] = rect->x();
	value[2] = rect->y();
	value[3] = rect->width();
	value[4] = rect->height();
	return value;
      }
      case QMetaType::QPoint: {
	Value value(ls, Value::TTable);
	const QPoint *point = reinterpret_cast<const QPoint*>(data);
	value[1] = point->x();
	value[2] = point->y();
	return value;
      }
      case QMetaType::QPointF: {
	Value value(ls, Value::TTable);
	const QPointF *point = reinterpret_cast<const QPointF*>(data);
	value[1] = point->x();
	value[2] = point->y();
	return value;
      }
      default:
	if (type == ud_ref_type)
	  return Value(ls, **(Ref<UserData>*)data);
	return Value(ls);
      }
  }

  bool Member::raw_set_object(int type, void *data, const Value &v)
  {
    switch (type)
      {
      case QMetaType::Bool:
	*(bool*)data = v.to_boolean();
	return true;
      case QMetaType::Int:
	*(int*)data = v.to_number();
	return true;
      case QMetaType::UInt:
	*(unsigned int*)data = v.to_number();
	return true;
      case QMetaType::Long:
	*(long*)data = v.to_number();
	return true;
      case QMetaType::LongLong:
	*(long long*)data = v.to_number();
	return true;
      case QMetaType::Short:
	*(short*)data = v.to_number();
	return true;
      case QMetaType::Char:
	*(char*)data = v.to_number();
	return true;
      case QMetaType::ULong:
	*(unsigned long*)data = v.to_number();
	return true;
      case QMetaType::ULongLong:
	*(unsigned long long*)data = v.to_number();
	return true;
      case QMetaType::UShort:
	*(unsigned short*)data = v.to_number();
	return true;
      case QMetaType::UChar:
	*(unsigned char*)data = v.to_number();
	return true;
      case QMetaType::Double:
	*(double*)data = v.to_number();
	return true;
      case QMetaType::Float:
	*(double*)data = v.to_number();
	return true;
      case QMetaType::QChar:
	*reinterpret_cast<QChar*>(data) = QChar((unsigned short)v.to_number());
	return true;
      case QMetaType::QString:
	*reinterpret_cast<QString*>(data) = v.to_string();
	return true;
      case QMetaType::QStringList: {
	QStringList *qsl = reinterpret_cast<QStringList*>(data);
	try {
	  for (int i = 1; ; i++)
	    qsl->push_back(v[i].to_string());
	} catch (String &e) {
	}
	return true;
      }
      case QMetaType::QByteArray:
	*reinterpret_cast<QByteArray*>(data) = v.to_string();
	return true;
      case QMetaType::QObjectStar:
	*reinterpret_cast<QObject**>(data) = &v.to_userdata_cast<QObjectWrapper>()->get_object();
	return true;
      case QMetaType::QWidgetStar: {
	QObject *obj = &v.to_userdata_cast<QObjectWrapper>()->get_object();
	QWidget *w = qobject_cast<QWidget*>(obj);
	if (!w)
	  throw String("Can not convert lua value, QObject is not a QWidget.");
	*reinterpret_cast<QWidget**>(data) = w;
	return true;
      }
      case QMetaType::QSize: {
	QSize *size = reinterpret_cast<QSize*>(data);
	size->setWidth(v[1].to_number());
	size->setHeight(v[2].to_number());
	return true;
      }
      case QMetaType::QSizeF: {
	QSizeF *size = reinterpret_cast<QSizeF*>(data);
	size->setWidth(v[1].to_number());
	size->setHeight(v[2].to_number());
	return true;
      }
      case QMetaType::QRect: {
	QRect *rect = reinterpret_cast<QRect*>(data);
	rect->setX(v[1].to_number());
	rect->setY(v[2].to_number());
	rect->setWidth(v[3].to_number());
	rect->setHeight(v[4].to_number());
	return true;
      }
      case QMetaType::QRectF: {
	QRectF *rect = reinterpret_cast<QRectF*>(data);
	rect->setX(v[1].to_number());
	rect->setY(v[2].to_number());
	rect->setWidth(v[3].to_number());
	rect->setHeight(v[4].to_number());
	return true;
      }
      case QMetaType::QPoint: {
	QPoint *point = reinterpret_cast<QPoint*>(data);
	point->setX(v[1].to_number());
	point->setY(v[2].to_number());
	return true;
      }
      case QMetaType::QPointF: {
	QPointF *point = reinterpret_cast<QPointF*>(data);
	point->setX(v[1].to_number());
	point->setY(v[2].to_number());
	return true;
      }
      default:
	if (type == ud_ref_type)
	  {
	    *reinterpret_cast<Ref<UserData>*>(data) = v.to_userdata();
	    return true;
	  }
	return false;
      }

  }
}

