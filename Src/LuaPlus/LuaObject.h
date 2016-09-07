///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2011 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUAPLUS__LUAOBJECT_H
#define LUAPLUS__LUAOBJECT_H

#include "LuaPlusInternal.h"
#include "LuaPlusCD.h"
#include "LuaStateCD.h"

///////////////////////////////////////////////////////////////////////////////
// namespace LuaPlus
///////////////////////////////////////////////////////////////////////////////
namespace LuaPlus
{

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

	bool IsValid() const;
	bool IsNil() const;
	bool IsTable() const;
	bool IsUserdata() const;
	bool IsCFunction() const;
	bool IsInteger() const;
	bool IsNumber() const;
	bool IsString() const;
	bool IsConvertibleToInteger() const;
	bool IsConvertibleToNumber() const;
	bool IsConvertibleToString() const;
	bool IsFunction() const;
	bool IsNone() const;
	bool IsLightUserdata() const;
	bool IsBoolean() const;

	lua_Integer ToInteger();
	lua_Number ToNumber();
	const char* ToString();
	size_t ObjLen();

	lua_Integer GetInteger() const;
	float GetFloat() const;
	double GetDouble() const;
	lua_Number GetNumber() const;
	const char* GetString() const;
	size_t StrLen() const;
	lua_CFunction GetCFunction() const;
	void* GetUserdata() const;
	const void* GetLuaPointer() const;
	void* GetLightUserdata() const;
	bool GetBoolean() const;

	LuaStackObject Push(lua_State* L) const;
	LuaStackObject Push(LuaState* state) const;

	LuaObject GetMetatable() const;
	LuaObject& SetMetatable(const LuaObject& valueObj);

	void Insert(LuaObject& obj);
	void Insert(int index, LuaObject& obj);
	void Remove(int index = -1);
	void Sort();

	size_t GetCount() const;
	size_t GetTableCount() const;

	LuaObject Clone() const;
	void DeepClone(LuaObject& outObj) const;

	LuaObject CreateTable(const char* key, int narray = 0, int nrec = 0) const;
	LuaObject CreateTable(int key, int narray = 0, int nrec = 0) const;
	LuaObject CreateTable(const LuaObject& key, int narray = 0, int nrec = 0) const;

	template <typename KeyT, typename ValueT> LuaObject& Set(const KeyT& key, const ValueT& value);
	template <typename KeyT, typename ValueT> LuaObject& Set(const KeyT& key, const ValueT& value, int len);
	template <typename KeyT> LuaObject& SetNil(const KeyT& key);

	template <typename KeyT, typename ValueT> LuaObject& RawSet(const KeyT& key, const ValueT& value);
	template <typename KeyT, typename ValueT> LuaObject& RawSet(const KeyT& key, const ValueT& value, int len);
	template <typename KeyT> LuaObject& RawSetNil(const KeyT& key);

	LuaObject& AssignNil();
	LuaObject& AssignNil(lua_State* L);
	LuaObject& AssignNil(LuaState* state);
	template <typename ValueT> LuaObject& Assign(const ValueT& value);
	LuaObject& Assign(lua_State* L, const char* value, int len);
	LuaObject& Assign(LuaState* state, const char* value, int len);
	template <typename ValueT> LuaObject& Assign(lua_State* L, const ValueT& value);
	template <typename ValueT> LuaObject& Assign(LuaState* state, const ValueT& value);
	template <typename ValueT> LuaObject& Assign(lua_State* L, const ValueT& value, int len);
	template <typename ValueT> LuaObject& Assign(LuaState* state, const ValueT& value, int len);
	LuaObject& AssignNewTable(int narray = 0, int nrec = 0);
	LuaObject& AssignNewTable(lua_State* L, int narray = 0, int nrec = 0);
	LuaObject& AssignNewTable(LuaState* state, int narray = 0, int nrec = 0);

