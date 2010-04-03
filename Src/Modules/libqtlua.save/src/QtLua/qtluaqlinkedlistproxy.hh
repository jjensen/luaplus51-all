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

#ifndef QTLUALINKEDLISTPROXY_HH_
#define QTLUALINKEDLISTPROXY_HH_

#include "qtluauserdata.hh"
#include "qtluaiterator.hh"

namespace QtLua {

  /**
   * @short QLinkedList access wrapper for lua script
   * @header QtLua/QLinkedListProxy
   * @module {Container proxies}
   *
   * This template class may be used to iterate over an attached @ref QLinkedList
   * container object from lua script.
   *
   * Containers may be attached and detached from the wrapper object
   * to solve cases where we want to destroy the container when lua
   * still holds references to the wrapper object. When no container
   * is attached access will raise an error.
   */

template <class Container>
class QLinkedListProxy : public UserData
{
public:
  QTLUA_REFTYPE(QLinkedListProxy);

  /** Create a @ref QListProxy object with no attached container */
  QLinkedListProxy();
  /** Create a @ref QListProxy object and attach given container */
  QLinkedListProxy(Container &list);

  /** Attach or detach container. argument may be NULL */
  void set_container(Container *list);

  Ref<Iterator> new_iterator(State &ls);

private:

  /**
   * @short QLinkedListProxy iterator class (internal)
   * @internal
   */
  class ProxyIterator : public Iterator
  {
  public:
    QTLUA_REFTYPE(ProxyIterator);
    ProxyIterator(State &ls, QLinkedListProxy::ptr proxy);

  private:
    bool more() const;
    void next();
    Value get_key() const;
    Value get_value() const;
    ValueRef get_value_ref();

    State &_ls;
    typename QLinkedListProxy::ptr _proxy;
    typename Container::const_iterator _it;
    unsigned int _i;
  };

  Container *_linkedlist;
};

}

#endif

