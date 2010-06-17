///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2010 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUAOBJECT_H
#define LUAOBJECT_H

#include "LuaPlusInternal.h"
#include "LuaPlusCD.h"
#include "src/lstate.h"

///////////////////////////////////////////////////////////////////////////////
// namespace LuaPlus
///////////////////////////////////////////////////////////////////////////////
namespace LuaPlus
{

USING_NAMESPACE_LUA;

/**
	Representation of a Lua object.
**/
class LuaObject
{
	friend class LuaState;
public:
	LuaObject();
	LuaObject(lua_State* L) throw();
	LuaObject(LuaState* state) throw();
	LuaObject(lua_State* L, int stackIndex) throw();
	LuaObject(LuaState* state, int stackIndex) throw();
	LuaObject(lua_State* L, int _ref, bool directAssign) throw();
	LuaObject(lua_State* L, bool popTop) throw();
	LuaObject(const LuaObject& src) throw();
	LuaObject(const LuaStackObject& src) throw();
	LuaObject& operator=(const LuaObject& src) throw();
	LuaObject& operator=(const LuaStackObject& src) throw();

/*	template <typename T>
	LuaObject& operator=(const T& value)
	{
		assert(L);
		LCD::Assign(*this, value);
		return *this;
	}*/

	~LuaObject();

	void Reset();

	/**
		Retrieves the LuaState object associated with this LuaObject.
	**/
	LuaState* GetState() const;
	lua_State* GetCState() const;
	int GetRef() const;

	bool operator==(const LuaObject& right) const;
	bool operator<(const LuaObject& right) const;

	const char* TypeName() const;
	int Type() const;

	bool IsNil() const;
	bool IsTable() const;
	bool IsUserData() const;
	bool IsCFunction() const;
	bool IsInteger() const;
	bool IsNumber() const;
	bool IsString() const;
	bool IsConvertibleToInteger() const;
	bool IsConvertibleToNumber() const;
	bool IsConvertibleToString() const;
#if LUA_WIDESTRING
	bool IsWString() const;
	bool IsConvertibleToWString() const;
#endif /* LUA_WIDESTRING */
	bool IsFunction() const;
	bool IsNone() const;
	bool IsLightUserData() const;
	bool IsBoolean() const;

	int ToInteger();
	lua_Number ToNumber();
	const char* ToString();
#if LUA_WIDESTRING
	const lua_WChar* ToWString();
#endif /* LUA_WIDESTRING */
	size_t ObjLen();

	int GetInteger() const;
	float GetFloat() const;
	double GetDouble() const;
	lua_Number GetNumber() const;
	const char* GetString() const;
#if LUA_WIDESTRING
	const lua_WChar* GetWString() const;
#endif /* LUA_WIDESTRING */
	int StrLen();
	lua_CFunction GetCFunction() const;
	void* GetUserData();
	const void* GetLuaPointer();
	void* GetLightUserData() const;
	bool GetBoolean() const;

	LuaStackObject Push() const;

	LuaObject GetMetaTable();
	void SetMetaTable(const LuaObject& valueObj);

	void Insert(LuaObject& obj);
	void Insert(int index, LuaObject& obj);
	void Remove(int index = -1);
	void Sort();

	int GetCount();
	int GetTableCount();

	LuaObject Clone();
	void DeepClone(LuaObject& outObj);

	LuaObject CreateTable(const char* key, int narray = 0, int nrec = 0);
	LuaObject CreateTable(int key, int narray = 0, int nrec = 0);
	LuaObject CreateTable(LuaObject& key, int narray = 0, int nrec = 0);

	template <typename KeyT, typename ValueT> LuaObject& Set(const KeyT& key, const ValueT& value);
	template <typename KeyT, typename ValueT> LuaObject& Set(const KeyT& key, const ValueT& value, int len);
	template <typename KeyT> LuaObject& SetNil(const KeyT& key);

	template <typename KeyT, typename ValueT> LuaObject& RawSet(const KeyT& key, const ValueT& value);
	template <typename KeyT, typename ValueT> LuaObject& RawSet(const KeyT& key, const ValueT& value, int len);
	template <typename KeyT> LuaObject& RawSetNil(const KeyT& key);

	void AssignNil();
	void AssignNil(lua_State* L);
	void AssignNil(LuaState* state);
	template <typename ValueT> void Assign(const ValueT& value);
	void Assign(const char* value, int len);
	template <typename ValueT> void Assign(lua_State* L, const ValueT& value);
	template <typename ValueT> void Assign(LuaState* state, const ValueT& value);
	template <typename ValueT> void Assign(lua_State* L, const ValueT& value, int len);
	template <typename ValueT> void Assign(LuaState* state, const ValueT& value, int len);
#if LUA_WIDESTRING
	void Assign(const lua_WChar* value, int len);
	void Assign(lua_State* L, const lua_WChar* value, int len);
	void Assign(LuaState* state, const lua_WChar* value, int len);
#endif // LUA_WIDESTRING
	void AssignNewTable(int narray = 0, int nrec = 0);
	void AssignNewTable(lua_State* L, int narray = 0, int nrec = 0);
	void AssignNewTable(LuaState* state, int narray = 0, int nrec = 0);