	LuaObject& SetBoolean(const char* key, bool value);
	LuaObject& SetBoolean(int key, bool value);
	LuaObject& SetBoolean(LuaObject& key, bool value);
	LuaObject& SetInteger(const char* key, lua_Integer value);
	LuaObject& SetInteger(int key, lua_Integer value);
	LuaObject& SetInteger(LuaObject& key, lua_Integer value);
	LuaObject& SetNumber(const char* key, lua_Number value);
	LuaObject& SetNumber(int key, lua_Number value);
	LuaObject& SetNumber(LuaObject& key, lua_Number value);
	LuaObject& SetString(const char* key, const char* value, int len = -1);
	LuaObject& SetString(int key, const char* value, int len = -1);
	LuaObject& SetString(LuaObject& key, const char* value, int len = -1);
	LuaObject& SetUserdata(const char* key, void* value);
	LuaObject& SetUserdata(int key, void* value);
	LuaObject& SetUserdata(LuaObject& key, void* value);
	LuaObject& SetLightUserdata(const char* key, void* value);
	LuaObject& SetLightUserdata(int key, void* value);
	LuaObject& SetLightUserdata(LuaObject& key, void* value);
	LuaObject& SetObject(const char* key, const LuaObject& value);
	LuaObject& SetObject(int key, const LuaObject& value);
	LuaObject& SetObject(LuaObject& key, const LuaObject& value);

	LuaObject& RawSetBoolean(const char* key, bool value);
	LuaObject& RawSetBoolean(int key, bool value);
	LuaObject& RawSetBoolean(LuaObject& key, bool value);
	LuaObject& RawSetInteger(const char* key, lua_Integer value);
	LuaObject& RawSetInteger(int key, lua_Integer value);
	LuaObject& RawSetInteger(LuaObject& key, lua_Integer value);
	LuaObject& RawSetNumber(const char* key, lua_Number value);
	LuaObject& RawSetNumber(int key, lua_Number value);
	LuaObject& RawSetNumber(LuaObject& key, lua_Number value);
	LuaObject& RawSetString(const char* key, const char* value, int len = -1);
	LuaObject& RawSetString(int key, const char* value, int len = -1);
	LuaObject& RawSetString(LuaObject& key, const char* value, int len = -1);
	LuaObject& RawSetUserdata(const char* key, void* value);
	LuaObject& RawSetUserdata(int key, void* value);
	LuaObject& RawSetUserdata(LuaObject& key, void* value);
	LuaObject& RawSetLightUserdata(const char* key, void* value);
	LuaObject& RawSetLightUserdata(int key, void* value);
	LuaObject& RawSetLightUserdata(LuaObject& key, void* value);
	LuaObject& RawSetObject(const char* key, LuaObject& value);
	LuaObject& RawSetObject(int key, LuaObject& value);
	LuaObject& RawSetObject(LuaObject& key, LuaObject& value);

	LuaObject& AssignBoolean(LuaState* state, bool value);
	LuaObject& AssignInteger(LuaState* state, lua_Integer value);
	LuaObject& AssignNumber(LuaState* state, lua_Number value);
	LuaObject& AssignString(LuaState* state, const char* value, int len = -1);

	LuaObject& AssignUserdata(LuaState* state, void* value);
	LuaObject& AssignUserdata(LuaState* state, size_t size);
	LuaObject& AssignLightUserdata(LuaState* state, void* value);
	LuaObject& AssignObject(LuaObject& value);		// Should this function be removed??

	LuaObject& AssignCFunction(LuaState* state, lua_CFunction func, int nupvalues = 0);
	LuaObject& AssignCFunction(LuaState* state, int (*func)(LuaState*), int nupvalues = 0);

	template <class Callee>
	LuaObject& AssignCFunction(LuaState* state, const Callee& callee, int (Callee::*func)(LuaState*), int nupvalues = 0) {
		const void* pCallee = &callee;
		return AssignCFunctionHelper(state, LPCD::LuaStateMemberDispatcherHelper<Callee>::LuaStateMemberDispatcher, nupvalues, &pCallee, sizeof(Callee*), &func, sizeof(func));
	}

	template <class Callee>
	LuaObject& AssignCFunctionObjectFunctor(LuaState* state, const char* funcName, int (Callee::*func)(LuaState*), int nupvalues = 0) {
		return AssignCFunctionHelper(state, LPCD::Object_MemberDispatcher_to_LuaStateHelper<Callee>::Object_MemberDispatcher_to_LuaState, nupvalues, NULL, 0, &func, sizeof(func));
	}

	template <typename Func>
	inline LuaObject& AssignCFunctionDirect(LuaState* state, Func func, unsigned int nupvalues = 0) {
		return AssignCFunctionHelper(state, LPCD::DirectCallFunctionDispatchHelper<Func>::DirectCallFunctionDispatcher, nupvalues, NULL, 0, &func, sizeof(func));
	}

	template <typename Callee, typename Func>
	inline LuaObject& AssignCFunctionDirect(LuaState* state, const Callee& callee, Func func, unsigned int nupvalues = 0) {
		const void* pCallee = &callee;
		return AssignCFunctionHelper(state, LPCD::DirectCallMemberDispatcherHelper<Callee, Func>::DirectCallMemberDispatcher, nupvalues, &pCallee, sizeof(Callee*), &func, sizeof(func));
	}

