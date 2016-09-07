///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2011 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUAPLUS__LUAOBJECT_INL
#define LUAPLUS__LUAOBJECT_INL

#include <ctype.h>

#if defined(_MSC_VER)
#include <malloc.h>
#endif

///////////////////////////////////////////////////////////////////////////////
// namespace LuaPlus
///////////////////////////////////////////////////////////////////////////////
namespace LuaPlus {

inline LuaObject::LuaObject()
	: L(NULL)
	, ref(LUA_FASTREFNIL){
}


inline LuaObject::LuaObject(lua_State* inL) throw()
	: L(inL)
	, ref(LUA_FASTREFNIL)
{
}


inline LuaObject::LuaObject(LuaState* state) throw()
	: L(LuaState_to_lua_State(state))
	, ref(LUA_FASTREFNIL)
{
}


inline LuaObject::LuaObject(lua_State* inL, int stackIndex) throw()
	: L(inL)
{
	ref = lua_fastrefindex(L, stackIndex);
}


inline LuaObject::LuaObject(LuaState* state, int stackIndex) throw()
	: L(LuaState_to_lua_State(state))
{
	ref = lua_fastrefindex(L, stackIndex);
}


inline LuaObject::LuaObject(lua_State* inL, int _ref, bool directAssign) throw()
	: L(inL)
{
#if LUA_FASTREF_SUPPORT
	ref = directAssign ? _ref : lua_fastrefindex(L, _ref);
#else
	ref = directAssign ? _ref : (lua_getfastref(L, _ref), lua_fastrefindex(L, _ref));
#endif // LUA_FASTREF_SUPPORT
}


inline LuaObject::LuaObject(lua_State* inL, bool popTop) throw()
	: L(inL)
{
	ref = popTop ? lua_fastref(L) : lua_fastrefindex(L, -1);
}


inline LuaObject::LuaObject(const LuaObject& src) throw() {
	if (src.L) {
		L = src.L;
#if LUA_FASTREF_SUPPORT
		ref = lua_fastrefindex(L, src.ref);
#else
		lua_getfastref(L, src.ref);
		ref = lua_fastref(L);
#endif // LUA_FASTREF_SUPPORT
	} else {
		L = NULL;
		ref = LUA_FASTREFNIL;
	}
}


inline LuaObject::LuaObject(const LuaStackObject& src) throw() {
	if (src.L) {
		L = src.L;
		ref = lua_fastrefindex(L, src.m_stackIndex);
	} else {
		L = NULL;
		ref = LUA_FASTREFNIL;
	}
}


inline LuaObject& LuaObject::operator=(const LuaObject& src) throw() {
	if (L)
		lua_fastunref(L, ref);
	if (src.L) {
		L = src.L;
#if LUA_FASTREF_SUPPORT
		ref = lua_fastrefindex(L, src.ref);
#else
		lua_getfastref(L, src.ref);
		ref = lua_fastref(L);
#endif // LUA_FASTREF_SUPPORT
	} else {
		L = NULL;
		ref = LUA_FASTREFNIL;
	}
	return *this;
}


inline LuaObject& LuaObject::operator=(const LuaStackObject& src) throw() {
	if (L)
		lua_fastunref(L, ref);
	if (src.L) {
		L = src.L;
		ref = lua_fastrefindex(L, src.m_stackIndex);
	} else {
		L = NULL;
		ref = LUA_FASTREFNIL;
	}
	return *this;
}


inline LuaObject::~LuaObject() {
	if (L)
		lua_fastunref(L, ref);
}


/**
	Resets the LuaObject by removing itself from the used GC list and setting the state to NULL.
**/
inline void LuaObject::Reset() {
	if (L) {
		lua_fastunref(L, ref);
		L = NULL;
		ref = LUA_FASTREFNIL;
	}
}


// Mirrors lua_typename().
inline const char* LuaObject::TypeName() const {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();
	return lua_typename(L, lua_type(L, LUA_FASTREF_REF_1));
}


// Mirrors lua_type().
inline int LuaObject::Type() const {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();
	return lua_type(L, LUA_FASTREF_REF_1);
}


inline bool LuaObject::IsValid() const {
	return L != NULL;
}


// Mirrors lua_isnil().
inline bool LuaObject::IsNil() const {
	if (!L)
		return true;
	LUA_FASTREF_PUSH();
	return lua_isnil(L, LUA_FASTREF_REF_1);
}


// Mirrors lua_istable().
inline bool LuaObject::IsTable() const {
	if (!L)
		return false;
	LUA_FASTREF_PUSH();
	return lua_istable(L, LUA_FASTREF_REF_1);
}


// Mirrors lua_isuserdata().
inline bool LuaObject::IsUserdata() const {
	if (!L)
		return false;
	LUA_FASTREF_PUSH();
	return lua_isuserdata(L, LUA_FASTREF_REF_1) != 0;
}


// Mirrors lua_iscfunction().
inline bool LuaObject::IsCFunction() const {
	if (!L)
		return false;
	LUA_FASTREF_PUSH();
	return lua_iscfunction(L, LUA_FASTREF_REF_1) != 0;
}


// Behaves differently than lua_isinteger().  This function only tests for a value that is
// a real integer, not something that can be converted to a integer.
inline bool LuaObject::IsInteger() const {
	if (!L)
		return false;
	LUA_FASTREF_PUSH();
	return lua_type(L, LUA_FASTREF_REF_1) == LUA_TNUMBER;
}


// Behaves differently than lua_isnumber().  This function only tests for a value that is
// a real number, not something that can be converted to a number.
inline bool LuaObject::IsNumber() const {
	if (!L)
		return false;
	LUA_FASTREF_PUSH();
	return lua_type(L, LUA_FASTREF_REF_1) == LUA_TNUMBER;
}


// Behaves differently than lua_isstring().  This function only tests for a value that is
// a real string, not something that can be converted to a string.
inline bool LuaObject::IsString() const {
	if (!L)
		return false;
	LUA_FASTREF_PUSH();
	return lua_type(L, LUA_FASTREF_REF_1) == LUA_TSTRING;
}


// Mirrors lua_isinteger().
inline bool LuaObject::IsConvertibleToInteger() const {
	if (!L)
		return false;
	LUA_FASTREF_PUSH();
	return lua_isnumber(L, LUA_FASTREF_REF_1) != 0;
}


// Mirrors lua_isnumber().
inline bool LuaObject::IsConvertibleToNumber() const {
	if (!L)
		return false;
	LUA_FASTREF_PUSH();
	return lua_isnumber(L, LUA_FASTREF_REF_1) != 0;
}


// Mirrors lua_isstring().
inline bool LuaObject::IsConvertibleToString() const {
	if (!L)
		return false;
	LUA_FASTREF_PUSH();
	return lua_isstring(L, LUA_FASTREF_REF_1) != 0;
}


// Mirrors lua_isfunction().
inline bool LuaObject::IsFunction() const {
	if (!L)
		return false;
	LUA_FASTREF_PUSH();
	return lua_isfunction(L, LUA_FASTREF_REF_1);
}


// Mirrors lua_isnone().
inline bool LuaObject::IsNone() const {
	if (!L)
		return false;
	LUA_FASTREF_PUSH();
	return lua_isnone(L, LUA_FASTREF_REF_1);
}


// Mirrors lua_islightuserdata().
inline bool LuaObject::IsLightUserdata() const {
	if (!L)
		return false;
	LUA_FASTREF_PUSH();
	return lua_islightuserdata(L, LUA_FASTREF_REF_1);
}


// Mirrors lua_isboolean().
inline bool LuaObject::IsBoolean() const {
	if (!L)
		return false;
	LUA_FASTREF_PUSH();
	return lua_isboolean(L, LUA_FASTREF_REF_1);
}


// Mirrors lua_tointeger()
inline lua_Integer LuaObject::ToInteger() {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();
#if LUA_FASTREF_SUPPORT
	return lua_tointeger(L, ref);
#else
	lua_getfastref(L, ref);
	lua_Integer ret = lua_tointeger(L, -1);
	lua_fastunref(L, ref);
	ref = lua_fastref(L);
	return ret;
#endif // LUA_FASTREF_SUPPORT
}


// Mirrors lua_tonumber()
inline lua_Number LuaObject::ToNumber() {
	luaplus_assert(L);
#if LUA_FASTREF_SUPPORT
	return lua_tonumber(L, ref);
#else
	lua_getfastref(L, ref);
	lua_Number ret = lua_tonumber(L, -1);
	lua_fastunref(L, ref);
	ref = lua_fastref(L);
	return ret;
#endif // LUA_FASTREF_SUPPORT
}


// Mirrors lua_tostring().
inline const char* LuaObject::ToString() {
	luaplus_assert(L);
#if LUA_FASTREF_SUPPORT
	return lua_tostring(L, ref);
#else
	lua_getfastref(L, ref);
	const char* ret = lua_tostring(L, -1);
	lua_fastunref(L, ref);
	ref = lua_fastref(L);
	return ret;
#endif // LUA_FASTREF_SUPPORT
}


inline size_t LuaObject::ObjLen() {
	luaplus_assert(L);
#if !LUA_FASTREF_SUPPORT
	if (Type() == LUA_TNUMBER  /*  ||  Type() == LUA_TINT */)
	{
		ToString();
	}
#endif // !LUA_FASTREF_SUPPORT
	LUA_FASTREF_PUSH();
#if LUA_VERSION_NUM == 501
	return lua_objlen(L, LUA_FASTREF_REF_1);
#elif LUA_VERSION_NUM >= 502
	return lua_rawlen(L, LUA_FASTREF_REF_1);
#endif
}


inline lua_Integer LuaObject::GetInteger() const {
	luaplus_assert(L  &&  IsInteger());
	LUA_FASTREF_PUSH();
	return lua_tointeger(L, LUA_FASTREF_REF_1);
}


inline float LuaObject::GetFloat() const {
	luaplus_assert(L  &&  IsNumber());
	LUA_FASTREF_PUSH();
	return (float)lua_tonumber(L, LUA_FASTREF_REF_1);
}


inline double LuaObject::GetDouble() const {
	luaplus_assert(L  &&  IsNumber());
	LUA_FASTREF_PUSH();
	return (double)lua_tonumber(L, LUA_FASTREF_REF_1);
}


inline lua_Number LuaObject::GetNumber() const {
	luaplus_assert(L  &&  IsNumber());
	LUA_FASTREF_PUSH();
	return (lua_Number)lua_tonumber(L, LUA_FASTREF_REF_1);
}


inline const char* LuaObject::GetString() const {
	luaplus_assert(L  &&  IsString());
	LUA_FASTREF_PUSH();
	return lua_tostring(L, LUA_FASTREF_REF_1);
}


inline size_t LuaObject::StrLen() const {
	luaplus_assert(L);
	luaplus_assert(IsString());
	LUA_FASTREF_PUSH();
#if LUA_VERSION_NUM == 501
	return lua_objlen(L, LUA_FASTREF_REF_1);
#elif LUA_VERSION_NUM >= 502
	return lua_rawlen(L, LUA_FASTREF_REF_1);
#endif
}


inline lua_CFunction LuaObject::GetCFunction() const {
	luaplus_assert(L  &&  IsCFunction());
	LUA_FASTREF_PUSH();
	return lua_tocfunction(L, LUA_FASTREF_REF_1);
}


// Mirrors lua_touserdata().
inline void* LuaObject::GetUserdata() const {
	luaplus_assert(L  &&  IsUserdata());
	LUA_FASTREF_PUSH();
	return lua_touserdata(L, LUA_FASTREF_REF_1);
}


// Mirrors lua_topointer.
inline const void* LuaObject::GetLuaPointer() const {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();
	return lua_topointer(L, LUA_FASTREF_REF_1);
}


// No equivalent.
inline void* LuaObject::GetLightUserdata() const {
	luaplus_assert(L  &&  IsLightUserdata());
	LUA_FASTREF_PUSH();
	return lua_touserdata(L, LUA_FASTREF_REF_1);
}


// Mirrors lua_toboolean().
inline bool LuaObject::GetBoolean() const {
	luaplus_assert(L  &&  IsBoolean()  ||  IsNil());
	LUA_FASTREF_PUSH();
	return lua_toboolean(L, LUA_FASTREF_REF_1) != 0;
}


inline LuaStackObject LuaObject::Push(lua_State* outL) const {
	luaplus_assert(outL);
	lua_getfastref(outL, ref);
	return LuaStackObject(outL, lua_gettop(outL));
}


inline LuaStackObject LuaObject::Push(LuaState* state) const {
	luaplus_assert(state);
	lua_State* outL = state->GetCState();
	luaplus_assert(outL);
	lua_getfastref(outL, ref);
	return LuaStackObject(outL, lua_gettop(outL));
}


inline LuaObject LuaObject::GetMetatable() const {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();
	if (lua_getmetatable(L, LUA_FASTREF_REF_1))
		return LuaObject(L, lua_fastref(L), true);
	return LuaObject(L);
}


inline LuaObject& LuaObject::SetMetatable(const LuaObject& valueObj) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();
	lua_getfastref(L, valueObj.ref);
	lua_setmetatable(L, LUA_FASTREF_REF_2);
	return *this;
}


