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


#ifndef QTLUAVALUE_HXX_
#define QTLUAVALUE_HXX_

#include <typeinfo>

#include "String"
#include "Iterator"
#include "ValueRef"
#include "String"
#include "qtluastate.hh"
#include "UserData"

namespace QtLua {

  Value::Value(const State &ls)
    : _st(ls._st)
  {
  }

  Value::Value(lua_State *st)
    : _st(st)
  {
  }

  Value::Value(const State &ls, Bool n)
    : _st(ls._st)
  {
    *this = n;
  }

  Value::Value(const State &ls, double n)
    : _st(ls._st)
  {
    *this = n;
  }

  Value::Value(const State &ls, int n)
    : _st(ls._st)
  {
    *this = (double)n;
  }

  Value::Value(const State &ls, const String &str)
    : _st(ls._st)
  {
    *this = str;
  }

  Value::Value(const State &ls, UserData::ptr item)
    : _st(ls._st)
  {
    *this = item;
  }

  Value::Value(const State &ls, QObject *obj)
    : _st(ls._st)
  {
    *this = obj;
  }

  Value::Value(lua_State *st, double n)
    : _st(st)
  {
    *this = n;
  }

  Value & Value::operator=(int n)
  {
    *this = (double)n;
    return *this;
  }

  Value::Value(lua_State *st, const String &str)
    : _st(st)
  {
    *this = str;
  }

  Value::Value(lua_State *st, UserData::ptr item)
    : _st(st)
  {
    *this = item;
  }

  Value::Value(lua_State *st, QObject *obj)
    : _st(st)
  {
    *this = obj;
  }

  template <typename ListContainer>
  inline void Value::from_list(const State &ls, const ListContainer &list)
  {
    *this = Value(ls, TTable);
    for (int i = 0; i < list.size(); i++)
      (*this)[i+1] = list.at(i);
  }

  template <typename ListContainer>
  ListContainer Value::to_list() const
  {
    ListContainer result;

    for (int i = 1; ; i++)
      try {
	result.push_back((*this)[i]);
      } catch (String &e) {
	return result;
      }
  }

  template <typename X>
  inline Value::Value(const State &ls, const QList<X> &list)
    : _st(ls._st)
  {
    from_list<const QList<X> >(ls, list);
  }

  template <typename X>
  inline Value::Value(const State &ls, QList<X> &list)
    : _st(ls._st)
  {
    from_list<QList<X> >(ls, list);
  }

  template <typename X>
  QList<X> Value::to_qlist() const
  {
    return to_list<QList<X> >();
  }

  template <typename X>
  Value::operator QList<X> () const
  {
    return to_qlist<X>();
  }

  template <typename X>
  inline Value::Value(const State &ls, const QVector<X> &vector)
    : _st(ls._st)
  {
    from_list<const QVector<X> >(ls, vector);
  }

  template <typename X>
  inline Value::Value(const State &ls, QVector<X> &vector)
    : _st(ls._st)
  {
    from_list<QVector<X> >(ls, vector);
  }

  template <typename X>
  QVector<X> Value::to_qvector() const
  {
    return to_list<QVector<X> >();
  }

  template <typename X>
  Value::operator QVector<X> () const
  {
    return to_qvector<X>();
  }

  template <typename HashContainer>
  inline void Value::from_hash(const State &ls, const HashContainer &hash)
  {
    *this = Value(ls, TTable);
    for (typename HashContainer::const_iterator i = hash.begin(); i != hash.end(); i++)
      (*this)[i.key()] = Value(ls, i.value());
  }

  template <typename HashContainer>
  inline void Value::from_hash(const State &ls, HashContainer &hash)
  {
    *this = Value(ls, TTable);
    for (typename HashContainer::iterator i = hash.begin(); i != hash.end(); i++)
      (*this)[i.key()] = Value(ls, i.value());
  }

  template <typename HashContainer>
  HashContainer Value::to_hash() const
  {
    HashContainer result;
    for (Value::const_iterator i = begin(); i != end(); i++)
      result[i.key()] = i.value();
    return result;
  }

