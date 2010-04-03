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


#ifndef QTLUAVALUEREF_HXX_
#define QTLUAVALUEREF_HXX_

#include "qtluavalue.hxx"

namespace QtLua {

  ValueRef & ValueRef::operator=(const ValueRef &ref)
  {
    *this = static_cast<const Value &>(ref);
    return *this;
  }

  ValueRef & ValueRef::operator=(Bool n)
  {
    *this = Value(_st, n);
    return *this;
  }

  ValueRef & ValueRef::operator=(double n)
  {
    *this = Value(_st, n);
    return *this;
  }

  ValueRef & ValueRef::operator=(int n)
  {
    *this = Value(_st, (double)n);
    return *this;
  }

  ValueRef & ValueRef::operator=(const String &str)
  {
    *this = Value(_st, str);
    return *this;
  }

  ValueRef & ValueRef::operator=(UserData::ptr ud)
  {
    *this = Value(_st, ud);
    return *this;
  }

  ValueRef & ValueRef::operator=(QObject *obj)
  {
    *this = Value(_st, obj);
    return *this;
  }

}

#endif

