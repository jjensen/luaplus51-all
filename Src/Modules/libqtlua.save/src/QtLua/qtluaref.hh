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


#ifndef QTLUAREF_HH_
#define QTLUAREF_HH_

#include <cassert>

namespace QtLua {

#ifndef QTLUA_NO_DEBUG

  /**
   * @short Guard class, assert allocated RefObj based objects get free'd (internal)
   * @module {Base}
   * @internal
   * @header QtLua/Ref
   */

  template <class X>
  struct RefDebug
  {
    RefDebug()
    {
      _alloc_count = 0;
    }

    ~RefDebug()
    {
      assert(_alloc_count == 0 || !"Some allocated objects are not free'd yet");
    }

    static int _alloc_count;
  };

  template <class X>
  int RefDebug<X>::_alloc_count;
#endif

  template <class X>
  class Refobj;

  /**
   * @short Smart pointer with reference counter.
   * @header QtLua/Ref
   * @module {Base}
   *
   * This template class implements a smart pointer with reference
   * counter. 
   *
   * The @ref QtLua::UserData class and derived classes are commonly
   * used with this smart pointer class in QtLua to take advantages of
   * the lua garbage collector. This allows objects to be deleted when
   * no reference are left in both C++ code and lua interpreter state.
   *
   * This smart pointer template class can be used as pointer to
   * objects with class derived from the @ref QtLua::Refobj. Most of the
   * time you need @ref QtLua::UserData based objects and you don't
   * want to inherit from the @ref QtLua::Refobj class directly.
   *
   * A @ref Ref pointer object can be assigned with an object
   * of type X or with an other @ref Ref pointer object.
   *
   * The @ref #QTLUA_REFNEW macro must be used to dynamically create
   * new objects. Objects allocated with this macro will be deleted
   * automatically when no more reference remains.
   *
   * Variable and member objects not allocated with the @ref
   * #QTLUA_REFNEW macro, can be handled too but they won't be
   * automatically deleted. They will still be checked for
   * remaining references when destroyed.
   *
   * Template parameters:
   * @param X Pointed object type, may be const.
   * @param Xnoconst Bare pointed object type. This parameter is optional, default is same as X.
   *
   * Two shortcuts to @tt{Ref<X, X>} and @tt{Ref<const X, X>} types
   * are provided for convenience, the @tt{X::ptr} and @tt{X::const_ptr} types
   * can be defined thanks the @ref #QTLUA_REFTYPE macro.
   */

  template <class X, class Xnoconst = X>
  class Ref
  {
    template <class, class> friend class Ref;

  public:

/**
 * This macro dynamically allocate and construct an object of
 * requested type with given constructor arguments and returns an
 * associated @ref QtLua::Ref object.
 * 
 * @param X object type to construct
 * @param ... constructor arguments
 *
 * Usage example:
 *
 * @example examples/cpp/userdata/ref.cc:3|5
 */
#define QTLUA_REFNEW(X, ...)			\
 (X::ptr::allocated(new X(__VA_ARGS__)))

/**
 * This macro may be used to declare the X::ptr and X::const_ptr
 * shortcuts to @ref QtLua::Ref types in class derived from @ref
 * QtLua::Refobj. It should be invoked from class body public part.
 *
 * @param X macro invocation class.
 *
 * Usage example:
 *
 * @example examples/cpp/userdata/ref.cc:1|2
 * @showcontent
 */
#define QTLUA_REFTYPE(X)					 \
 /** Shortcut for @ref QtLua::Ref smart pointer class to X type provided for convenience */ \
 typedef QtLua::Ref<const X, X> const_ptr;			 \
 /** Shortcut for @ref QtLua::Ref smart pointer class to X type provided for convenience */ \
 typedef QtLua::Ref<X, X> ptr;

    /** Construct a null reference. */
    Ref()
      : _obj(0)
    {
    }

    /** Construct a const Ref from non const Ref. */
    Ref(const Ref<Xnoconst, Xnoconst> & r)
      : _obj(r._obj)
    {
      if (_obj)
	_obj->_inc();
    }

    /** Construct a const Ref from const Ref. */
    Ref(const Ref<const Xnoconst, Xnoconst> & r)
      : _obj(r._obj)
    {
      if (_obj)
	_obj->_inc();
    }

    /** Construct a const Ref from derived class Ref. */
    template <class T>
    Ref(const Ref<T, T> & r)
      : _obj(r._obj)
    {
      if (_obj)
	_obj->_inc();
    }

    /** Construct a const Ref from derived class const Ref. */
    template <class T>
    Ref(const Ref<const T, T> & r)
      : _obj(r._obj)
    {
      if (_obj)
	_obj->_inc();
    }

    /** Construct a Ref which points to specified object. */
    Ref(X & obj)
      : _obj(&obj)
    {
      _obj->_inc();
    }