	LuaObject GetByName(const char* name);
	LuaObject GetByIndex(int index);
	LuaObject GetByObject(const LuaStackObject& obj);
	LuaObject GetByObject(const LuaObject& obj);
	LuaObject RawGetByName(const char* name);
	LuaObject RawGetByIndex(int index);
	LuaObject RawGetByObject(const LuaStackObject& obj);
	LuaObject RawGetByObject(const LuaObject& obj);

	// Raw
	LuaObject operator[](const char* name);
	LuaObject operator[](int index);
	LuaObject operator[](const LuaStackObject& obj);
	LuaObject operator[](const LuaObject& obj);

	LuaObject Lookup(const char* key);

	void Register(const char* funcName, lua_CFunction func, int nupvalues = 0);

	void Register(const char* funcName, int (*func)(LuaState*), int nupvalues = 0);

	template <class Callee>
	void Register(const char* funcName, const Callee& callee, int (Callee::*func)(LuaState*), int nupvalues = 0) {
		const void* pCallee = &callee;
		RegisterHelper(funcName, LPCD::LuaStateMemberDispatcherHelper<Callee>::LuaStateMemberDispatcher, nupvalues, &pCallee, sizeof(Callee*), &func, sizeof(func));
	}

	template <class Callee>
	void RegisterObjectFunctor(const char* funcName, int (Callee::*func)(LuaState*), int nupvalues = 0)
	{
		RegisterHelper(funcName, LPCD::Object_MemberDispatcher_to_LuaStateHelper<Callee>::Object_MemberDispatcher_to_LuaState, nupvalues, NULL, 0, &func, sizeof(func));
	}

	template <typename Func>
	inline void RegisterDirect(const char* funcName, Func func, unsigned int nupvalues = 0) {
		RegisterHelper(funcName, LPCD::DirectCallFunctionDispatchHelper<Func>::DirectCallFunctionDispatcher, nupvalues, NULL, 0, &func, sizeof(func));
	}

	template <typename Callee, typename Func>
	inline void RegisterDirect(const char* funcName, const Callee& callee, Func func, unsigned int nupvalues = 0) {
		const void* pCallee = &callee;
		RegisterHelper(funcName, LPCD::DirectCallMemberDispatcherHelper<Callee, Func>::DirectCallMemberDispatcher, nupvalues, &pCallee, sizeof(Callee*), &func, sizeof(func));
	}

	template <typename Callee, typename Func>
	inline void RegisterObjectDirect(const char* funcName, const Callee* callee, Func func, unsigned int nupvalues = 0) {
		RegisterHelper(funcName, LPCD::DirectCallObjectMemberDispatcherHelper<Callee, Func, 2>::DirectCallMemberDispatcher, nupvalues, NULL, 0, &func, sizeof(func));
	}

	void Unregister(const char* funcName);

protected:
	void RegisterHelper(const char* funcName, lua_CFunction function, int nupvalues, const void* callee, unsigned int sizeofCallee, void* func, unsigned int sizeofFunc);

private:
	lua_State* L;
	int ref;
};


} // namespace LuaPlus


namespace LPCD {
	using namespace LuaPlus;

	inline void Push(lua_State* L, const LuaObject& value)
		{  value.Push();  }
	inline void Push(lua_State* L, LuaObject& value)
		{  value.Push();  }
	inline bool	Match(TypeWrapper<LuaObject>, lua_State* L, int idx)
		{  (void)L, (void)idx;  return true;  }
	inline LuaObject		Get(TypeWrapper<LuaObject>, lua_State* L, int idx)
		{  return LuaObject(lua_State_To_LuaState(L), idx);  }

	template <typename Object, typename VarType>
	inline void PropertyCreate(LuaObject& metaTableObj, const char* varName, VarType Object::* var, bool read = true, bool write = true) {
		LuaObject propsObj = metaTableObj["__props"];
		if (propsObj.IsNil()) {
			propsObj = metaTableObj.CreateTable("__props");
		}

		LuaObject varObj = propsObj.CreateTable(varName);

		lua_State* L = metaTableObj.GetCState();

		varObj.Push();

		if (read) {
			lua_pushnumber(L, 1);
			lpcd_pushmemberpropertygetclosure(L, var);
			lua_rawset(L, -3);
		}

		if (write) {
			lua_pushnumber(L, 2);
			lpcd_pushmemberpropertysetclosure(L, var);
			lua_rawset(L, -3);
		}

		lua_pop(L, 1);
	}


	inline void MetaTable_IntegratePropertySupport(LuaObject& metaTableObj) {
		metaTableObj.Register("__index", PropertyMetaTable_index);
		metaTableObj.Register("__newindex", PropertyMetaTable_newindex);
	}


