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

#include <cassert>

#include <QtLua/Value>
#include <QtLua/Iterator>

#include <internal/TableIterator>

extern "C" {
#include <lua.h>
}

namespace QtLua {
  
TableIterator::TableIterator(lua_State *st, const Value &table)
  : _st(st),
    _key(Value(st)),
    _value(Value(st)),
    _more(true)
{
  lua_pushlightuserdata(_st, this);
  table.push_value();
  assert(lua_type(_st, -1) == Value::TTable);
  lua_rawset(_st, LUA_REGISTRYINDEX);

  fetch();
}

TableIterator::~TableIterator()
{
  lua_pushlightuserdata(_st, this);
  lua_pushnil(_st);
  lua_rawset(_st, LUA_REGISTRYINDEX);
}

bool TableIterator::more() const
{
  return _more;
}

void TableIterator::next()
{
  fetch();
}

void TableIterator::fetch()
{
  assert(_more);

  lua_pushlightuserdata(_st, this);
  lua_rawget(_st, LUA_REGISTRYINDEX);

  _key.push_value();

  if (lua_next(_st, -2))
    {
      _key = Value(_st, -2);
      _value = Value(_st, -1);
      lua_pop(_st, 2);
    }
  else
    {
      _more = false;
    }

  lua_pop(_st, 1);
}

Value TableIterator::get_key() const
{
  return _key;
}

Value TableIterator::get_value() const
{
  return _value;
}

ValueRef TableIterator::get_value_ref()
{
  lua_pushlightuserdata(_st, this);
  lua_rawget(_st, LUA_REGISTRYINDEX);
  Value val(_st, -1);
  ValueRef ref(val, _key);
  lua_pop(_st, 1);
  return ref;
}

}

