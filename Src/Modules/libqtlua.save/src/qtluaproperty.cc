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

#include <cstring>

#include <internal/QObjectWrapper>

#include <internal/Member>
#include <internal/Property>

namespace QtLua {

  Property::Property(const QMetaObject *mo, int index)
    : Member(mo, index)
  { 
  }

  void Property::assign(QObjectWrapper &qow, const Value &value)
  {
    QMetaProperty mp = _mo->property(_index);
    QObject &obj = qow.get_object();

    // Try to reset property if assign nil
    if (value.type() == Value::TNil)
      {
	if (!mp.isResettable())
 	  throw String("QObject property '%' is not resettable.").arg(mp.name());

	if (!mp.reset(&obj))
 	  throw String("Unable to reset QObject property '%'.").arg(mp.name());
	return;
      }

    if (!mp.isWritable())
      throw String("QObject property '%' is read only.").arg(mp.name());

    int type = mp.userType();

    if (type > 0)
      {
	void *data = QMetaType::construct(type);
	assert(data);

	if (Member::raw_set_object(type, data, value))
	  {
	    bool ok = mp.write(&obj, QVariant(type, data));
	    QMetaType::destroy(type, data);

	    if (!ok)
	      throw String("Unable to set QObject property.");

	    return;
	  }

	QMetaType::destroy(type, data);
      }

    throw String("Unsupported convertion from % lua type to % Qt type.").arg(value.type_name_u()).arg(mp.typeName());
  }

  Value Property::access(QObjectWrapper &qow)
  {
    QMetaProperty mp = _mo->property(_index);
    QObject &obj = qow.get_object();

    if (!mp.isReadable())
      throw String("QObject property '%' is not readable.").arg(mp.name());

    QVariant variant = mp.read(&obj);

    if (!variant.isValid())
      throw String("Unable to get QObject property.");

    return Value(Member::raw_get_object(qow.get_state(), variant.type(), variant.constData()));
  }

  String Property::get_value_str() const
  {
    return String(_mo->className()) + "::" + _mo->property(_index).name();
  }

  String Property::get_type_name() const
  {
    QMetaProperty mp = _mo->property(_index);
    return Member::get_type_name() + "<" + mp.typeName() + ">";
  }

}