inline void LuaObject::Insert(LuaObject& obj) {
	luaplus_assert(L);
	luaplus_assert(L == obj.L);
	lua_getglobal(L, "table");
	luaplus_assert(lua_istable(L, -1));
	lua_getfield(L, -1, "insert");
	luaplus_assert(lua_isfunction(L, -1));
	Push(L);
	obj.Push(L);
	if (lua_pcall(L, 2, 0, 0) != 0) {
		luaplus_assert(0);
	}
	lua_pop(L, 1);
}


inline void LuaObject::Insert(int index, LuaObject& obj) {
	luaplus_assert(L);
	luaplus_assert(L == obj.L);
	lua_getglobal(L, "table");
	luaplus_assert(lua_istable(L, -1));
	lua_getfield(L, -1, "insert");
	luaplus_assert(lua_isfunction(L, -1));
	Push(L);
	lua_pushinteger(L, index);
	obj.Push(L);
	if (lua_pcall(L, 3, 0, 0) != 0) {
		luaplus_assert(0);
	}
	lua_pop(L, 1);
}


inline void LuaObject::Remove(int index) {
	luaplus_assert(L);
	lua_getglobal(L, "table");
	luaplus_assert(lua_istable(L, -1));
	lua_getfield(L, -1, "remove");
	luaplus_assert(lua_isfunction(L, -1));
	Push(L);
	lua_pushinteger(L, index);
	if (lua_pcall(L, 2, 0, 0) != 0) {
		luaplus_assert(0);
	}
	lua_pop(L, 1);
}