	template <typename Callee, typename Func>
	inline LuaObject& AssignCFunctionObjectDirect(LuaState* state, const Callee* callee, Func func, unsigned int nupvalues = 0) {
		return AssignCFunctionHelper(state, LPCD::DirectCallObjectMemberDispatcherHelper<Callee, Func, 2>::DirectCallMemberDispatcher, nupvalues, NULL, 0, &func, sizeof(func));
	}


	LuaObject operator[](const char* name) const;
	LuaObject operator[](int index) const;
	LuaObject operator[](const LuaStackObject& obj) const;
	LuaObject operator[](const LuaObject& obj) const;
	LuaObject Get(const char* name) const;
	LuaObject Get(int index) const;
	LuaObject Get(const LuaStackObject& obj) const;
	LuaObject Get(const LuaObject& obj) const;
	LuaObject GetByName(const char* name) const;
	LuaObject GetByIndex(int index) const;
	LuaObject GetByObject(const LuaStackObject& obj) const;
	LuaObject GetByObject(const LuaObject& obj) const;

	LuaObject RawGet(const char* name) const;
	LuaObject RawGet(int index) const;
	LuaObject RawGet(const LuaStackObject& obj) const;
	LuaObject RawGet(const LuaObject& obj) const;
	LuaObject RawGetByName(const char* name) const;
	LuaObject RawGetByIndex(int index) const;
	LuaObject RawGetByObject(const LuaStackObject& obj) const;
	LuaObject RawGetByObject(const LuaObject& obj) const;

	LuaObject Lookup(const char* key) const;

	template <typename T>
	LuaObject ForceGetTable(T key) const {
		LuaObject tableObj = (*this)[key];
		if (tableObj.IsTable())
			return tableObj;
		return CreateTable(key);
	}

	LuaObject& Register(const char* funcName, lua_CFunction func, int nupvalues = 0);

	LuaObject& Register(const char* funcName, int (*func)(LuaState*), int nupvalues = 0);

	template <class Callee>
	inline LuaObject& Register(const char* funcName, const Callee& callee, int (Callee::*func)(LuaState*), int nupvalues = 0) {
		const void* pCallee = &callee;
		return RegisterHelper(funcName, LPCD::LuaStateMemberDispatcherHelper<Callee>::LuaStateMemberDispatcher, nupvalues, &pCallee, sizeof(Callee*), &func, sizeof(func));
	}

	template <class Callee>
	inline LuaObject& RegisterObjectFunctor(const char* funcName, int (Callee::*func)(LuaState*), int nupvalues = 0) {
		return RegisterHelper(funcName, LPCD::Object_MemberDispatcher_to_LuaStateHelper<Callee>::Object_MemberDispatcher_to_LuaState, nupvalues, NULL, 0, &func, sizeof(func));
	}

	template <typename Func>
	inline LuaObject& RegisterDirect(const char* funcName, Func func, unsigned int nupvalues = 0) {
		return RegisterHelper(funcName, LPCD::DirectCallFunctionDispatchHelper<Func>::DirectCallFunctionDispatcher, nupvalues, NULL, 0, &func, sizeof(func));
	}

	template <typename Callee, typename Func>
	inline LuaObject& RegisterDirect(const char* funcName, const Callee& callee, Func func, unsigned int nupvalues = 0) {
		const void* pCallee = &callee;
		return RegisterHelper(funcName, LPCD::DirectCallMemberDispatcherHelper<Callee, Func>::DirectCallMemberDispatcher, nupvalues, &pCallee, sizeof(Callee*), &func, sizeof(func));
	}

	template <typename Callee, typename Func>
	inline LuaObject& RegisterObjectDirect(const char* funcName, const Callee* callee, Func func, unsigned int nupvalues = 0) {
		return RegisterHelper(funcName, LPCD::DirectCallObjectMemberDispatcherHelper<Callee, Func, 2>::DirectCallMemberDispatcher, nupvalues, NULL, 0, &func, sizeof(func));
	}

	template <typename Callee, typename Func>
	inline LuaObject& RegisterInPlaceObjectDirect(const char* funcName, const Callee* callee, Func func, unsigned int nupvalues = 0) {
		return RegisterHelper(funcName, LPCD::DirectCallInPlaceObjectMemberDispatcherHelper<Callee, Func, 2>::DirectCallMemberDispatcher, nupvalues, NULL, 0, &func, sizeof(func));
	}

