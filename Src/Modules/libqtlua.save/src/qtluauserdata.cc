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


#include <cstdarg>

#ifdef __GNUC__
#include <cxxabi.h>
#endif

extern "C" {
#include <lua.h>
}

#include <QtLua/UserData>
#include <QtLua/Value>
#include <QtLua/State>
#include <QtLua/String>

namespace QtLua {

void UserData::push_ud(lua_State *st)
{
  // allocate lua user data to store reference to 'this'
  new (lua_newuserdata(st, sizeof (UserData::ptr))) UserData::ptr(*this);

  // attach metatable
  lua_pushlightuserdata(st, &State::_key_item_metatable);
  lua_rawget(st, LUA_REGISTRYINDEX);
  lua_setmetatable(st, -2);
}

QtLua::Ref<UserData> UserData::get_ud(lua_State *st, int i)
{
#ifndef QTLUA_NO_USERDATA_CHECK
  if (lua_getmetatable(st, i))
    {
      lua_pushlightuserdata(st, &State::_key_item_metatable);
      lua_rawget(st, LUA_REGISTRYINDEX);

      if (lua_rawequal(st, -2, -1))
	{
	  lua_pop(st, 2);
#endif
	  UserData::ptr	*item = static_cast<UserData::ptr *>(lua_touserdata(st, i));
	  return *item;
#ifndef QTLUA_NO_USERDATA_CHECK
	}

      lua_pop(st, 1);
    }

  lua_pop(st, 1);

  throw String("Lua userdata is not a QtLua::UserData.");
#endif
}

String UserData::get_type_name() const
{
#ifdef __GNUC__
  int s;
  return abi::__cxa_demangle(typeid(*this).name(), 0, 0, &s);
#else
  return typeid(*this).name();
#endif
}

String UserData::get_value_str() const
{
  return QString().sprintf("%p", this);
}

Value UserData::meta_operation(State &ls, Operation op,
			       const Value &a, const Value &b) 
{
  throw String("Operation not handled by % type").arg(get_type_name());
};

void UserData::meta_newindex(State &ls, const Value &key, const Value &value) 
{
  throw String("Table write access not handled by % type").arg(get_type_name());
};

Value UserData::meta_index(State &ls, const Value &key) 
{
  throw String("Table read access not handled by % type").arg(get_type_name());
};

Value::List UserData::meta_call(State &ls, const Value::List &args) 
{
  throw String("Function call not handled by % type").arg(get_type_name());
};

Ref<Iterator> UserData::new_iterator(State &ls)
{
  throw String("Table iteration not handled by % type").arg(get_type_name());
}

void UserData::meta_call_check_args(const Value::List &args,
				    int min_count, int max_count, ...) 
{
  int			i;
  va_list		ap;

  if (args.count() < min_count)
    throw String("Missing argument(s), at least % arguments expected").arg(min_count);

  if (max_count && args.count() > max_count)
    throw String("Too many argument(s), at most % arguments expected").arg(max_count);

  va_start(ap, max_count);

  Value::ValueType	type = Value::TNone;

  for (i = 0; i < args.size(); i++)
    {
      if (i < min_count || i < max_count)
	type = (Value::ValueType)va_arg(ap, int);

      if (type != Value::TNone && type != args[i].type())
	{
	  va_end(ap);
	  throw String("Wrong type for argument %, lua::% expected instead of %.")
	    .arg(i+1).arg(lua_typename(0, type)).arg(args[i].type_name());
	}
    }

  va_end(ap);
}

bool UserData::operator==(const UserData &ud)
{
  return this == &ud;
}

bool UserData::operator<(const UserData &ud)
{
  return this < &ud;
}

void UserData::completion_patch(String &path, String &entry, int &offset)
{
}

}