inline void LuaObject::Sort() {
	luaplus_assert(L);
	lua_getglobal(L, "table");
	luaplus_assert(lua_istable(L, -1));
	lua_getfield(L, -1, "sort");
	luaplus_assert(lua_isfunction(L, -1));
	Push(L);
	lua_call(L, 1, 0);
	lua_pop(L, 1);
}


inline size_t LuaObject::GetCount() const {
	luaplus_assert(L);
	Push(L);
#if LUA_VERSION_NUM == 501
	size_t count = lua_objlen(L, -1);
#elif LUA_VERSION_NUM >= 502
	size_t count = lua_rawlen(L, -1);
#endif
	lua_pop(L, 1);
	return count;
}


inline size_t LuaObject::GetTableCount() const {
	luaplus_assert(L);
	size_t count = 0;
	for (LuaTableIterator it(*this); it; ++it) {
		count++;
	}
	return count;
}


/**
	Creates a table called [name] within the current LuaObject.

	@param key The name of the table to create.
	@param size The size of the table.
	@return Returns the object representing the newly created table.
**/
inline LuaObject LuaObject::CreateTable(const char* key, int narray, int nrec) const {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_pushstring(L, key);				// (table) key
	lua_pushvalue(L, -1);				// (table) key key
	lua_createtable(L, narray, nrec);	// (table) key key table
#if LUA_FASTREF_SUPPORT
	lua_settable(L, ref);				// key
	lua_gettable(L, ref);				// table
#else
	lua_settable(L, -4);				// (table) key
	lua_gettable(L, -2);				// (table) table
#endif // LUA_FASTREF_SUPPORT
	return LuaObject(L, lua_fastref(L), true);
}


