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

#include <QSet>
#include <QMetaMethod>

#include <internal/Method>
#include <internal/Enum>
#include <internal/Property>
#include <internal/MetaCache>

namespace QtLua {

  meta_cache_t MetaCache::_meta_cache;

  MetaCache::MetaCache(const QMetaObject *mo)
    : _mo(mo)
  {
    // Fill a set with existing member names in parent classes to
    // detect names collisions

    QSet<String> existing;

    for (const QMetaObject *tmp = mo->superClass(); tmp; tmp = tmp->superClass())
      {
	const member_cache_t &mt = get_meta(tmp).get_member_table();

	for (member_cache_t::const_iterator i = mt.begin(); i != mt.end(); i++)
	  existing.insert(i.key());
      }

    // Add method members
    for (int i = 0; i < mo->methodCount(); i++)
      {
	int index = mo->methodOffset() + i;
	QMetaMethod mm = mo->method(index);

	if (!mm.signature())
	  continue;

	String signature(mm.signature());
	String name(signature.constData(), signature.indexOf('('));

	while (existing.contains(name) || _member_cache.contains(name))
	  name += "_m";

	_member_cache.insert(name, QTLUA_REFNEW(Method, mo, index));
      }

    // Add enum members
    for (int i = 0; i < mo->enumeratorCount(); i++)
      {
	int index = mo->enumeratorOffset() + i;
	QMetaEnum me = mo->enumerator(index);

	if (!me.isValid())
	  continue;

	String name(me.name());

	while (existing.contains(name) || _member_cache.contains(name))
	  name += "_e";

	_member_cache.insert(name, QTLUA_REFNEW(Enum, mo, index));
      }

    // Add 
    for (int i = 0; i < mo->propertyCount(); i++)
      {
	int index = mo->propertyOffset() + i;
	QMetaProperty mp = mo->property(index);

	if (!mp.isValid())
	  continue;

	String name(mp.name());

	while (existing.contains(name) || _member_cache.contains(name))
	  name += "_p";

	_member_cache.insert(name, QTLUA_REFNEW(Property, mo, index));
      }
  }

  Member::ptr MetaCache::get_member(const String &name) const
  {
    const MetaCache *mc = this;
    Member::ptr m;

    for (m = _member_cache.value(name);
	 !m.valid() && mc->_mo->superClass();
	 m = (mc = &MetaCache::get_meta(mc->_mo->superClass()))->get_member(name))
      ;

    return m;
  }

  MetaCache & MetaCache::get_meta(const QMetaObject *mo)
  {
    meta_cache_t::iterator i = _meta_cache.find(mo);

    if (i != _meta_cache.end())
      return i.value();

    return _meta_cache.insert(mo, MetaCache(mo)).value();
  }

}

