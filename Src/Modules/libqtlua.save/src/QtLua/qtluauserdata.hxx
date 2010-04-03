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


#ifndef QTLUAUSERDATA_HXX_
#define QTLUAUSERDATA_HXX_

#ifdef __GNUC__
#include <cxxabi.h>
#endif

#include "Iterator"
#include "Value"

namespace QtLua {

  UserData::~UserData()
  {
  }

  template <class X>
  inline String UserData::type_name()
  {
#ifdef __GNUC__
    int s;
    return String(abi::__cxa_demangle(typeid(X).name(), 0, 0, &s));
#else
    return String(typeid(X).name());
#endif
  }

}

#endif

