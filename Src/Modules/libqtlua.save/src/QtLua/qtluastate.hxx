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

#ifndef QTLUASTATE_HXX_
#define QTLUASTATE_HXX_

#include "qtluastring.hxx"
#include "qtluavalue.hxx"
#include "qtluavalueref.hxx"

namespace QtLua {

  Value State::operator[] (const String &key) const
  {
    return (*this)[Value(*this, key)];
  }

  ValueRef State::operator[] (const String &key)
  {
    return (*this)[Value(*this, key)];
  }

}

#endif