/**
	Creates a table called [key] within the current LuaStackObject.

	@param index The index of the table to create.
	@param size The size of the table.
	@return Returns the object representing the newly created table.
**/
inline LuaObject LuaObject::CreateTable(int key, int narray, int nrec) const {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_pushinteger(L, key);			// (table) key
	lua_pushvalue(L, -1);				// (table) key key
	lua_createtable(L, narray, nrec);	// (table) key key table
#if LUA_FASTREF_SUPPORT
	lua_settable(L, ref);				// key
	lua_gettable(L, ref);				// table
#else
	lua_settable(L, -4);				// (table) key
	lua_gettable(L, -2);				// (table) table
#endif // LUA_FASTREF_SUPPORT
	return LuaObject(L, lua_fastref(L), true);
}


/**
	Creates a table called [key] within the current LuaStackObject.

	@param index The index of the table to create.
	@param size The size of the table.
	@return Returns the object representing the newly created table.
**/
inline LuaObject LuaObject::CreateTable(const LuaObject& key, int narray, int nrec) const {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_getfastref(L, key.ref);			// (table) key
	lua_pushvalue(L, -1);				// (table) key key
	lua_createtable(L, narray, nrec);	// (table) key key table
#if LUA_FASTREF_SUPPORT
	lua_settable(L, ref);				// key
	lua_gettable(L, ref);				// table
#else
	lua_settable(L, -4);				// (table) key
	lua_gettable(L, -2);				// (table) table
#endif // LUA_FASTREF_SUPPORT
	return LuaObject(L, lua_fastref(L), true);
}


inline LuaObject LuaObject::operator[](const char* key) const {
	return Get(key);
}


inline LuaObject LuaObject::operator[](int key) const {
	return Get(key);
}


inline LuaObject LuaObject::operator[](const LuaObject& key) const {
	return Get(key);
}


inline LuaObject LuaObject::operator[](const LuaStackObject& key) const {
	return Get(key);
}


inline LuaObject LuaObject::Get(const char* key) const {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_pushstring(L, key);				// (table) key
	lua_gettable(L, LUA_FASTREF_REF_2);	// (table) value
	return LuaObject(L, lua_fastref(L), true);
}


inline LuaObject LuaObject::Get(int key) const {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_pushinteger(L, key);			// (table) key
	lua_gettable(L, LUA_FASTREF_REF_2);	// (table) value
	return LuaObject(L, lua_fastref(L), true);
}

inline LuaObject LuaObject::Get(const LuaObject& key) const {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_getfastref(L, key.ref);			// (table) key
	lua_gettable(L, LUA_FASTREF_REF_2);	// (table) value
	return LuaObject(L, lua_fastref(L), true);
}


inline LuaObject LuaObject::Get(const LuaStackObject& key) const {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_pushvalue(L, key.m_stackIndex);	// (table) key
	lua_gettable(L, LUA_FASTREF_REF_2);	// (table) table
	return LuaObject(L, lua_fastref(L), true);
}


inline LuaObject LuaObject::GetByName(const char* key) const {
	return Get(key);
}


inline LuaObject LuaObject::GetByIndex(int key) const {
	return Get(key);
}


inline LuaObject LuaObject::GetByObject(const LuaObject& key) const {
	return Get(key);
}


inline LuaObject LuaObject::GetByObject(const LuaStackObject& key) const {
	return Get(key);
}


inline LuaObject LuaObject::RawGet(const char* key) const {
	return RawGetByName(key);
}