	LuaObject& Unregister(const char* funcName);

	bool CheckUData(const char* tname, void** out) const;

protected:
#if !LUA_FASTREF_SUPPORT
	friend class LuaFastRefPush;
#endif // LUA_FASTREF_SUPPORT
	LuaObject& RegisterHelper(const char* funcName, lua_CFunction function, int nupvalues, const void* callee, unsigned int sizeofCallee, void* func, unsigned int sizeofFunc);
	LuaObject& AssignCFunctionHelper(LuaState* state, lua_CFunction function, int nupvalues, const void* callee, unsigned int sizeofCallee, void* func, unsigned int sizeofFunc);

private:
	lua_State* L;
	int ref;
};


} // namespace LuaPlus


namespace LPCD {
	using namespace LuaPlus;

	template<> struct Type<LuaObject> {
		static inline void Push(lua_State* L, const LuaObject& value)						{  value.Push(L);  }
		static inline bool Match(lua_State* L, int idx)										{  (void)L, (void)idx;  return true;  }
		static inline LuaPlus::LuaObject Get(lua_State* L, int idx)							{  return LuaObject(lua_State_to_LuaState(L), idx);  }
	};
	template<> struct Type<LuaObject&> : public Type<LuaObject> {};
	template<> struct Type<const LuaObject&> : public Type<LuaObject> {};

	inline LuaPlus::LuaObject PropertyMetatable_GetFunctions(const LuaPlus::LuaObject& metatableObj) {
		lua_State* L = metatableObj.GetCState();
		metatableObj.Push(L);							// metatable
		lpcd_propertymetatable_getfunctions(L, lua_gettop(L));
		LuaPlus::LuaObject retObj(L, true);
		lua_pop(L, 1);
		return retObj;
	}

	template <typename Object, typename VarType>
	inline void PropertyCreate(const LuaPlus::LuaObject& metatableObj, const char* varName, VarType Object::* var, bool read = true, bool write = true) {
		lua_State* L = metatableObj.GetCState();
		metatableObj.Push(L);							// metatable
		lpcd_propertycreate(L, lua_gettop(L), varName, var, read, write);
		lua_pop(L, 1);
	}


	inline void Metatable_IntegratePropertySupport(LuaPlus::LuaObject& metatableObj, bool inPlace = false) {
		lua_State* L = metatableObj.GetCState();
		metatableObj.Push(L);							// metatable
		lpcd_integratepropertysupport(L, lua_gettop(L), inPlace);
		lua_pop(L, 1);						//
	}


	template <typename Object, typename VarType>
	inline void Register_MemberPropertyGetFunction(LuaPlus::LuaObject& obj, const char* funcName, VarType Object::* var) {
		lua_State* L = obj.GetCState();
		obj.Push(L);
		lua_pushstring(L, funcName);
		lpcd_pushmemberpropertygetclosure(L, var);
		lua_rawset(L, -3);

		lua_pop(L, 1);
	}

	template <typename Object, typename VarType>
	void Register_MemberPropertySetFunction(LuaPlus::LuaObject& obj, const char* funcName, VarType Object::* var) {
		lua_State* L = obj.GetCState();
		obj.Push(L);
		lua_pushstring(L, funcName);
		lpcd_pushmemberpropertysetclosure(L, var);
		lua_rawset(L, -3);

		lua_pop(L, 1);
	}


	template <typename VarType>
	void Register_GlobalPropertyGetFunction(const LuaPlus::LuaObject& obj, const char* funcName, VarType* var) {
		lua_State* L = obj.GetCState();
		obj.Push(L);
		lua_pushstring(L, funcName);
		lpcd_pushglobalpropertygetclosure(L, var);
		lua_rawset(L, -3);

		lua_pop(L, 1);
	}

	template <typename VarType>
	void Register_GlobalPropertySetFunction(const LuaPlus::LuaObject& obj, const char* funcName, VarType* var) {
		lua_State* L = obj.GetCState();
		obj.Push(L);
		lua_pushstring(L, funcName);
		lpcd_pushglobalpropertysetclosure(L, var);
		lua_rawset(L, -3);

		lua_pop(L, 1);
	}
} // namespace LPCD


namespace LuaPlus {

#if !LUA_FASTREF_SUPPORT
class LuaFastRefPush
{
public:
	LuaFastRefPush(const LuaObject* self) {
		lua_getfastref(self->L, self->ref);
		this->self = self;
	}

