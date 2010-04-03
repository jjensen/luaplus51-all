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

#ifndef QTLUAMETACACHE_HH_
#define QTLUAMETACACHE_HH_

#include <QMap>
#include <QHash>

#include <QtLua/Ref>

namespace QtLua {

  class MetaCache;
  class QObjectWrapper;
  class Member;

  typedef QMap<String, Ref<Member> > member_cache_t;
  typedef QHash<const QMetaObject *, MetaCache> meta_cache_t;

/**
 * @short Cache of existing Qt meta member wrappers (internal)
 * @header internal/MetaCache
 * @module {QObject wrapping}
 * @internal
 *
 * Qt meta properties, enums and methods are constant as described by
 * @ref QMetaObject objects. These meta members are exposed to lua
 * through wrapper objects. This class manages a cache of already
 * created @ref Member based wrappers.
 */

  class MetaCache
  {
    friend class QObjectWrapper;

    MetaCache(const QMetaObject *mo);

  public:
    /** Copy constructor */
    inline MetaCache(const MetaCache &mc);

    /** Get cache meta information for a QObject */
    inline static MetaCache & get_meta(const QObject &obj);
    /** Get cache meta information for a QMetaObject */
    static MetaCache & get_meta(const QMetaObject *mo);

    /** Recursively search for memeber in class and parent classes */
    Ref<Member> get_member(const String &name) const;
    /** Recursively search for memeber in class and parent classes, throw if not found */
    inline Ref<Member> get_member_throw(const String &name) const;
    /** Recursively search for memeber in class and parent classes and
	try to cast to given type, throw if fail. */
    template <class X>
    typename X::ptr get_member_throw(const String &name) const;

    /** Get member table */
    inline const member_cache_t & get_member_table() const;

    /** Get associated QMetaObject pointer */
    inline const QMetaObject * get_meta_object() const;

  private:
    member_cache_t _member_cache;
    const QMetaObject *_mo;
    static meta_cache_t _meta_cache;
  };

}

#endif