inline LuaObject LuaObject::RawGet(int key) const {
	return RawGetByIndex(key);
}

inline LuaObject LuaObject::RawGet(const LuaObject& key) const {
	return RawGetByObject(key);
}


inline LuaObject LuaObject::RawGet(const LuaStackObject& key) const {
	return RawGetByObject(key);
}


inline LuaObject LuaObject::RawGetByName(const char* key) const {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_pushstring(L, key);				// (table) key
	lua_rawget(L, LUA_FASTREF_REF_2);	// (table) value
	return LuaObject(L, lua_fastref(L), true);
}

inline LuaObject LuaObject::RawGetByIndex(int key) const {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_rawgeti(L, LUA_FASTREF_REF_1, key);		// (table) value
	return LuaObject(L, lua_fastref(L), true);
}

inline LuaObject LuaObject::RawGetByObject(const LuaObject& key) const {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_getfastref(L, key.ref);			// (table) key
	lua_rawget(L, LUA_FASTREF_REF_2);	// (table) value
	return LuaObject(L, lua_fastref(L), true);
}


inline LuaObject LuaObject::RawGetByObject(const LuaStackObject& key) const {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_pushvalue(L, key.m_stackIndex);	// (table) key
	lua_rawget(L, LUA_FASTREF_REF_2);	// (table) value
	return LuaObject(L, lua_fastref(L), true);
}


inline LuaObject& LuaObject::SetBoolean(const char* key, bool value) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetBoolean(int key, bool value) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetBoolean(LuaObject& key, bool value) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetInteger(const char* key, lua_Integer value) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetInteger(int key, lua_Integer value) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetInteger(LuaObject& key, lua_Integer value) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetNumber(const char* key, lua_Number value) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetNumber(int key, lua_Number value) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetNumber(LuaObject& key, lua_Number value) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetString(const char* key, const char* value, int len) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetString(int key, const char* value, int len) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetString(LuaObject& key, const char* value, int len) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetUserdata(const char* key, void* value) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetUserdata(int key, void* value) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetUserdata(LuaObject& key, void* value) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetLightUserdata(const char* key, void* value) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetLightUserdata(int key, void* value) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetLightUserdata(LuaObject& key, void* value) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetObject(const char* key, const LuaObject& value) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetObject(int key, const LuaObject& value) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetObject(LuaObject& key, const LuaObject& value) {
	return Set(key, value);
}


inline LuaObject& LuaObject::RawSetBoolean(const char* key, bool value) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_pushstring(L, key);
	lua_pushboolean(L, value);
	lua_rawset(L, LUA_FASTREF_REF_3);
	return *this;
}


inline LuaObject& LuaObject::RawSetBoolean(int key, bool value) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_pushboolean(L, value);
	lua_rawseti(L, LUA_FASTREF_REF_2, key);
	return *this;
}


inline LuaObject& LuaObject::RawSetBoolean(LuaObject& key, bool value) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_getfastref(L, key.ref);
	lua_pushboolean(L, value);
	lua_rawset(L, LUA_FASTREF_REF_3);
	return *this;
}


inline LuaObject& LuaObject::RawSetInteger(const char* key, lua_Integer value) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_pushstring(L, key);
	lua_pushinteger(L, value);
	lua_rawset(L, LUA_FASTREF_REF_3);
	return *this;
}


inline LuaObject& LuaObject::RawSetInteger(int key, lua_Integer value) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_pushinteger(L, value);
	lua_rawseti(L, LUA_FASTREF_REF_2, key);
	return *this;
}


inline LuaObject& LuaObject::RawSetInteger(LuaObject& key, lua_Integer value) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_getfastref(L, key.ref);
	lua_pushinteger(L, value);
	lua_rawset(L, LUA_FASTREF_REF_3);
	return *this;
}


inline LuaObject& LuaObject::RawSetNumber(const char* key, lua_Number value) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_pushstring(L, key);
	lua_pushnumber(L, value);
	lua_rawset(L, LUA_FASTREF_REF_3);
	return *this;
}


inline LuaObject& LuaObject::RawSetNumber(int key, lua_Number value) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_pushnumber(L, value);
	lua_rawseti(L, LUA_FASTREF_REF_2, key);
	return *this;
}


inline LuaObject& LuaObject::RawSetNumber(LuaObject& key, lua_Number value) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_getfastref(L, key.ref);
	lua_pushnumber(L, value);
	lua_rawset(L, LUA_FASTREF_REF_3);
	return *this;
}


inline LuaObject& LuaObject::RawSetString(const char* key, const char* value, int len) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_pushstring(L, key);
	lua_pushlstring(L, value, len == -1 ? strlen(value) : len);
	lua_rawset(L, LUA_FASTREF_REF_3);
	return *this;
}


inline LuaObject& LuaObject::RawSetString(int key, const char* value, int len) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_pushlstring(L, value, len == -1 ? strlen(value) : len);
	lua_rawseti(L, LUA_FASTREF_REF_2, key);
	return *this;
}


