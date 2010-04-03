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

#ifndef QTLUAFUNCTION_HXX_
#define QTLUAFUNCTION_HXX_

#include "qtluauserdata.hxx"
#include "qtluavalue.hxx"

namespace QtLua {

template <class X>
X Function::get_arg(const Value::List &args, int n, const X & default_)
{
  return n >= args.size() ? default_ : args[n];
}

template <class X>
X Function::get_arg(const Value::List &args, int n)
{
  if (n >= args.size())
    throw String("Missing argument %, expected % type argument.").arg(n).arg(UserData::type_name<X>());

  return args[n];
}

template <class X>
Ref<X> Function::get_arg_ud(const Value::List &args, int n)
{
  return get_arg<const Value &>(args, n).to_userdata_cast<X>();
}

}

#endif