    /**
     * Construct Ref from dynamically allocated object pointer.
     * Pointed object is marked as deletable when last reference is destroyed.

     * @internal
     */
    static Ref allocated(X * obj)
    {
#ifndef QTLUA_NO_DEBUG
      RefDebug<X>::_alloc_count++;
#endif
      obj->_qtlua_Ref_delete = true;
      return Ref(obj);
    }

    /** Initialize Ref from Ref */
    Ref & operator=(const Ref &r)
    {
      *this = *r._obj;
      return *this;
    }

    /** Initialize Ref from object Reference */
    Ref & operator=(X & obj)
    {
      X *tmp = _obj;
      _obj = 0;
      if (tmp)
	tmp->_drop();
      _obj = &obj;
      if (_obj)
	_obj->_inc();
      return *this;
    }

    /** Dynamic cast Ref to Ref of given type */
    template <class T>
    Ref<T, T> dynamiccast() const
    {
      return Ref<T, T>(dynamic_cast<T*>(_obj));
    }

    /** Dynamic cast Ref to const Ref of given type */
    template <class T>
    Ref<const T, T> dynamiccast_const() const
    {
      return Ref<const T, T>(dynamic_cast<const T*>(_obj));
    }

    /** Drop a Ref */
    ~Ref()
    {
      if (_obj)
	_obj->_drop();
    }

    /** Invalidate Ref (set internal pointer to null) */
    void invalidate()
    {
      X *tmp = _obj;
      _obj = 0;
      if (tmp)
	tmp->_drop();
    }

    /** Test if Ref is valid (check if internal pointer is not null) */
    bool valid() const
    {
      return _obj != 0;
    }

    /** Access object */
    X & operator*() const
    {
      assert(_obj);
      return *_obj;
    }

    /** Access object */
    X * operator->() const
    {
      assert(_obj);
      return _obj;
    }

    /** Get Ref internal object pointer */
    X * ptr() const
    {
      return _obj;
    }

    /** Get object Reference count */
    int count() const
    {
      return _obj ? _obj->_qtlua_Ref_count : 0;
    }

    /** Test if pointed ojects are the same */
    bool operator==(const Ref &r) const
    {
      return _obj == r._obj;
    }

    /** Test if pointed ojects are not the same */
    bool operator!=(const Ref &r) const
    {
      return _obj != r._obj;
    }

  protected:

    explicit Ref(X * obj)
      : _obj(obj)
    {
      if (_obj)
	_obj->_inc();
    }

    X *_obj;
  };


  /**
   * @short Referenced objects base class (internal)
   * @header QtLua/Ref
   * @module {Base}
   * @internal
   *
   * This template class must be a base class for any class which may
   * be referenced by the @ref QtLua::Ref smart pointer.
   * @see QtLua::UserData.
   */
  template <class X>
  class Refobj
  {
    template <class, class> friend class Ref;

  public:
    QTLUA_REFTYPE(X);

    Refobj()
      : _qtlua_Ref_count(0),
	_qtlua_Ref_delete(false)
    {
    }

    Refobj(const Refobj &r)
      : _qtlua_Ref_count(0),
	_qtlua_Ref_delete(false)
    {
    }

    Refobj & operator=(const Refobj &r)
    {
      assert(_qtlua_Ref_count == 0 || !"Can not overwrite object with live References");
      return *this;
    }

    virtual ~Refobj()
    {
      assert(_qtlua_Ref_count == 0 || !"Can not destruct object with live References");
    }

  protected:

    /** @internal */
    void _inc() const
    {
      Refobj<X> *y = const_cast<Refobj<X>*>(this);
      y->ref_inc(++y->_qtlua_Ref_count);
    }

    /** @internal */
    void _drop() const
    {
      Refobj<X> *y = const_cast<Refobj<X>*>(this);
      assert(_qtlua_Ref_count > 0);
      int count = --y->_qtlua_Ref_count;

      if (count == 0 && _qtlua_Ref_delete)
	{
#ifndef QTLUA_NO_DEBUG
	  RefDebug<X>::_alloc_count--;
#endif
	  delete this;
	  return;
	}

      y->ref_drop(count);
    }

    /** This function is called when reference count has just increased.

	@param Count new reference count.
    */
    virtual void ref_inc(int count)
    {
    }

    /** This functions is called when reference count has just decreased.

	@param Count new reference count.
    */
    virtual void ref_drop(int count)
    {
    }

    /** Get object current reference count */
    int ref_count() const
    {
      return _qtlua_Ref_count;
    }

    /** @internal */
    typedef X _qtlua_Ref_base_type;

    /** @internal Reference counter value */
    int _qtlua_Ref_count;

    /** @internal delete object pointer when refcount reach zero */
    bool _qtlua_Ref_delete;
  };

}

#endif