  template <typename Key, typename Val>
  inline Value::Value(const State &ls, const QHash<Key, Val> &hash)
    : _st(ls._st)
  {
    from_hash<const QHash<Key, Val> >(ls, hash);
  }

  template <typename Key, typename Val>
  inline Value::Value(const State &ls, const QMap<Key, Val> &map)
    : _st(ls._st)
  {
    from_hash<const QMap<Key, Val> >(ls, map);
  }

  template <typename Key, typename Val>
  inline Value::Value(const State &ls, QHash<Key, Val> &hash)
    : _st(ls._st)
  {
    from_hash<QHash<Key, Val> >(ls, hash);
  }

  template <typename Key, typename Val>
  inline Value::Value(const State &ls, QMap<Key, Val> &map)
    : _st(ls._st)
  {
    from_hash<QMap<Key, Val> >(ls, map);
  }

  template <typename Key, typename Val>
  QHash<Key, Val> Value::to_qhash() const
  {
    return to_hash<QHash<Key, Val> >();
  }

  template <typename Key, typename Val>
  Value::operator QHash<Key, Val> () const
  {
    return to_qhash<Key, Val>();
  }

  template <typename Key, typename Val>
  QMap<Key, Val> Value::to_qmap() const
  {
    return to_hash<QMap<Key, Val> >();
  }

  template <typename Key, typename Val>
  Value::operator QMap<Key, Val> () const
  {
    return to_qmap<Key, Val>();
  }

  Value::operator String () const
  {
    return to_string();
  }

  Value::operator QString () const
  {
    return to_string();
  }

  Value::operator double () const
  {
    return to_number();
  }

  Value::operator int () const
  {
    return to_integer();
  }

  Value::operator Bool () const
  {
    return to_boolean();
  }

  Value::List::List()
  {
  }

  Value::List::List(const List &vl)
    : QList<Value>(vl)
  {
  }

  Value::List::List(const Value &v1)
  {
    *this << v1;
  }

  Value::List::List(const Value &v1, const Value &v2)
  {
    *this << v1 << v2;
  }

  Value::List::List(const Value &v1, const Value &v2, const Value &v3)
  {
    *this << v1 << v2 << v3;
  }

  Value::List::List(const Value &v1, const Value &v2, const Value &v3, const Value &v4)
  {
    *this << v1 << v2 << v3 << v4;
  }

  Value::List::List(const Value &v1, const Value &v2, const Value &v3, const Value &v4, const Value &v5)
  {
    *this << v1 << v2 << v3 << v4 << v5;
  }

  Value::List::List(const Value &v1, const Value &v2, const Value &v3, const Value &v4, const Value &v5, const Value &v6)
  {
    *this << v1 << v2 << v3 << v4 << v5 << v6;
  }

#if 0
  Value::List::operator const Value & () const
  {
    assert(!isEmpty());
    return first();
  }
#endif

  Value::List Value::operator() () const
  {
    return this->call(List());
  }

  Value::List Value::operator() (const Value &arg1) const
  {
    List args;
    args << arg1;
    return this->call(args);
  }

  Value::List Value::operator() (const Value &arg1, const Value &arg2) const
  {
    List args;
    args << arg1 << arg2;
    return this->call(args);
  }

  Value::List Value::operator() (const Value &arg1, const Value &arg2, const Value &arg3) const
  {
    List args;
    args << arg1 << arg2 << arg3;
    return this->call(args);
  }

  Value::List Value::operator() (const Value &arg1, const Value &arg2, const Value &arg3,
				 const Value &arg4) const
  {
    List args;
    args << arg1 << arg2 << arg3 << arg4;
    return this->call(args);
  }

  Value::List Value::operator() (const Value &arg1, const Value &arg2, const Value &arg3,
				 const Value &arg4, const Value &arg5) const
  {
    List args;
    args << arg1 << arg2 << arg3 << arg4 << arg5;
    return this->call(args);
  }