	~LuaFastRefPush() {
		lua_pop(self->L, 1);
	}

	const LuaObject* self;
};

#define LUA_FASTREF_PUSH() LuaFastRefPush _frp(this)
#define LUA_FASTREF_REF_1 -1
#define LUA_FASTREF_REF_2 -2
#define LUA_FASTREF_REF_3 -3
#else
#define LUA_FASTREF_PUSH()
#define LUA_FASTREF_REF_1 this->ref
#define LUA_FASTREF_REF_2 this->ref
#define LUA_FASTREF_REF_3 this->ref
#endif // !LUA_FASTREF_SUPPORT

template <typename T>
inline LuaObject& LuaObject::SetNil(const T& key) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();
	LPCD::Type<T>::Push(L, key);
	lua_pushnil(L);
	lua_settable(L, LUA_FASTREF_REF_3);
	return *this;
}


template <typename KeyT, typename ValueT>
LuaObject& LuaObject::Set(const KeyT& key, const ValueT& value) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();
	LPCD::Type<KeyT>::Push(L, key);
	LPCD::Type<ValueT>::Push(L, value);
	lua_settable(L, LUA_FASTREF_REF_3);
	return *this;
}


template <typename KeyT, typename ValueT>
LuaObject& LuaObject::Set(const KeyT& key, const ValueT& value, int len) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();
	LPCD::Type<KeyT>::Push(L, key);
	LPCD::Push(L, value, len);
	lua_settable(L, LUA_FASTREF_REF_3);
	return *this;
}


template <typename KeyT>
LuaObject& LuaObject::RawSetNil(const KeyT& key) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();
	LPCD::Type<KeyT>::Push(L, key);
	lua_pushnil(L);
	lua_rawset(L, LUA_FASTREF_REF_3);
	return *this;
}


template <typename KeyT, typename ValueT>
LuaObject& LuaObject::RawSet(const KeyT& key, const ValueT& value) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();
	LPCD::Type<KeyT>::Push(L, key);
	LPCD::Type<ValueT>::Push(L, value);
	lua_rawset(L, LUA_FASTREF_REF_3);
	return *this;
}


template <typename KeyT, typename ValueT>
LuaObject& LuaObject::RawSet(const KeyT& key, const ValueT& value, int len) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();
	LPCD::Type<KeyT>::Push(L, key);
	LPCD::Push(L, value, len);
	lua_rawset(L, LUA_FASTREF_REF_3);
	return *this;
}


template <typename ValueT>
LuaObject& LuaObject::Assign(const ValueT& value) {
	luaplus_assert(L);
	lua_fastunref(L, ref);
	LPCD::Type<ValueT>::Push(L, value);
	ref = lua_fastref(L);
	return *this;
}


inline LuaObject& LuaObject::Assign(lua_State* _L, const char* value, int len) {
	luaplus_assert(_L);
	if (L)
		lua_fastunref(L, ref);
	L = _L;
	LPCD::Push(L, value, len == -1 ? (int)strlen(value) : len);
	ref = lua_fastref(L);
	return *this;
}


inline LuaObject& LuaObject::Assign(LuaState* state, const char* value, int len) {
	return Assign(LuaState_to_lua_State(state), value, len);
}


template <typename ValueT>
LuaObject& LuaObject::Assign(LuaState* state, const ValueT& value, int len) {
	if (L)
		lua_fastunref(L, ref);
	L = LuaState_to_lua_State(state);
	LPCD::Push(L, value, len);
	ref = lua_fastref(L);
	return *this;
}


template <typename ValueT>
LuaObject& LuaObject::Assign(lua_State* _L, const ValueT& value) {
	if (L)
		lua_fastunref(L, ref);
	L = _L;
	LPCD::Type<ValueT>::Push(L, value);
	ref = lua_fastref(L);
	return *this;
}


template <typename ValueT>
LuaObject& LuaObject::Assign(LuaState* state, const ValueT& value) {
	if (L)
		lua_fastunref(L, ref);
	L = LuaState_to_lua_State(state);
	LPCD::Type<ValueT>::Push(L, value);
	ref = lua_fastref(L);
	return *this;
}


template <typename ValueT>
LuaObject& LuaObject::Assign(lua_State* _L, const ValueT& value, int len) {
	if (L)
		lua_fastunref(L, ref);
	L = _L;
	LPCD::Push(L, value, len);
	ref = lua_fastref(L);
	return *this;
}


} // namespace LuaPlus

#endif // LUAPLUS__LUAOBJECT_H
