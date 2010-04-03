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

#ifndef QTLUAMETACACHE_HXX_
#define QTLUAMETACACHE_HXX_

#include <QtLua/String>
#include <internal/Member>

namespace QtLua {

  MetaCache::MetaCache(const MetaCache &mc)
    : _member_cache(mc._member_cache),
      _mo(mc._mo)
  {
  }

  const member_cache_t & MetaCache::get_member_table() const
  {
    return _member_cache;
  }

  const QMetaObject * MetaCache::get_meta_object() const
  {
    return _mo;
  }

  Member::ptr MetaCache::get_member_throw(const String &name) const
  {
    Member::ptr m = get_member(name);

    if (!m.valid())
      throw String("Unknow QObject member '%'.").arg(name);

    return m;
  }

  template <class X>
  typename X::ptr MetaCache::get_member_throw(const String &name) const
  {
    typename X::ptr x = get_member_throw(name).dynamiccast<X>();

    if (!x.valid())
      throw String("Member '%' is not of % type.").arg(name).arg(UserData::type_name<X>());

    return x;
  }

  MetaCache & MetaCache::get_meta(const QObject &obj)
  {
    return get_meta(obj.metaObject());
  }

}

#endif