  Value::List Value::operator() (const Value &arg1, const Value &arg2, const Value &arg3,
				 const Value &arg4, const Value &arg5, const Value &arg6) const
  {
    List args;
    args << arg1 << arg2 << arg3 << arg4 << arg5 << arg6;
    return this->call(args);
  }

  Value Value::operator[] (const String &key) const
  {
    return (*this)[Value(_st, key)];
  }

  Value Value::operator[] (const char *key) const
  {
    assert(key);
    return (*this)[String(key)];
  }

  Value Value::operator[] (double key) const
  {
    return (*this)[Value(_st, key)];
  }

  Value Value::operator[] (int key) const
  {
    return (*this)[(double)key];
  }

  ValueRef Value::operator[] (const Value &key)
  {
    return ValueRef(*this, key);
  }

  ValueRef Value::operator[] (const String &key)
  {
    return (*this)[Value(_st, key)];
  }

  ValueRef Value::operator[] (const char *key)
  {
    assert(key);
    return (*this)[String(key)];
  }

  ValueRef Value::operator[] (double key)
  {
    return (*this)[Value(_st, (double)key)];
  }

  ValueRef Value::operator[] (int key)
  {
    return (*this)[(double)key];
  }

  inline int Value::to_integer() const
  {
    return (int)to_number();
  }

  template <class X>
  inline QtLua::Ref<X> Value::to_userdata_cast() const
  {
    UserData::ptr ud = to_userdata_null();

    if (!ud.valid())
      throw String("Can not convert % type to %.").arg(type_name()).arg(UserData::type_name<X>());

    Ref<X> ref = ud.dynamiccast<X>();

    if (!ref.valid())
      throw String("Can not convert % type to %.").arg(ud->get_type_name()).arg(UserData::type_name<X>());

    return ref;
  }

  template <class X>
  inline Value::operator Ref<X> () const
  {
    return to_userdata_cast<X>();
  }

  State & Value::get_state() const
  {
    return *State::get_this(_st);
  }

  Value::iterator Value::begin()
  {
    return iterator(new_iterator());
  }

  Value::iterator Value::end()
  {
    return iterator(Ref<Iterator>());
  }

  Value::const_iterator Value::begin() const
  {
    return const_iterator(new_iterator());
  }

  Value::const_iterator Value::end() const
  {
    return const_iterator(Ref<Iterator>());
  }

  ////////////////////////////////////////////////////
  //	Value::value_iterator
  ////////////////////////////////////////////////////

  Value::iterator_::iterator_()
  {
  }

  Value::iterator_::iterator_(Ref<Iterator> i)
    : _i(i)
  {
  }

  Value::iterator_ & Value::iterator_::operator++()
  {
    _i->next();
    return *this;
  }

  Value::iterator_ Value::iterator_::operator++(int)
  {
    iterator_ tmp(*this);
    _i->next();
    return tmp;
  }

  bool Value::iterator_::operator==(const iterator_ &i) const
  {
    return ((!i._i.valid() && !_i->more()) ||
	    (!_i.valid() && !i._i->more()));
  }

  bool Value::iterator_::operator!=(const iterator_ &i) const
  {
    return !(*this == i);
  }

  Value Value::iterator_::key() const
  {
    return _i->get_key();
  }

  Value::const_iterator::const_iterator(Ref<Iterator> i)
    : iterator_(i)
  {
  }

  Value::const_iterator::const_iterator(const iterator &i)
    : iterator_(i)
  {
  }

  Value::const_iterator::const_iterator()
  {
  }

  Value Value::const_iterator::operator* () const
  {
    return _i->get_value();
  }

  Value Value::const_iterator::value() const
  {
    return _i->get_value();
  }

  Value::iterator::iterator(Ref<Iterator> i)
    : iterator_(i)
  {
  }

  Value::iterator::iterator()
  {
  }

  ValueRef Value::iterator::operator* ()
  {
    return _i->get_value_ref();
  }

  ValueRef Value::iterator::value()
  {
    return _i->get_value_ref();
  }

}

#endif

