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


#ifndef QTLUASTATE_HH_
#define QTLUASTATE_HH_

#include <QIODevice>
#include <QObject>
#include <QHash>

#include "qtluastring.hh"
#include "qtluavalue.hh"
#include "qtluavalueref.hh"

struct lua_State;

namespace QtLua {

  class UserData;
  class QObjectWrapper;

  /** @internal */
  typedef QHash<QObject *, QObjectWrapper *> wrapper_hash_t;

  /** Specify lua standard libraries and QtLua lua libraries to load
      with the @ref State::openlib function. */
  enum Library
    {
      BaseLib,		//< standard lua base library
      PackageLib,	//< standard lua package library
      StringLib,	//< standard lua string library
      TableLib,		//< standard lua table library
      MathLib,		//< standard lua math library
      IoLib,		//< standard lua io library
      OsLib,		//< standard lua os library
      DebugLib,		//< standard lua debug library
      QtLuaLib,		//< lua library with base functions, see @xref{Predefined lua functions} section.
      QtLib,		//< lua library with wrapped Qt functions, see @xref{Wrapped Qt functions} section.
      AllLibs,		//< All libraries wildcard
    };

  /**
   * @short Lua interpreter state wrapper class
   * @header QtLua/State
   * @module {Base}
   *
   * This class wraps the lua interpreter state.
   * 
   * This class provides various functions to execute lua code, access
   * lua variables from C++ and load lua libraries.
   *
   * Some functions in this class may throw an exception to handle lua
   * errors, see @xref{Error handling and exceptions}.
   *
   * This class provides Qt slots and signals for use with the @ref
   * Console widget. This enables table names completion and error
   * messages reporting on user console.
   */

class State : public QObject
{
  Q_OBJECT

  friend class QObjectWrapper;
  friend class UserData;
  friend class Value;
  friend class ValueRef;

public:

  /** Create a lua interpreter state object. */
  State();

  /** 
   * Lua interpreter state is checked for remaining @ref Value objects
   * with references to @ref UserData objects when destroyed.
   *
   * Program is aborted if such references are found because these
   * objects would try to access the destroyed @ref State later and
   * probably crash.
   *
   * QtLua takes care of clearing all global variables before
   * performing this sanity check.
   */
  ~State();

  /** 
   * Execute a lua chuck read from @ref QIODevice .
   * @xsee{Error handling and exceptions}
   */
  Value::List exec_chunk(QIODevice &io);

  /**
   * Execute a lua script string.
   * @xsee{Error handling and exceptions}
   */
  Value::List exec_statements(const String &statements);

  /** Initiate a garbage collection cycle. This is useful to ensure
      all unused @ref UserData based objects are destroyed. */
  void gc_collect();

  /** Set a global variable. If path contains '.', intermediate tables
      will be created on the fly. The @ref __operator_sqb2__ function may be
      used if no intermediate table access is needed. */
  void set_global(const String &path, const Value &value);

  /** Get global variable. If path contains '.', intermediate tables
      will be accessed. The @ref __operator_sqb1__ function may be used if no
      intermediate table access is needed. */
  Value get_global(const String &path) const;

  /**
   * Index operation on global table. This function return a @ref
   * Value object which is a @strong copy of the requested global
   * variable. This value can be modified but will not change the
   * original lua variable.
   *
   * @example examples/cpp/value/global.cc:i1|i2|1
   * @alias operator_sqb1
   */
  Value operator[] (const Value &key) const;

  /**
   * Index operation on global table, shortcut for string key access
   * @see __operator_sqb1__
   */
  inline Value operator[] (const String &key) const;

  /**
   * Index operation on global table. This function return a @ref
   * ValueRef object which is a modifiable reference to requested
   * global variable. It can assigned to modify original lua variable:
   *
   * @example examples/cpp/value/global.cc:i1|2
   * @alias operator_sqb2
   */
  ValueRef operator[] (const Value &key);

  /**
   * Index operation on global table, shortcut for string key access.
   * @see __operator_sqb2__
   */
  inline ValueRef operator[] (const String &key);

  /** 
   * This function open a lua standard library or QtLua lua library.
   * @see QtLua::Library
   * @xsee{QtLua lua libraries}
   */
  void openlib(Library lib);

  /** 
   * Call given function pointer with internal @ref lua_State
   * pointer. Can be used to register extra libraries or access
   * internal lua interpreter directly.
   *
   * Use with care if you are nor familiar with the lua C API.
   */
  void lua_do(void (*func)(lua_State *st));

public slots:

  /**
   * This slot function execute the given script string and initiate a
   * garbage collection cycle. It will catch and print lua
   * errors using the @ref output signal.
   * @see Console
   */
  void exec(const QString &statements);

  /**
   * Lua global variables completion handler. May be connected to
   * Console widget for default global variables completion behavior.
   */
  void fill_completion_list(const QString &prefix, QStringList &list, int &cursor_offset);

signals:

  /**
   * Text output signal. This signal is used to report errors and
   * on lua @tt{print()} function call (see @xref{Predefined lua functions}).
   */
  void output(const QString &str);

private:

  // get pointer to lua state object from lua state
  static State *get_this(lua_State *st);

  void fill_completion_list_r(String &path, const String &prefix,
			      QStringList &list, const Value &tbl,
			      int &cursor_offset);

  bool set_global_r(const String &name, const Value &value, int tblidx);
  void get_global_r(const String &name, Value &value, int tblidx) const;

  void reg_c_function(const char *name, int (*fcn)(lua_State *));

  // lua c functions
  static int lua_panic(lua_State *st);
  static int lua_cmd_iterator(lua_State *st);
  static int lua_cmd_each(lua_State *st);
  static int lua_cmd_print(lua_State *st);
  static int lua_cmd_list(lua_State *st);
  static int lua_cmd_help(lua_State *st);
  static int lua_cmd_plugin(lua_State *st);

  // lua meta methods functions
  static int lua_meta_item_add(lua_State *st);
  static int lua_meta_item_sub(lua_State *st);
  static int lua_meta_item_mul(lua_State *st);
  static int lua_meta_item_div(lua_State *st);
  static int lua_meta_item_mod(lua_State *st);
  static int lua_meta_item_pow(lua_State *st);
  static int lua_meta_item_unm(lua_State *st);
  static int lua_meta_item_concat(lua_State *st);
  static int lua_meta_item_len(lua_State *st);
  static int lua_meta_item_eq(lua_State *st);
  static int lua_meta_item_lt(lua_State *st);
  static int lua_meta_item_le(lua_State *st);
  static int lua_meta_item_index(lua_State *st);
  static int lua_meta_item_newindex(lua_State *st);
  static int lua_meta_item_call(lua_State *st);
  static int lua_meta_item_gc(lua_State *st);

  // static member addresses are used as lua registry table keys
  static char _key_item_metatable;
  static char _key_this;

  // QObjects wrappers are referenced here
  wrapper_hash_t _whash;

  lua_State	*_st;
};

}

#endif

