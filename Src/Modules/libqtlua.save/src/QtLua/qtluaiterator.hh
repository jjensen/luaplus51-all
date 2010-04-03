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


#ifndef QTLUAITERATOR_HH_
#define QTLUAITERATOR_HH_

#include "qtluauserdata.hh"
#include "qtluavalueref.hh"

namespace QtLua {

  /**
   * @short Lua iterator base class
   * @header QtLua/Iterator
   * @module {Base}
   *
   * This abstract class can be subclassed to implement iterators to
   * iterate over user defined objects from both lua and C++ code.
   *
   * @ref UserData based classes can reimplement the @ref
   * UserData::new_iterator function to return a @ref Ref
   * pointer to an @ref Iterator based class. This allow iteration
   * over user defined objects.
   *
   * Some @ref Iterator based classes are already defined internally
   * in QtLua library for iteration over lua tables and other table
   * like @ref UserData based objects.
   *
   * @ref Iterator based classes are used by @ref Value::iterator and
   * @ref Value::const_iterator classes, this allow iteration on lua
   * tables and @ref UserData based objects from C++:
   *
   * @example examples/cpp/value/iterate.cc:1
   *
   * Non const iterator can be used to modify a lua table as well:
   *
   * @example examples/cpp/value/iterate.cc:2
   *
   * The @ref QtLuaLib lua library provides an @tt{each()} lua function
   * which returns a suitable @ref Iterator to iterate over any
   * @ref UserData based object or lua table:
   *
   * @example examples/cpp/value/iterate.cc:3
   */

class Iterator : public UserData
{
public:

  QTLUA_REFTYPE(Iterator);

  /** @return true if more entries are available. */
  virtual bool more() const = 0;
  /** Jump to next entry. */
  virtual void next() = 0;
  /** @return current entry key */
  virtual Value get_key() const = 0;
  /** @return current entry value */
  virtual Value get_value() const = 0;
  /** @return reference to current entry value */
  virtual ValueRef get_value_ref() = 0;
};

}

#endif