inline LuaObject& LuaObject::RawSetString(LuaObject& key, const char* value, int len) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_getfastref(L, key.ref);
	lua_pushlstring(L, value, len == -1 ? strlen(value) : len);
	lua_rawset(L, LUA_FASTREF_REF_3);
	return *this;
}


inline LuaObject& LuaObject::RawSetUserdata(const char* key, void* value) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_pushstring(L, key);
	lua_pushlightuserdata(L, value);
	lua_rawset(L, LUA_FASTREF_REF_3);
	return *this;
}


inline LuaObject& LuaObject::RawSetUserdata(int key, void* value) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_pushlightuserdata(L, value);
	lua_rawseti(L, LUA_FASTREF_REF_2, key);
	return *this;
}


inline LuaObject& LuaObject::RawSetUserdata(LuaObject& key, void* value) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_getfastref(L, key.ref);
	lua_pushlightuserdata(L, value);
	lua_rawset(L, LUA_FASTREF_REF_3);
	return *this;
}


inline LuaObject& LuaObject::RawSetLightUserdata(const char* key, void* value) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_pushstring(L, key);
	lua_pushlightuserdata(L, value);
	lua_rawset(L, LUA_FASTREF_REF_3);
	return *this;
}


inline LuaObject& LuaObject::RawSetLightUserdata(int key, void* value) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_pushlightuserdata(L, value);
	lua_rawseti(L, LUA_FASTREF_REF_2, key);
	return *this;
}


inline LuaObject& LuaObject::RawSetLightUserdata(LuaObject& key, void* value) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_getfastref(L, key.ref);
	lua_pushlightuserdata(L, value);
	lua_rawset(L, LUA_FASTREF_REF_3);
	return *this;
}


inline LuaObject& LuaObject::RawSetObject(const char* key, LuaObject& value) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_pushstring(L, key);
	lua_pushvalue(L, value.GetRef());
	lua_rawset(L, LUA_FASTREF_REF_3);
	return *this;
}


inline LuaObject& LuaObject::RawSetObject(int key, LuaObject& value) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_pushvalue(L, value.GetRef());
	lua_rawseti(L, LUA_FASTREF_REF_2, key);
	return *this;
}


inline LuaObject& LuaObject::RawSetObject(LuaObject& key, LuaObject& value) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_getfastref(L, key.ref);
	lua_getfastref(L, value.ref);
	lua_rawset(L, LUA_FASTREF_REF_3);
	return *this;
}


inline LuaObject& LuaObject::AssignNil() {
	luaplus_assert(L);
	lua_fastunref(L, ref);
	lua_pushnil(L);
	ref = lua_fastref(L);
	return *this;
}


inline LuaObject& LuaObject::AssignNil(lua_State* inL) {
	if (L)
		lua_fastunref(L, ref);
	L = inL;
	lua_pushnil(L);
	ref = lua_fastref(L);
	return *this;
}


inline LuaObject& LuaObject::AssignNil(LuaState* state) {
	if (L)
		lua_fastunref(L, ref);
	L = LuaState_to_lua_State(state);
	lua_pushnil(L);
	ref = lua_fastref(L);
	return *this;
}


inline LuaObject& LuaObject::AssignBoolean(LuaState* state, bool value) {
	return Assign(state, value);
}


inline LuaObject& LuaObject::AssignInteger(LuaState* state, lua_Integer value) {
	return Assign(state, value);
}


inline LuaObject& LuaObject::AssignNumber(LuaState* state, lua_Number value) {
	return Assign(state, value);
}


inline LuaObject& LuaObject::AssignString(LuaState* state, const char* value, int len) {
	return Assign(state, value, len);
}


inline LuaObject& LuaObject::AssignUserdata(LuaState* state, void* value) {
	lua_State* inL = LuaState_to_lua_State(state);
	luaplus_assert(inL);
	if (L)
		lua_fastunref(L, ref);
	L = inL;
	LPCD::Type<LPCD::LuaUserdata>::Push(L, LPCD::LuaUserdata(value));
	ref = lua_fastref(L);
	return *this;
}


inline LuaObject& LuaObject::AssignUserdata(LuaState* state, size_t size) {
	lua_State* inL = LuaState_to_lua_State(state);
	luaplus_assert(inL);
	if (L)
		lua_fastunref(L, ref);
	L = inL;
	lua_newuserdata(L, size);
	ref = lua_fastref(L);
	return *this;
}


inline LuaObject& LuaObject::AssignLightUserdata(LuaState* state, void* value) {
	return Assign(state, value);
}


inline LuaObject& LuaObject::AssignObject(LuaObject& value) {
	*this = value;
	return *this;
}


inline LuaObject& LuaObject::AssignNewTable(int narray, int nrec) {
	luaplus_assert(L);
	lua_fastunref(L, ref);
	lua_createtable(L, narray, nrec);
	ref = lua_fastref(L);
	return *this;
}


