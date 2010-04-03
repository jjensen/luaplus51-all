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


#ifndef QTLUAVALUEREF_HH_
#define QTLUAVALUEREF_HH_

#include "qtluavalue.hh"

namespace QtLua {

  class State;

  /**
   * @short Lua value reference object class
   * @header QtLua/ValueRef
   * @module {Base}
   *
   * This class acts as a reference to a lua value stored in a lua
   * table (or userdata value). It stores two lua values: a table
   * along with a key value.
   *
   * This is mainly used in the @ref State, @ref Value and
   * @ref Value::iterator classes to allow modification of lua tables with
   * the C++ square bracket operator functions.
   */
  class ValueRef : public Value
  {
    friend class Value;
    friend class State;

  public:
    /** Construct reference with given table and key. */
    ValueRef(const Value &table, const Value &key);

    /** Copy reference object. */
    ValueRef(const ValueRef &ref);

    /** Assign new value to reference. */
    ValueRef & operator=(const Value &v);
    /** Assign new value to reference. */
    inline ValueRef & operator=(const ValueRef &v);
    /** Assign new boolean value to reference. */
    inline ValueRef & operator=(Bool n);
    /** Assign new number value to reference. */
    inline ValueRef & operator=(double n);
    /** Assign new number value to reference. */
    inline ValueRef & operator=(int n);
    /** Assign new string value to reference. */
    inline ValueRef & operator=(const String &str);
    /** Assign new user data value to reference. */
    inline ValueRef & operator=(Ref<UserData> ud);
    /** Assign a wrapped QObject to reference. */
    inline ValueRef & operator=(QObject *obj);

  private:
    void push_value() const;

    Value _key;
  };

}

#endif