	template <typename Object, typename VarType>
	void Register_MemberPropertyGetFunction(LuaObject& obj, const char* funcName, VarType Object::* var) {
		obj.Push();

		lua_State* L = obj.GetCState();
		lua_pushstring(L, funcName);
		lpcd_pushmemberpropertygetclosure(L, var);
		lua_rawset(L, -3);

		lua_pop(L, 1);
	}

	template <typename Object, typename VarType>
	void Register_MemberPropertySetFunction(LuaObject& obj, const char* funcName, VarType Object::* var) {
		obj.Push();

		lua_State* L = obj.GetCState();
		lua_pushstring(L, funcName);
		lpcd_pushmemberpropertysetclosure(L, var);
		lua_rawset(L, -3);

		lua_pop(L, 1);
	}


	template <typename VarType>
	void Register_GlobalPropertyGetFunction(const LuaObject& obj, const char* funcName, VarType* var) {
		obj.Push();

		lua_State* L = obj.GetCState();
		lua_pushstring(L, funcName);
		lpcd_pushglobalpropertygetclosure(L, var);
		lua_rawset(L, -3);

		lua_pop(L, 1);
	}

	template <typename VarType>
	void Register_GlobalPropertySetFunction(const LuaObject& obj, const char* funcName, VarType* var) {
		obj.Push();

		lua_State* L = obj.GetCState();
		lua_pushstring(L, funcName);
		lpcd_pushglobalpropertysetclosure(L, var);
		lua_rawset(L, -3);

		lua_pop(L, 1);
	}
} // namespace LPCD


namespace LuaPlus {

template <typename T>
inline LuaObject& LuaObject::SetNil(const T& key) {
	luaplus_assert(L  &&  IsTable());
	LPCD::Push(L, key);
	lua_pushnil(L);
	lua_settable(L, ref);
	return *this;
}


template <typename KeyT, typename ValueT>
LuaObject& LuaObject::Set(const KeyT& key, const ValueT& value) {
	luaplus_assert(L  &&  IsTable());
	LPCD::Push(L, key);
	LPCD::Push(L, value);
	lua_settable(L, ref);
	return *this;
}


template <typename KeyT, typename ValueT>
LuaObject& LuaObject::Set(const KeyT& key, const ValueT& value, int len) {
	luaplus_assert(L  &&  IsTable());
	LPCD::Push(L, key);
	LPCD::Push(L, value, len);
	lua_settable(L, ref);
	return *this;
}


template <typename T>
inline LuaObject& LuaObject::RawSetNil(const T& key) {
	luaplus_assert(L  &&  IsTable());
	LPCD::Push(L, key);
	lua_pushnil(L);
	lua_rawset(L, ref);
	return *this;
}


template <typename KeyT, typename ValueT>
LuaObject& LuaObject::RawSet(const KeyT& key, const ValueT& value) {
	luaplus_assert(L  &&  IsTable());
	LPCD::Push(L, key);
	LPCD::Push(L, value);
	lua_rawset(L, ref);
	return *this;
}


template <typename KeyT, typename ValueT>
LuaObject& LuaObject::RawSet(const KeyT& key, const ValueT& value, int len) {
	luaplus_assert(L  &&  IsTable());
	LPCD::Push(L, key);
	LPCD::Push(L, value, len);
	lua_rawset(L, ref);
	return *this;
}


template <typename ValueT>
void LuaObject::Assign(const ValueT& value) {
	luaplus_assert(L);
	lua_fastunref(L, ref);
	LPCD::Push(L, value);
	ref = lua_fastref(L);
}


inline void LuaObject::Assign(const char* value, int len) {
	luaplus_assert(L);
	lua_fastunref(L, ref);
	LPCD::Push(L, value, len);
	ref = lua_fastref(L);
}

#if LUA_WIDESTRING

inline void LuaObject::Assign(const lua_WChar* value, int len) {
	luaplus_assert(L);
	lua_fastunref(L, ref);
	LPCD::Push(L, value, len);
	ref = lua_fastref(L);
}

#endif // LUA_WIDESTRING


template <typename ValueT>
void LuaObject::Assign(LuaState* state, const ValueT& value, int len) {
	if (L)
		lua_fastunref(L, ref);
	L = LuaState_to_lua_State(state);
	LPCD::Push(L, value, len);
	ref = lua_fastref(L);
}


template <typename ValueT>
void LuaObject::Assign(lua_State* _L, const ValueT& value) {
	if (L)
		lua_fastunref(L, ref);
	L = _L;
	LPCD::Push(L, value);
	ref = lua_fastref(L);
}


template <typename ValueT>
void LuaObject::Assign(LuaState* state, const ValueT& value) {
	if (L)
		lua_fastunref(L, ref);
	L = LuaState_to_lua_State(state);
	LPCD::Push(L, value);
	ref = lua_fastref(L);
}


template <typename ValueT>
void LuaObject::Assign(lua_State* _L, const ValueT& value, int len) {
	if (L)
		lua_fastunref(L, ref);
	L = _L;
	LPCD::Push(L, value, len);
	ref = lua_fastref(L);
}


} // namespace LuaPlus

#endif // LUAOBJECT_H