inline LuaObject& LuaObject::AssignNewTable(lua_State* inL, int narray, int nrec) {
	if (L)
		lua_fastunref(L, ref);
	L = inL;
	lua_createtable(L, narray, nrec);
	ref = lua_fastref(L);
	return *this;
}


inline LuaObject& LuaObject::AssignNewTable(LuaState* state, int narray, int nrec) {
	if (L)
		lua_fastunref(L, ref);
	L = LuaState_to_lua_State(state);
	lua_createtable(L, narray, nrec);
	ref = lua_fastref(L);
	return *this;
}


/**
	Assuming the current object is a table, registers a C function called
	[funcName] within the table.

	@param funcName The name of the function to register.
	@param function A pointer to the C function to register.
**/
inline LuaObject& LuaObject::Register(const char* funcName, lua_CFunction function, int nupvalues) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_pushcclosure(L, function, nupvalues);
	lua_setfield(L, LUA_FASTREF_REF_2, funcName);
	return *this;
}


inline LuaObject& LuaObject::Register(const char* funcName, int (*function)(LuaState*), int nupvalues) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
	lua_pushcclosure(L, (lua_CFunction)function, nupvalues);
	lua_setfield(L, LUA_FASTREF_REF_2, funcName);
	return *this;
}


inline LuaObject& LuaObject::RegisterHelper(const char* funcName, lua_CFunction function, int nupvalues, const void* callee, unsigned int sizeofCallee, void* func, unsigned int sizeofFunc) {
	luaplus_assert(L);
	LUA_FASTREF_PUSH();					// (table)
#if !LUA_FASTREF_SUPPORT
	int top = lua_gettop(L);
#endif // !LUA_FASTREF_SUPPORT

	lua_pushstring(L, funcName);

	if (sizeofFunc != 0)
	{
		unsigned char* buffer = (unsigned char*)lua_newuserdata(L, sizeofCallee + sizeofFunc);
		unsigned int pos = 0;
		if (sizeofCallee > 0)
		{
			memcpy(buffer, callee, sizeofCallee);
			pos += sizeofCallee;
		}

		memcpy(buffer + pos, func, sizeofFunc);

		lua_insert(L, -1 - (int)nupvalues);
		nupvalues++;
	}

	lua_pushcclosure(L, (lua_CFunction)function, nupvalues);
#if LUA_FASTREF_SUPPORT
	lua_rawset(L, ref);
#else
	lua_rawset(L, top);
#endif // LUA_FASTREF_SUPPORT

	return *this;
}


/**
	Assuming the current object is a table, unregisters the function called
	[funcName].

	@param funcName The name of the function to unregister.
**/
inline LuaObject& LuaObject::Unregister(const char* funcName)
{
	luaplus_assert(L);
	return SetNil(funcName);
}


/**
**/
inline bool LuaObject::operator==(const LuaObject& right) const
{
	luaplus_assert(L);
	return lua_State_to_LuaState(L)->Equal(*this, right) != 0;
}

	
/**
**/
inline bool LuaObject::operator<(const LuaObject& right) const
{
	luaplus_assert(L);
	return lua_State_to_LuaState(L)->LessThan(*this, right) != 0;
}

	
/**
**/
inline LuaState* LuaObject::GetState() const
{
	return lua_State_to_LuaState(L);
}


inline lua_State* LuaObject::GetCState() const
{
	return L;
}

inline int LuaObject::GetRef() const {
	return ref;
}


/**
**/
inline LuaObject LuaObject::Clone() const {
	if (IsTable()) {
		LuaObject tableObj(L);
//		sethvalue(L, &tableObj.m_object, luaH_new(L, hvalue(&m_object)->sizearray, hvalue(&m_object)->lsizenode));
		tableObj.AssignNewTable(lua_State_to_LuaState(L), 0, 0);
		tableObj.SetMetatable(GetMetatable());

		for (LuaTableIterator it(*this); it; ++it) {
			if (it.GetValue().IsTable()) {
				LuaObject clonedChildTableObj = it.GetValue().Clone();
				tableObj.Set(it.GetKey(), clonedChildTableObj);
			}
			else
				tableObj.Set(it.GetKey(), it.GetValue());
		}

		return tableObj;
	}

	return LuaObject(L, ref);
}


inline void LuaObject::DeepClone(LuaObject& outObj) const {
	if (IsTable())
	{
		outObj.AssignNewTable(outObj.GetState());

		for (LuaTableIterator it(*this); it; ++it)
		{
			LuaObject keyObj;
			switch (it.GetKey().Type())
			{
				case LUA_TBOOLEAN:	keyObj.AssignBoolean(outObj.GetState(), it.GetKey().GetBoolean());		break;
				case LUA_TNUMBER:		keyObj.AssignNumber(outObj.GetState(), it.GetKey().GetNumber());	break;
				case LUA_TSTRING:		keyObj.AssignString(outObj.GetState(), it.GetKey().GetString());	break;
			}

			switch (it.GetValue().Type())
			{
				case LUA_TBOOLEAN:	outObj.SetBoolean(keyObj, it.GetValue().GetBoolean());		break;
				case LUA_TNUMBER:		outObj.SetNumber(keyObj, it.GetValue().GetNumber());	break;
				case LUA_TSTRING:		outObj.SetString(keyObj, it.GetValue().GetString());	break;
				case LUA_TTABLE:
				{
					LuaObject newTableObj(outObj.GetState());
					it.GetValue().DeepClone(newTableObj);
					outObj.SetObject(keyObj, newTableObj);
					break;
				}
			}
		}
	}
	else
	{
		switch (Type())
		{
			case LUA_TBOOLEAN:	outObj.AssignBoolean(outObj.GetState(), this->GetBoolean());		break;
			case LUA_TNUMBER:	outObj.AssignNumber(outObj.GetState(), this->GetNumber());			break;
			case LUA_TSTRING:	outObj.AssignString(outObj.GetState(), this->GetString());			break;
			case LUA_TTABLE:	DeepClone(outObj);													break;
		}
	}
}


