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

#include <cstdlib>
#include <cassert>

#include <QMetaMethod>

#include <QtLua/Value>
#include <QtLua/ValueRef>
#include <QtLua/UserData>
#include <QtLua/String>
#include <QtLua/State>

#include <internal/QObjectWrapper>
#include <internal/TableIterator>

extern "C" {
#include <lua.h>
}

namespace QtLua {

void Value::push_value() const
{
  lua_pushlightuserdata(_st, (void*)this);
  lua_rawget(_st, LUA_REGISTRYINDEX);  
}

int Value::empty_fcn(lua_State *st)
{
  return 0;
}

Value::Value(const State &ls, ValueType type)
  : _st(ls._st)
{
  lua_pushlightuserdata(_st, this);

  switch (type)
    {
    case TNone:
    case TNil:
      lua_pushnil(_st);
      break;

    case TBool:
      lua_pushboolean(_st, false);
      break;

    case TNumber:
      lua_pushnumber(_st, 0.0f);
      break;

    case TString:
      lua_pushstring(_st, "");
      break;

    case TTable:
      lua_newtable(_st);
      break;

    case TFunction:
      lua_pushcfunction(_st, empty_fcn);
      break;

    case TUserData: {
      UserData::ptr ud = QTLUA_REFNEW(QtLua::UserData, );
      ud->push_ud(_st);
      break;
    }

    }

  lua_rawset(_st, LUA_REGISTRYINDEX);
}

Value & Value::operator=(Bool n)
{
  lua_pushlightuserdata(_st, (void*)this);
  lua_pushboolean(_st, n);
  lua_rawset(_st, LUA_REGISTRYINDEX);
  return *this;
}

Value & Value::operator=(double n)
{
  lua_pushlightuserdata(_st, (void*)this);
  lua_pushnumber(_st, n);
  lua_rawset(_st, LUA_REGISTRYINDEX);
  return *this;
}

Value & Value::operator=(const String &str)
{
  lua_pushlightuserdata(_st, this);
  lua_pushlstring(_st, str.constData(), str.size());
  lua_rawset(_st, LUA_REGISTRYINDEX);
  return *this;
}

Value & Value::operator=(UserData::ptr ud)
{
  lua_pushlightuserdata(_st, this);
  ud->push_ud(_st);
  lua_rawset(_st, LUA_REGISTRYINDEX);
  return *this;
}

Value::Value(State &ls, QObject *obj, bool delete_, bool reparent)
  : _st(ls._st)
{
  lua_pushlightuserdata(_st, this);
  QObjectWrapper::get_wrapper(ls, obj, reparent, delete_)->push_ud(_st);
  lua_rawset(_st, LUA_REGISTRYINDEX);
}

Value & Value::operator=(QObject *obj)
{
  lua_pushlightuserdata(_st, this);
  QObjectWrapper::get_wrapper(*State::get_this(_st), obj)->push_ud(_st);
  lua_rawset(_st, LUA_REGISTRYINDEX);
  return *this;
}

bool Value::connect(QObject *obj, const char *signal)
{
  try {
    State &ls = get_state();
    QObjectWrapper::ptr qow = QObjectWrapper::get_wrapper(ls, obj);
    QByteArray ns(QMetaObject::normalizedSignature(signal));
    const QMetaObject *mo = obj->metaObject();
    int sigid = mo->indexOfMethod(ns.constData());

    if (sigid < 0 || mo->method(sigid).methodType() != QMetaMethod::Signal)
      return false;

    qow->_lua_connect(sigid, *this);

  } catch (String &e) {
    return false;
  }
  return true;
}

bool Value::disconnect(QObject *obj, const char *signal)
{
  State &ls = get_state();
  QObjectWrapper::ptr qow = QObjectWrapper::get_wrapper(ls, obj);
  QByteArray ns(QMetaObject::normalizedSignature(signal));
  const QMetaObject *mo = obj->metaObject();
  int sigid = mo->indexOfMethod(ns.constData());

  if (sigid < 0 || mo->method(sigid).methodType() != QMetaMethod::Signal)
    return false;

  return qow->_lua_disconnect(sigid, *this);
}

Value::List Value::call (const List &args) const
{
  push_value();

  int t = lua_type(_st, -1);

  switch (t)
    {
    case TFunction: {
      int oldtop = lua_gettop(_st);

      foreach(const Value &v, args)
	v.push_value();

      if (!lua_pcall(_st, args.size(), LUA_MULTRET, 0))
	{
	  Value::List res;

	  for (int i = oldtop; i <= lua_gettop(_st); i++)
	    res += Value(_st, i);

	  lua_pop(_st, lua_gettop(_st) - oldtop + 1);
	  return res;
	}

      String err(lua_tostring(_st, -1));
      lua_pop(_st, 1);
      throw err;
    }

    case TUserData: {
      UserData::ptr ud = UserData::get_ud(_st, -1);
      lua_pop(_st, 1);

      if (!ud.valid())
	throw String("Can not call null lua::userdata value.");

      return ud->meta_call(*State::get_this(_st), args);
    }

    default:
      lua_pop(_st, 1);
      throw String("Can not call lua::% value.").arg(lua_typename(_st, t));
    }
}

Value Value::operator[] (const Value &key) const
{
  push_value();

  int t = lua_type(_st, -1);

  switch (t)
    {
    case TUserData: {
      UserData::ptr ud = UserData::get_ud(_st, -1);
      lua_pop(_st, 1);

      if (!ud.valid())
	throw String("Can not index null lua::userdata value.");

      return ud->meta_index(*State::get_this(_st), key);
    }

    case TTable: {
      key.push_value();
      lua_gettable(_st, -2);
      Value res(_st, -1);
      lua_pop(_st, 2);
      return res;
    }

    default:
      lua_pop(_st, 1);
      throw String("Can not index lua::% value.").arg(lua_typename(_st, t));
    }
}

Ref<Iterator> Value::new_iterator() const
{
  push_value();

  switch (int t = lua_type(_st, -1))
    {
    case TUserData: {
      UserData::ptr ud = UserData::get_ud(_st, -1);
      lua_pop(_st, 1);

      if (!ud.valid())
	throw String("Can not iterate through null lua::userdata value.");

      return ud->new_iterator(*State::get_this(_st));
    }

    case TTable: {
      Iterator::ptr it = QTLUA_REFNEW(TableIterator, _st, *this);
      lua_pop(_st, 1);
      return it;
    }

    default:
      lua_pop(_st, 1);
      throw String("Can not iterate through lua::% value.").arg(lua_typename(_st, t));
    }
}

Value & Value::operator=(const Value &lv)
{
  if (_st != lv._st)
    {
      lua_pushlightuserdata(_st, this);
      lua_pushnil(_st);
      lua_rawset(_st, LUA_REGISTRYINDEX);
      _st = lv._st;
    }

  lua_pushlightuserdata(_st, this);
  lv.push_value();
  lua_rawset(_st, LUA_REGISTRYINDEX);

  return *this;
}

Value::Value(const Value &lv)
  : _st(lv._st)
{
  lua_pushlightuserdata(_st, this);
  lv.push_value();
  lua_rawset(_st, LUA_REGISTRYINDEX);
}

Value::Value(const State &ls, const Value &lv)
  : _st(lv._st)
{
  assert(ls._st == lv._st);
  lua_pushlightuserdata(_st, this);
  lv.push_value();
  lua_rawset(_st, LUA_REGISTRYINDEX);
}

Value::~Value()
{
  lua_pushlightuserdata(_st, this);
  lua_pushnil(_st);
  lua_rawset(_st, LUA_REGISTRYINDEX);
}

Value::Bool Value::to_boolean() const
{
  push_value();
  Bool res = (Bool)lua_toboolean(_st, -1);
  lua_pop(_st, 1);
  return res;
}

Value::ValueType Value::type() const
{
  push_value();
  int res = lua_type(_st, -1);
  lua_pop(_st, 1);
  return (Value::ValueType)res;
}

String Value::type_name() const
{
  return String("lua::") + lua_typename(_st, type());
}

String Value::type_name_u() const
{
  push_value();
  int t = lua_type(_st, -1);

  if (t == TUserData)
    {
      UserData::ptr ud = UserData::get_ud(_st, -1);
      lua_pop(_st, 1);
      String res(ud.valid() ? ud->get_type_name() : "QtLua::UserData");
      return res;
    }
  else
    {
      lua_pop(_st, 1);
      return String("lua::") + lua_typename(_st, t);
    }
}

Value::Value(lua_State *st, int index)
  : _st(st)
{ 
  lua_pushlightuserdata(_st, this);
  if (index < 0 && index != LUA_GLOBALSINDEX)
    index--;
  lua_pushvalue(_st, index);
  lua_rawset(_st, LUA_REGISTRYINDEX);
}

void Value::convert_error(ValueType type) const
{
  int type_b = lua_type(_st, -1);

  lua_pop(_st, 1);

  throw String("Can not convert lua::% value to lua::%.")
    .arg(lua_typename(_st, type_b)).arg(lua_typename(_st, (int)type));
}

lua_Number Value::to_number() const
{
  push_value();

  switch (lua_type(_st, -1))
    {
    case LUA_TBOOLEAN:
    case LUA_TNUMBER: {
      lua_Number res = lua_tonumber(_st, -1);
      lua_pop(_st, 1);
      return res;
    }

    case LUA_TSTRING: {
      char *end;
      lua_Number res = strtod(lua_tostring(_st, -1), &end);
      lua_pop(_st, 1);

      if (!*end)
	return res;
    }

    }

  convert_error(TNumber);
  std::abort();
}

String Value::to_string() const
{
  push_value();

  const char	*str = lua_tostring(_st, -1);

  if (str)
    {
      String res(lua_tostring(_st, -1), lua_strlen(_st, -1));
      lua_pop(_st, 1);
      return res;
    }

  convert_error(TString);
  std::abort();
}

String Value::to_string_p() const
{
  push_value();
  String res(to_string_p(_st, -1));
  lua_pop(_st, 1);
  return res;
}

String Value::to_string_p(lua_State *st, int index)
{
  switch (lua_type(st, index))
    {
    case TNone:
      return "(none)";

    case TNil:
      return "(nil)";

    case TBool: {
      String res(lua_toboolean(st, index) ? "true" : "false");
      return res;
    }

    case TNumber: {
      String res;
      res.setNum(lua_tonumber(st, index));
      return res;
    }

    case TString:
      return String("\"") + lua_tostring(st, index) + "\"";

    case TFunction:
      return "(lua::function)";

    case TUserData: {
      UserData::ptr ud = UserData::get_ud(st, index);
      String res(ud.valid() ? ud->get_value_str() : ud->UserData::get_value_str());
      return res;
    }

    default: {
      String res;
      res.setNum((qulonglong)lua_topointer(st, index), 16);
      return "0x" + res;
    }

    }
}

const char * Value::to_cstring() const
{
  push_value();

  const char	*str = lua_tostring(_st, -1);

  if (str)
    {
      lua_pop(_st, 1);
      return str;
    }

  convert_error(TString);
  std::abort();
}

UserData::ptr Value::to_userdata() const
{
  push_value();

  if (lua_type(_st, -1) == LUA_TUSERDATA)
    {
      UserData::ptr ptr = UserData::get_ud(_st, -1);
      lua_pop(_st, 1);
      return ptr;
    }

  convert_error(TUserData);
  std::abort();
}

UserData::ptr Value::to_userdata_null() const
{
  push_value();

  if (lua_type(_st, -1) == LUA_TUSERDATA)
    {
      UserData::ptr ptr = UserData::get_ud(_st, -1);
      lua_pop(_st, 1);
      return ptr;
    }

  lua_pop(_st, 1);
  return UserData::ptr();
}

bool Value::operator==(const Value &lv) const
{
  bool		res;;

  if (lv._st != _st)
    return false;

  push_value();
  lv.push_value();

  if ((lua_type(_st, -1) == TUserData) &&
      (lua_type(_st, -2) == TUserData))
    {
      UserData::ptr a = UserData::get_ud(_st, -1);
      UserData::ptr b = UserData::get_ud(_st, -2);

      res = (a.valid() == b.valid()) && (!a.valid() || (*a == *b));
    }
  else
    {
      res = lua_equal(_st, -1, -2);
    }

  lua_pop(_st, 2);
  return res;
}

bool Value::operator<(const Value &lv) const
{
  bool		res;;

  if (lv._st != _st)
    return false;

  push_value();
  lv.push_value();

  if ((lua_type(_st, -1) == TUserData) &&
      (lua_type(_st, -2) == TUserData))
    {
      UserData::ptr a = UserData::get_ud(_st, -1);
      UserData::ptr b = UserData::get_ud(_st, -2);

      res = a.valid() && b.valid() && (*a < *b);
    }
  else
    {
      res = lua_lessthan(_st, -1, -2);
    }

  lua_pop(_st, 2);
  return res;
}

bool Value::operator==(const String &str) const
{
  bool res = false;
  push_value();

  if (lua_isstring(_st, -1))
    {
      String s(lua_tostring(_st, -1), lua_strlen(_st, -1));

      res = (str == s);
    }

  lua_pop(_st, 1);

  return res;
}

bool Value::operator==(const char *str) const
{
  bool res = false;
  push_value();

  if (lua_isstring(_st, -1))
    res = !strcmp(lua_tostring(_st, -1), str);

  lua_pop(_st, 1);

  return res;
}

bool Value::operator==(double n) const
{
  bool res = false;
  push_value();

  if (lua_isnumber(_st, -1))
    res = lua_tonumber(_st, -1) == n;

  lua_pop(_st, 1);

  return res;
}

uint qHash(const Value &lv)
{
  uint	res;

  lv.push_value();

  switch (lua_type(lv._st, -1))
    {
    case LUA_TBOOLEAN:
      res = lua_toboolean(lv._st, -1);
      break;

    case LUA_TNUMBER: {
      lua_Number n = lua_tonumber(lv._st, -1);
      res = *(uint*)&n;
      break;
    }

    case LUA_TSTRING:
      res = qHash(String(lua_tostring(lv._st, -1), lua_strlen(lv._st, -1)));
      break;

    case LUA_TUSERDATA: {
      QtLua::Ref<UserData> ud = UserData::get_ud(lv._st, -1);
      res = (uint)(long)ud.ptr();
      break;
    }

    default:
      res = (uint)(long)lua_topointer(lv._st, -1);
    }

  lua_pop(lv._st, 1);

  return res;
}

}

