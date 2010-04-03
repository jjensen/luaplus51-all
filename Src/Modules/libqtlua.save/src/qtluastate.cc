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

#include "../config.hh"

#include <cstdlib>

#include <QStringList>

#include <QtLua/State>
#include <QtLua/UserData>
#include <QtLua/Value>
#include <QtLua/ValueRef>
#include <QtLua/Iterator>
#include <QtLua/String>
#include <QtLua/Function>
#include <internal/QObjectWrapper>

#include "qtluaqtlib.hh"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

namespace QtLua {

char State::_key_item_metatable;
char State::_key_this;

/************************************************************************
	lua c functions
************************************************************************/

int State::lua_cmd_iterator(lua_State *st)
{
  try {
    Iterator::ptr	i = Value(st, 1).to_userdata_cast<Iterator>();

    if (i->more())
      {
	i->get_key().push_value();
	i->get_value().push_value();
	i->next();
	return 2;
      }
    else
      {
	lua_pushnil(st);
	return 1;
      }

  } catch (String &e) {
    lua_pushstring(st, e.constData());
    lua_error(st);
  }

  std::abort();
}

int State::lua_cmd_each(lua_State *st)
{
  int idx = 1;

  if (lua_gettop(st) < 1)
    idx = LUA_GLOBALSINDEX;

  try {
    Value		table(st, idx);
    Iterator::ptr	i = table.new_iterator();

    lua_pushcfunction(st, lua_cmd_iterator);
    i->push_ud(st);
    lua_pushnil(st);

  } catch (String &e) {
    lua_pushstring(st, e.constData());
    lua_error(st);
  }

  return 3;
}

int State::lua_cmd_print(lua_State *st)
{
  try {
    State	*this_ = get_this(st);

    for (int i = 1; i <= lua_gettop(st); i++)
      {
	String s = Value::to_string_p(st, i);
	this_->output(s);
	this_->output("\n");
	qDebug("QtLua print:%s", s.constData());
      }

  } catch (String &e) {
    lua_pushstring(st, e.constData());
    lua_error(st);
  }

  return 0;
}

int State::lua_cmd_plugin(lua_State *st)
{
  try {
    State	*this_ = get_this(st);

    if (lua_gettop(st) < 1 || !lua_isstring(st, 1))
      {
	this_->output("Usage: plugin(\"library_filename_without_ext\")\n");
	return 0;
      }

    QTLUA_REFNEW(Plugin, String(lua_tostring(st, 1)) + Plugin::get_plugin_ext())->push_ud(st);
    return 1;

  } catch (String &e) {
    lua_pushstring(st, e.constData());
    lua_error(st);
  }

  return 0;
}

int State::lua_cmd_list(lua_State *st)
{
  try {
    State	*this_ = get_this(st);

    int idx = lua_gettop(st) > 0 ? 1 : LUA_GLOBALSINDEX;

    // display table object content
    const Value	t = Value(st, idx);

    for (Value::const_iterator i = t.begin(); i != t.end(); i++)
      {
	this_->output(QString("\033[18m") + i.value().type_name_u() + "\033[2m " +
		      i.key().to_string_p() + " = " + i.value().to_string_p() + "\n");
      }

  } catch (String &e) {
    lua_pushstring(st, e.constData());
    lua_error(st);
  }

  return 0;
}

int State::lua_cmd_help(lua_State *st)
{
  State *this_ = get_this(st);

  if (lua_gettop(st) < 1)
    {
      this_->output("Usage: help(function)\n");
      return 0;
    }

  Value v(st, 1);

  if (v.type() == Value::TUserData)
    {
      Function::ptr cmd = v.to_userdata().dynamiccast<Function>();

      if (cmd.valid())
	{
	  this_->output(cmd->get_help() + "\n");
	  return 0;
	}
    }

  this_->output("Help is only available for QtLua::Function objects\n");
  return 0;
}

// lua item metatable methods

#define LUA_META_2OP_FUNC(n, op)					\
									\
int State::lua_meta_item_##n(lua_State *st)				\
{									\
  int		x = lua_gettop(st);					\
									\
  try {									\
    Value	a(st, 1);						\
    Value	b(st, 2);						\
									\
    if (a.type() == Value::TUserData)					\
      a.to_userdata()->meta_operation(*get_this(st), op, a, b).push_value();	\
    else if (b.type() == Value::TUserData)				\
      b.to_userdata()->meta_operation(*get_this(st), op, a, b).push_value();	\
    else								\
      std::abort();								\
									\
  } catch (String &e) {							\
    lua_pushstring(st, e.constData());					\
    lua_error(st);							\
  }									\
									\
  return lua_gettop(st) - x;						\
}

#define LUA_META_1OP_FUNC(n, op)					\
									\
int State::lua_meta_item_##n(lua_State *st)				\
{									\
  int		x = lua_gettop(st);					\
									\
  try {									\
    Value	a(st, 1);						\
									\
     a.to_userdata()->meta_operation(*get_this(st), op, a, a).push_value();	\
									\
  } catch (String &e) {							\
    lua_pushstring(st, e.constData());					\
    lua_error(st);							\
  }									\
									\
  return lua_gettop(st) - x;						\
}

LUA_META_2OP_FUNC(add, UserData::OpAdd)
LUA_META_2OP_FUNC(sub, UserData::OpSub)
LUA_META_2OP_FUNC(mul, UserData::OpMul)
LUA_META_2OP_FUNC(div, UserData::OpDiv)
LUA_META_2OP_FUNC(mod, UserData::OpMod)
LUA_META_2OP_FUNC(pow, UserData::OpPow)
LUA_META_1OP_FUNC(unm, UserData::OpUnm)
LUA_META_2OP_FUNC(concat, UserData::OpConcat)
LUA_META_1OP_FUNC(len, UserData::OpLen)
LUA_META_2OP_FUNC(eq, UserData::OpEq)
LUA_META_2OP_FUNC(lt, UserData::OpLt)
LUA_META_2OP_FUNC(le, UserData::OpLe)

int State::lua_meta_item_index(lua_State *st)
{
  int		x = lua_gettop(st);

  try {
    UserData::ptr ud = UserData::get_ud(st, 1);

    if (!ud.valid())
      throw String("Can not index null lua::userdata value.");

    Value	op(st, 2);

    Value v = ud->meta_index(*get_this(st), op);
    assert(v._st == st);
    v.push_value();

  } catch (String &e) {
    lua_pushstring(st, e.constData());
    lua_error(st);
  }

  return lua_gettop(st) - x;
}

int State::lua_meta_item_newindex(lua_State *st)
{
  int		x = lua_gettop(st);

  try {
    UserData::ptr ud = UserData::get_ud(st, 1);

    if (!ud.valid())
      throw String("Can not index null lua::userdata value.");

    Value	op1(st, 2);
    Value	op2(st, 3);

    ud->meta_newindex(*get_this(st), op1, op2);

  } catch (String &e) {
    lua_pushstring(st, e.constData());
    lua_error(st);
  }

  return lua_gettop(st) - x;
}

int State::lua_meta_item_call(lua_State *st)
{
  int			n = lua_gettop(st);

  try {
    UserData::ptr ud = UserData::get_ud(st, 1);

    if (!ud.valid())
      throw String("Can not call null lua::userdata value.");

    Value::List	args;

    for (int i = 2; i <= lua_gettop(st); i++)
      args.append(Value(st, i));

    foreach(const Value &v, ud->meta_call(*get_this(st), args))
      {
	assert(v._st == st);
	v.push_value();
      }

  } catch (String &e) {
    lua_pushstring(st, e.constData());
    lua_error(st);
  }

  return lua_gettop(st) - n;
}

int State::lua_meta_item_gc(lua_State *st)
{
  UserData::get_ud(st, 1).~Ref<UserData>();

  return 0;
}

/************************************************************************/

bool State::set_global_r(const String &name, const Value &value, int tblidx)
{
  int len = name.indexOf('.', 0);

  if (len < 0)
    {
      // set value in table if last
      lua_pushstring(_st, name.constData());
      value.push_value();
      lua_settable(_st, tblidx);

      return true;
    }
  else
    {
      // find intermediate value in path
      String prefix(name.mid(0, len));

      lua_pushstring(_st, prefix.constData());
      lua_gettable(_st, tblidx);

      if (lua_isnil(_st, -1))
	{
	  // create intermediate table
	  lua_pop(_st, 1);
	  lua_pushstring(_st, prefix.constData());
	  lua_newtable(_st);

	  if (set_global_r(name.mid(len + 1), value, lua_gettop(_st)))
	    {
	      lua_settable(_st, tblidx);
	      return true;
	    }
	  else
	    {
	      lua_pop(_st, 2);
	      return false;
	    }
	}
      else if (lua_istable(_st, -1))
	{
	  // use existing intermediate table
	  bool res = set_global_r(name.mid(len + 1), value, lua_gettop(_st));
	  lua_pop(_st, 1);
	  return res;
	}
      else
	{
	  // bad existing intermediate value
	  lua_pop(_st, 1);
	  return false;
	}
    }
}

void State::set_global(const String &name, const Value &value)
{
  if (!set_global_r(name, value, LUA_GLOBALSINDEX))
    throw String("Unable to set lua global variable.");
}

void State::get_global_r(const String &name, Value &value, int tblidx) const
{
  int len = name.indexOf('.', 0);

  if (len < 0)
    {
      // get value from table if last
      lua_pushstring(_st, name.constData());
      lua_gettable(_st, tblidx);
      value = Value(_st, -1);
      lua_pop(_st, 1);
    }
  else
    {
      // find intermediate value in path
      String prefix(name.mid(0, len));

      lua_pushstring(_st, prefix.constData());
      lua_gettable(_st, tblidx);

      if (lua_istable(_st, -1))
	{
	  get_global_r(name.mid(len + 1), value, lua_gettop(_st));
	  lua_pop(_st, 1);
	}

      lua_pop(_st, 1);
    }
}

Value State::get_global(const String &path) const
{
  Value res(const_cast<State&>(*this));
  get_global_r(path, res, LUA_GLOBALSINDEX);
  return res;
}

Value State::operator[] (const Value &key) const
{
  key.push_value();
  lua_gettable(_st, LUA_GLOBALSINDEX);
  Value res(_st, -1);
  lua_pop(_st, 1);
  return res;
}

ValueRef State::operator[] (const Value &key)
{
  return ValueRef(Value(_st, LUA_GLOBALSINDEX), key);
}

State::State()
{
  assert(Value::TNone == LUA_TNONE);
  assert(Value::TNil == LUA_TNIL);
  assert(Value::TBool == LUA_TBOOLEAN);
  assert(Value::TNumber == LUA_TNUMBER);
  assert(Value::TString == LUA_TSTRING);
  assert(Value::TTable == LUA_TTABLE);
  assert(Value::TFunction == LUA_TFUNCTION);
  assert(Value::TUserData == LUA_TUSERDATA);

  _st = lua_open();

  if (!_st)
    throw std::bad_alloc();

  //lua_atpanic(_st, lua_panic);

  // creat metatable for UserData events

  lua_pushlightuserdata(_st, &_key_item_metatable);
  lua_newtable(_st);

#define LUA_META_BIND(n)			\
  lua_pushstring(_st, "__" #n);			\
  lua_pushcfunction(_st, lua_meta_item_##n);	\
  lua_rawset(_st, -3);

  LUA_META_BIND(add);
  LUA_META_BIND(sub);
  LUA_META_BIND(mul);
  LUA_META_BIND(div);
  LUA_META_BIND(mod);
  LUA_META_BIND(pow);
  LUA_META_BIND(unm);
  LUA_META_BIND(concat);
  LUA_META_BIND(len);
  LUA_META_BIND(eq);
  LUA_META_BIND(lt);
  LUA_META_BIND(le);
  LUA_META_BIND(index);
  LUA_META_BIND(newindex);
  LUA_META_BIND(call);
  LUA_META_BIND(gc);

  lua_rawset(_st, LUA_REGISTRYINDEX);

  // pointer to this

  lua_pushlightuserdata(_st, &_key_this);
  lua_pushlightuserdata(_st, this);
  lua_rawset(_st, LUA_REGISTRYINDEX);
}

State::~State()
{
  // clear all global variables
  while (1)
    {
      lua_pushnil(_st);
      if (!lua_next(_st, LUA_GLOBALSINDEX))
	break;
      lua_pop(_st, 1);
      lua_pushnil(_st);
      lua_rawset(_st, LUA_GLOBALSINDEX);
    }

  gc_collect();

  // wipe remaining QObjectWrapper objects
  wrapper_hash_t::const_iterator i;

  while ((i = _whash.begin()) != _whash.end())
    i.value()->_drop();

#ifndef QTLUA_NO_DEBUG
  // check for existing Value objects
  lua_pushlightuserdata(_st, &_key_this);
  lua_pushnil(_st);
  lua_rawset(_st, LUA_REGISTRYINDEX);

  lua_pushlightuserdata(_st, &_key_item_metatable);
  lua_pushnil(_st);
  lua_rawset(_st, LUA_REGISTRYINDEX);

  bool bad = false;

  lua_pushnil(_st);
  while (lua_next(_st, LUA_REGISTRYINDEX))
    {
      if (lua_islightuserdata(_st, -2))
	{
	  qCritical("QtLua::Value<%s> at %p still refers to lua state being free'd.",
		    lua_typename(_st, -1), lua_touserdata(_st, -2));
	  bad = true;
	}
      lua_pop(_st, 1);
    }

  if (bad)
    abort();
#endif

  lua_close(_st);
}

int State::lua_panic(lua_State *st)
{
  String err(lua_tostring(st, -1));
  lua_pop(st, 1);
  throw err;
}

struct lua_reader_state_s
{
  QIODevice *_io;
  QByteArray _read_buf;
};

static const char * lua_reader(lua_State *st, void *data, size_t *size)
{
  struct lua_reader_state_s *rst = (struct lua_reader_state_s *)data;

  rst->_read_buf = rst->_io->read(4096);
  *size = rst->_read_buf.size();
  return rst->_read_buf.constData();
}

Value::List State::exec_chunk(QIODevice &io)
{
  struct lua_reader_state_s rst;
  rst._io = &io;

  if (lua_load(_st, &lua_reader, &rst, ""))
    {
      String err(lua_tostring(_st, -1));
      lua_pop(_st, 1);
      throw err;
    }

  int oldtop = lua_gettop(_st);

  if (lua_pcall(_st, 0, LUA_MULTRET, 0))
    {
      String err(lua_tostring(_st, -1));
      lua_pop(_st, 1);
      throw err;
    }

  Value::List res;
  for (int i = oldtop; i <= lua_gettop(_st); i++)
    res += Value(_st, i);
  lua_pop(_st, lua_gettop(_st) - oldtop + 1);

  return res;
}

Value::List State::exec_statements(const String & statement)
{
  if (luaL_loadbuffer(_st, statement.constData(), statement.size(), ""))
    {
      String err(lua_tostring(_st, -1));
      lua_pop(_st, 1);
      throw err;
    }

  int oldtop = lua_gettop(_st);

  if (lua_pcall(_st, 0, LUA_MULTRET, 0))
    {
      String err(lua_tostring(_st, -1));
      lua_pop(_st, 1);
      throw err;
    }

  Value::List res;
  for (int i = oldtop; i <= lua_gettop(_st); i++)
    res += Value(_st, i);
  lua_pop(_st, lua_gettop(_st) - oldtop + 1);

  return res;
}

void State::exec(const QString &statement)
{
  try {
    exec_statements(statement);

  } catch (QtLua::String &e) {
    output(QString("\033[7merror\033[2m: ") + e.constData() + "\n");
  }

  gc_collect();
}

void State::gc_collect()
{
#ifdef HAVE_LUA_GC
  lua_gc(_st, LUA_GCCOLLECT, 0);
#else
  lua_setgcthreshold(_st, 0);
#endif
}

void State::reg_c_function(const char *name, lua_CFunction f)
{
  lua_pushstring(_st, name);
  lua_pushcfunction(_st, f);
  lua_rawset(_st, LUA_GLOBALSINDEX);
}

State * State::get_this(lua_State *st)
{
  void *data;

  lua_pushlightuserdata(st, &_key_this);
  lua_rawget(st, LUA_REGISTRYINDEX);
  data = lua_touserdata(st, -1);
  lua_pop(st, 1);

  return static_cast<State*>(data);
}

void State::openlib(Library lib)
{
  switch (lib)
    {
    case BaseLib:
      luaopen_base(_st);
      return;
#ifdef HAVE_LUA_PACKAGELIB
    case PackageLib:
      luaopen_package(_st);
      return;
#endif
    case StringLib:
      luaopen_string(_st);
      return;
    case TableLib:
      luaopen_table(_st);
      return;
    case MathLib:
      luaopen_math(_st);
      return;
    case IoLib:
      luaopen_io(_st);
      return;
#ifdef HAVE_LUA_OSLIB
    case OsLib:
      luaopen_os(_st);
      return;
#endif
    case DebugLib:
      luaopen_debug(_st);
      return;
    case AllLibs:
#ifdef HAVE_LUA_OPENLIBS
      luaL_openlibs(_st);
#else
      luaopen_base(_st);
      luaopen_string(_st);
      luaopen_table(_st);
      luaopen_math(_st);
      luaopen_io(_st);
      luaopen_debug(_st);
#endif
      qtluaopen_qt(*this);
    case QtLuaLib:
      reg_c_function("print", lua_cmd_print);
      reg_c_function("list", lua_cmd_list);
      reg_c_function("each", lua_cmd_each);
      reg_c_function("help", lua_cmd_help);
      reg_c_function("plugin", lua_cmd_plugin);
      return;
    case QtLib:
      qtluaopen_qt(*this);
      return;
    }
}

void State::lua_do(void (*func)(lua_State *st))
{
  func(_st);
}

void State::fill_completion_list_r(String &path, const String &prefix,
				   QStringList &list, const Value &tbl,
				   int &cursor_offset)
{
  int len = strcspn(prefix.constData(), ":.");

  if (len == prefix.size())
    {
      String lastentry, tpath(path);

      // enumerate table object
      for (Value::const_iterator i = tbl.begin(); i != tbl.end(); i++)
	{
	  const Value &k = i.key();

	  // ignore non string keys
	  if (k.type() != Value::TString)
	    continue;

	  String entry = k.to_string();

	  if (entry.startsWith(prefix))
	    {
	      const Value &v = i.value();

	      // add operator for known types
	      switch (v.type())
		{
		case Value::TTable:
		  entry += ".";
		  break;

		case Value::TFunction:
		  entry += "()";
		  cursor_offset = -1;
		  break;

		case Value::TUserData:
		  v.to_userdata()->completion_patch(tpath, entry, cursor_offset);
		default:
		  break;
		}

	      lastentry = entry;
	      list.push_back(path + entry);
	    }
	}

      // apply path patch only if single match
      if (list.size() == 1)
	list[0] = tpath + lastentry;
    }

  if (list.empty())
    {
      // find intermediate values in path
      String next = prefix.mid(0, len);

      try {
	path += next + prefix[len];
	fill_completion_list_r(path, prefix.mid(len + 1), list, tbl[next], cursor_offset);
      } catch (...) {
      }

    }
}

void State::fill_completion_list(const QString &prefix, QStringList &list, int &cursor_offset)
{
  String path;

  fill_completion_list_r(path, prefix, list, Value(_st, LUA_GLOBALSINDEX), cursor_offset);
}

}