inline int LuaObject_str2d (const char *s, lua_Number *result) {
  char *endptr;
  *result = lua_str2number(s, &endptr);
  if (endptr == s) return 0;  /* conversion failed */
  if (*endptr == 'x' || *endptr == 'X')  /* maybe an hexadecimal constant? */
    *result = (lua_Number)(strtoul(s, &endptr, 16));
  if (*endptr == '\0') return 1;  /* most common case */
  while (isspace((unsigned char)*endptr)) endptr++;
  if (*endptr != '\0') return 0;  /* invalid trailing characters? */
  return 1;
}


inline LuaObject LuaObject::Lookup(const char* key) const {
	LuaObject table = *this;

	size_t keyLen = strlen(key);
#if defined(_MSC_VER)
 	char* buf = (char*)_alloca(keyLen + 1);
#else // !_MSC_VER
	char* buf = new char[keyLen + 1];
#endif // _MSC_VER
	strncpy(buf, key, keyLen);
	buf[keyLen] = 0;

	char* lastPos = (char*)buf;

	while (true) {
		lua_Number num;

		char* curPos = strchr(lastPos, '.');
		if (!curPos) {
			if (LuaObject_str2d(lastPos, &num)) {
#if !defined(_MSC_VER)
                delete [] buf;
#endif
				return table[(int)num];
			}

			LuaObject obj = table[(const char*)lastPos];
#if !defined(_MSC_VER)
            delete [] buf;
#endif
			return obj;
		}

		*curPos = 0;
		if (LuaObject_str2d(lastPos, &num)) {
			table = table[(int)num];
		} else {
			table = table[(const char*)lastPos];
		}

		if (table.IsNil()) {
#if !defined(_MSC_VER)
            delete [] buf;
#endif
			return table;
        }

		lastPos = curPos + 1;
	}

#if !defined(_MSC_VER)
    delete [] buf;
#endif
	return LuaObject(L);
}


/**
**/
inline LuaObject& LuaObject::AssignCFunction(LuaState* state, lua_CFunction function, int nupvalues) {
	return AssignCFunctionHelper(state, function, nupvalues, NULL, 0, NULL, 0);
}


/**
**/
inline LuaObject& LuaObject::AssignCFunction(LuaState* state, int (*func)(LuaState*), int nupvalues) {
	return AssignCFunctionHelper(state, LPCD::LuaStateFunctionDispatcher, nupvalues, NULL, 0, &func, sizeof(func));
}


/**
**/
inline LuaObject& LuaObject::AssignCFunctionHelper(LuaState* state, lua_CFunction function, int nupvalues, const void* callee, unsigned int sizeofCallee, void* func, unsigned int sizeofFunc) {
	if (L)
		lua_fastunref(L, ref);

	L = LuaState_to_lua_State(state);

	if (sizeofFunc != 0)
	{
		unsigned char* buffer = (unsigned char*)lua_newuserdata(L, sizeofCallee + sizeofFunc);
		unsigned int pos = 0;
		if (sizeofCallee > 0)
		{
			memcpy(buffer, callee, sizeofCallee);
			pos += sizeofCallee;
		}

		memcpy(buffer + pos, func, sizeofFunc);

		nupvalues++;
	}

	lua_pushcclosure(L, (lua_CFunction)function, nupvalues);
	ref = lua_fastref(L);

	return *this;
}


/**
**/
inline bool LuaObject::CheckUData(const char* tname, void** out) const {
	LUA_FASTREF_PUSH();
	void *p = lua_touserdata(L, LUA_FASTREF_REF_1);
	if (p != NULL) {  /* value is a userdata? */
		if (lua_getmetatable(L, LUA_FASTREF_REF_1)) {  /* does it have a metatable? */
			lua_getfield(L, LUA_REGISTRYINDEX, tname);  /* get correct metatable */
			if (lua_rawequal(L, -1, -2)) {  /* does it have the correct mt? */
				lua_pop(L, 2);  /* remove both metatables */
				*out = p;
				return true;
			}
		}
	}
	*out = NULL;
	return false;
}


} // namespace LuaPlus

#endif // LUAPLUS__LUAOBJECT_INL
