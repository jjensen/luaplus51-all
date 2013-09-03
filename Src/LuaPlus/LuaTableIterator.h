///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2011 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUAPLUS__LUATABLEITERATOR_H
#define LUAPLUS__LUATABLEITERATOR_H

#include "LuaPlusInternal.h"
#include "LuaObject.h"

///////////////////////////////////////////////////////////////////////////////
// namespace LuaPlus
///////////////////////////////////////////////////////////////////////////////
namespace LuaPlus
{

/**
	The LuaTableIterator class provides a far simpler, safer, and more natural
	looking table iteration method.
**/
class LuaTableIterator
{
public:
	LuaTableIterator(const LuaObject& tableObj, bool doReset = true);
	~LuaTableIterator();
	void Reset();
	void Invalidate();
	bool Next();
	bool IsValid() const;
	LuaTableIterator& operator++();
	operator bool() const;
	LuaObject& GetKey();
	LuaObject& GetValue();

protected:

private:
	/**
		Don't allow copies.
	**/
	LuaTableIterator& operator=( const LuaTableIterator& iter );
	LuaTableIterator( const LuaTableIterator& iter );

	LuaObject m_keyObj;
	LuaObject m_valueObj;
	LuaObject m_tableObj;				///< The table object being iterated.
	bool m_isDone;
};


/**
	\param tableObj The table to iterate the contents of.
	\param doReset If true, the Reset() function is called at constructor
		initialization time, allowing the iterator to be used immediately.
		If false, then Reset() must be called before iterating.
**/
inline LuaTableIterator::LuaTableIterator( const LuaObject& tableObj, bool doReset ) :
	m_keyObj(tableObj.GetState()),
	m_valueObj(tableObj.GetState()),
	m_tableObj(tableObj),
	m_isDone(false) {
	luaplus_assert(tableObj.IsTable());

	// If the user requested it, perform the automatic reset.
	if ( doReset )
		Reset();
}


/**
	The destructor.
**/
inline LuaTableIterator::~LuaTableIterator() {
}


/**
	Start iteration at the beginning of the table.
**/
inline void LuaTableIterator::Reset() {
	// Start afresh...
	LuaState* state = m_tableObj.GetState();

	// Start at the beginning.
	m_keyObj.AssignNil(state);

	// Start the iteration.  If the return value is 0, then the iterator
	// will be invalid.
//	m_isDone = !LuaPlusH_next(state, &m_tableObj, &m_keyObj, &m_valueObj);
#if LUA_FASTREF_SUPPORT
	m_keyObj.Push(state);
	m_isDone = lua_next(m_tableObj.GetCState(), m_tableObj.GetRef()) == 0;
#else
	lua_getfastref(m_tableObj.GetCState(), m_tableObj.GetRef());
	m_keyObj.Push(state);
	m_isDone = lua_next(m_tableObj.GetCState(), -2) == 0;
	lua_remove(m_tableObj.GetCState(), m_isDone ? -1 : -3);
#endif // !LUA_FASTREF_SUPPORT
	if (m_isDone) {
		m_keyObj.Reset();
		m_valueObj.Reset();
	} else {
		m_keyObj = LuaObject(m_tableObj.GetCState(), -2);
		m_valueObj = LuaObject(m_tableObj.GetCState(), -1);
		lua_pop(m_tableObj.GetCState(), 2);
	}
}


/**
	Invalidates the iterator.  Call this function if you early abort from
	an iteration loop (such as before it hits the end).
**/
inline void LuaTableIterator::Invalidate() {
	// This is a local helper variable so we don't waste space in the class
	// definition.
	LuaState* state = m_tableObj.GetState();
	m_keyObj.AssignNil(state);
	m_valueObj.AssignNil(state);
}

/**
	Go to the next entry in the table.

	\return Returns true if the iteration is done.
**/
inline bool LuaTableIterator::Next() {
	// This function is only active if Reset() has been called head.
	luaplus_assert( IsValid() );

	// This is a local helper variable so we don't waste space in the class
	// definition.
	LuaState* state = m_tableObj.GetState();

	// Do the Lua table iteration.
#if LUA_FASTREF_SUPPORT
	m_keyObj.Push(state);
	m_isDone = lua_next(m_tableObj.GetCState(), m_tableObj.GetRef()) == 0;
#else
	lua_getfastref(m_tableObj.GetCState(), m_tableObj.GetRef());
	m_keyObj.Push(state);
	m_isDone = lua_next(m_tableObj.GetCState(), -2) == 0;
	lua_remove(m_tableObj.GetCState(), m_isDone ? -1 : -3);
#endif // LUA_FASTREF_SUPPORT
	if (!m_isDone) {
		m_keyObj = LuaObject(m_tableObj.GetCState(), -2);
		m_valueObj = LuaObject(m_tableObj.GetCState(), -1);
		lua_pop(m_tableObj.GetCState(), 2);
	}
	return !m_isDone;
}


/**
	\return Returns true if the iterator is valid (there is a current element).
**/
inline bool LuaTableIterator::IsValid() const {
	return !m_isDone;
}


/**
	We can easily allow a prefix operator++.  Postfix would be a stack
	management nightmare.
**/
inline LuaTableIterator& LuaTableIterator::operator++() {
	Next();
	return *this;
}


/**
	\return Returns true if the iterator is valid (there is a current element).
**/
inline LuaTableIterator::operator bool() const {
	// If the iterator is valid, then we're good.
	return IsValid();
}


/**
	\return Returns a LuaObject describing the current key.
**/
inline LuaObject& LuaTableIterator::GetKey() {
	// This function is only active if Reset() has been called head.
	luaplus_assert( IsValid() );

	return m_keyObj;
}


/**
	\return Returns a LuaObject describing the current value.
**/
inline LuaObject& LuaTableIterator::GetValue() {
	// This function is only active if Reset() has been called head.
	luaplus_assert( IsValid() );

	return m_valueObj;
}

} // namespace LuaPlus

#endif // LUAPLUS__LUATABLEITERATOR_H
