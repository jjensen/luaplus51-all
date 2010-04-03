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

#ifndef QTLUAVECTORPROXY_HH_
#define QTLUAVECTORPROXY_HH_

#include "qtluauserdata.hh"
#include "qtluaiterator.hh"

namespace QtLua {

  /**
   * @short QVector read only access wrapper for lua script
   * @header QtLua/QVectorProxy
   * @module {Container proxies}
   *
   * This template class may be used to expose an attached @ref QVector
   * container object to lua script for read access. The
   * @ref QVectorProxy class may be used for read/write access.
   *
   * QVector may be resized if accessing above current size, depending
   * on @tt resize template parameter value.
   *
   * See @ref QVectorProxy class documentation for details and examples.
   */

template <class Container, bool resize = false>
class QVectorProxyRo : public UserData
{
public:
  QTLUA_REFTYPE(QVectorProxyRo);

  /** Create a @ref QVectorProxy object with no attached container */
  QVectorProxyRo();
  /** Create a @ref QVectorProxy object and attach given container */
  QVectorProxyRo(Container &vector);

  /** Attach or detach container. argument may be NULL */
  void set_container(Container *vector);

  Value meta_operation(State &ls, Operation op, const Value &a, const Value &b);
  Value meta_index(State &ls, const Value &key);
  Ref<Iterator> new_iterator(State &ls);

private:

  /**
   * @short QVectorProxyRo iterator class (internal)
   * @internal
   */
  class ProxyIterator : public Iterator
  {
  public:
    QTLUA_REFTYPE(ProxyIterator);
    ProxyIterator(State &ls, QVectorProxyRo::ptr proxy);

  private:
    bool more() const;
    void next();
    Value get_key() const;
    Value get_value() const;
    ValueRef get_value_ref();

    State &_ls;
    typename QVectorProxyRo::ptr _proxy;
    unsigned int _it;
  };

protected:
  Container *_vector;
};

  /**
   * @short QVector access wrapper for lua script
   * @header QtLua/QVectorProxy
   * @module {Container proxies}
   *
   * This template class may be used to expose an attached @ref QVector
   * container object to lua script for read and write access. The
   * @ref QVectorProxyRo class may be used for read only access.
   *
   * Containers may be attached and detached from the wrapper object
   * to solve cases where we want to destroy the container when lua
   * still holds references to the wrapper object. When no container
   * is attached access will raise an error.
   *
   * First entry has index 1. Lua @tt nil value is returned if no such
   * entry exists on table read. A @tt nil value write will delete
   * entry at given index. Write access at vector size + 1 append a new
   * entry to the vector if the @tt resize template argument is true.
   *
   * QVector may be resized if accessing above current size, depending
   * on @tt resize template parameter value.
   *
   * Lua operator @tt # returns the container entry count. Lua
   * operator @tt - returns a lua table copy of the container.
   *
   * The following example show how a @ref QVector object can be
   * accessed from both C++ and lua script directly:
   *
   * @example examples/cpp/proxy/qvectorproxy_string.cc:1|2|3
   */

template <class Container, bool resize = false>
class QVectorProxy : public QVectorProxyRo<Container, resize>
{
  using QVectorProxyRo<Container, resize>::_vector;

public:
  QTLUA_REFTYPE(QVectorProxy);

  /** Create a @ref QVectorProxy object */
  QVectorProxy();
  /** Create a @ref QVectorProxy object */
  QVectorProxy(Container &vector);

  void meta_newindex(State &ls, const Value &key, const Value &value);
};

}

#endif

