///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2010 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUAOBJECT_INL
#define LUAOBJECT_INL

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


inline LuaObject::LuaObject(lua_State* _L) throw()
	: L(_L)
	, ref(LUA_FASTREFNIL)
{
}


inline LuaObject::LuaObject(LuaState* state) throw()
	: L(LuaState_to_lua_State(state))
	, ref(LUA_FASTREFNIL)
{
}


inline LuaObject::LuaObject(lua_State* _L, int stackIndex) throw()
	: L(_L)
{
	ref = lua_fastrefindex(L, stackIndex);
}


inline LuaObject::LuaObject(LuaState* state, int stackIndex) throw()
	: L(LuaState_to_lua_State(state))
{
	ref = lua_fastrefindex(L, stackIndex);
}


inline LuaObject::LuaObject(lua_State* _L, int _ref, bool directAssign) throw()
	: L(_L)
{
	ref = directAssign ? _ref : lua_fastrefindex(L, _ref);
}


inline LuaObject::LuaObject(lua_State* _L, bool popTop) throw()
	: L(_L)
{
	ref = popTop ? lua_fastref(L) : lua_fastrefindex(L, -1);
}


inline LuaObject::LuaObject(const LuaObject& src) throw() {
	if (src.L) {
		L = src.L;
		ref = lua_fastrefindex(L, src.ref);
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
		ref = lua_fastrefindex(L, src.ref);
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
	return lua_typename(L, lua_type(L, ref));
}


// Mirrors lua_type().
inline int LuaObject::Type() const {
	luaplus_assert(L);
	return lua_type(L, ref);
}


// Mirrors lua_isnil().
inline bool LuaObject::IsNil() const {
	luaplus_assert(L);
	return lua_isnil(L, ref);
}


// Mirrors lua_istable().
inline bool LuaObject::IsTable() const {
	luaplus_assert(L);
	return lua_istable(L, ref);
}


// Mirrors lua_isuserdata().
inline bool LuaObject::IsUserData() const {
	luaplus_assert(L);
	return lua_isuserdata(L, ref) != 0;
}


// Mirrors lua_iscfunction().
inline bool LuaObject::IsCFunction() const {
	luaplus_assert(L);
	return lua_iscfunction(L, ref) != 0;
}


// Behaves differently than lua_isinteger().  This function only tests for a value that is
// a real integer, not something that can be converted to a integer.
inline bool LuaObject::IsInteger() const {
	luaplus_assert(L);
	return lua_type(L, ref) == LUA_TNUMBER;
}


// Behaves differently than lua_isnumber().  This function only tests for a value that is
// a real number, not something that can be converted to a number.
inline bool LuaObject::IsNumber() const {
	luaplus_assert(L);
	return lua_type(L, ref) == LUA_TNUMBER;
}


// Behaves differently than lua_isstring().  This function only tests for a value that is
// a real string, not something that can be converted to a string.
inline bool LuaObject::IsString() const {
	luaplus_assert(L);
	return lua_type(L, ref) == LUA_TSTRING;
}


#if LUA_WIDESTRING
inline bool LuaObject::IsWString() const {
	luaplus_assert(L);
	return lua_type(L, ref) == LUA_TWSTRING;
}
#endif /* LUA_WIDESTRING */


// Mirrors lua_isinteger().
inline bool LuaObject::IsConvertibleToInteger() const {
	luaplus_assert(L);
	return lua_isnumber(L, ref) != 0;
}


// Mirrors lua_isnumber().
inline bool LuaObject::IsConvertibleToNumber() const {
	luaplus_assert(L);
	return lua_isnumber(L, ref) != 0;
}


// Mirrors lua_isstring().
inline bool LuaObject::IsConvertibleToString() const {
	luaplus_assert(L);
	return lua_isstring(L, ref) != 0;
}


// Mirrors lua_iswstring().
#if LUA_WIDESTRING
inline bool LuaObject::IsConvertibleToWString() const {
	luaplus_assert(L);
	return lua_iswstring(L, ref) != 0;
}
#endif /* LUA_WIDESTRING */


// Mirrors lua_isfunction().
inline bool LuaObject::IsFunction() const {
	luaplus_assert(L);
	return lua_isfunction(L, ref);
}


// Mirrors lua_isnone().
inline bool LuaObject::IsNone() const {
	luaplus_assert(L);
	return lua_isnone(L, ref);
}


// Mirrors lua_islightuserdata().
inline bool LuaObject::IsLightUserData() const {
	luaplus_assert(L);
	return lua_islightuserdata(L, ref);
}


// Mirrors lua_isboolean().
inline bool LuaObject::IsBoolean() const {
	luaplus_assert(L);
	return lua_isboolean(L, ref);
}


// Mirrors lua_tointeger()
inline int LuaObject::ToInteger() {
	luaplus_assert(L);
	return lua_tointeger(L, ref);
}


// Mirrors lua_tonumber()
inline lua_Number LuaObject::ToNumber() {
	luaplus_assert(L);
	return lua_tonumber(L, ref);
}


// Mirrors lua_tostring().
inline const char* LuaObject::ToString() {
	luaplus_assert(L);
	return lua_tostring(L, ref);
}


// Mirrors lua_towstring().
#if LUA_WIDESTRING
inline const lua_WChar* LuaObject::ToWString() {
	luaplus_assert(L);
	return lua_towstring(L, ref);
}
#endif /* LUA_WIDESTRING */


inline size_t LuaObject::ObjLen() {
	luaplus_assert(L);
	return lua_objlen(L, ref);
}


inline int LuaObject::GetInteger() const {
	luaplus_assert(L  &&  IsInteger());
	return lua_tointeger(L, ref);
}


inline float LuaObject::GetFloat() const {
	luaplus_assert(L  &&  IsNumber());
	return (float)lua_tonumber(L, ref);
}


inline double LuaObject::GetDouble() const {
	luaplus_assert(L  &&  IsNumber());
	return (double)lua_tonumber(L, ref);
}


inline lua_Number LuaObject::GetNumber() const {
	luaplus_assert(L  &&  IsNumber());
	return (lua_Number)lua_tonumber(L, ref);
}


inline const char* LuaObject::GetString() const {
	luaplus_assert(L  &&  IsString());
	return lua_tostring(L, ref);
}


#if LUA_WIDESTRING
inline const lua_WChar* LuaObject::GetWString()const {
	luaplus_assert(L  &&  IsWString());
	return lua_towstring(L, ref);
}
#endif /* LUA_WIDESTRING */


inline size_t LuaObject::StrLen() const {
	luaplus_assert(L);
#if LUA_WIDESTRING
	luaplus_assert(IsString()  ||  IsWString());
#else
	luaplus_assert(IsString());
#endif /* LUA_WIDESTRING */
	return lua_objlen(L, ref);
}


inline NAMESPACE_LUA_PREFIX lua_CFunction LuaObject::GetCFunction() const {
	luaplus_assert(L  &&  IsCFunction());
	return lua_tocfunction(L, ref);
}


// Mirrors lua_touserdata().
inline void* LuaObject::GetUserData() const {
	luaplus_assert(L  &&  IsUserData());
	return lua_touserdata(L, ref);
}


// Mirrors lua_topointer.
inline const void* LuaObject::GetLuaPointer() const {
	luaplus_assert(L);
	return lua_topointer(L, ref);
}


// No equivalent.
inline void* LuaObject::GetLightUserData() const {
	luaplus_assert(L  &&  IsLightUserData());
	return lua_touserdata(L, ref);
}


// Mirrors lua_toboolean().
inline bool LuaObject::GetBoolean() const {
	luaplus_assert(L  &&  IsBoolean()  ||  IsNil());
	return lua_toboolean(L, ref) != 0;
}


inline LuaStackObject LuaObject::Push() const {
	luaplus_assert(L);
	lua_pushvalue(L, ref);
	return LuaStackObject(L, lua_gettop(L));
}


inline LuaObject LuaObject::GetMetaTable() const {
	luaplus_assert(L);
	if (lua_getmetatable(L, ref))
		return LuaObject(L, lua_fastref(L), true);
	return LuaObject(L);
}


inline void LuaObject::SetMetaTable(const LuaObject& valueObj) {
	luaplus_assert(L);
	lua_pushvalue(L, valueObj.ref);
	lua_setmetatable(L, ref);
}


inline void LuaObject::Insert(LuaObject& obj) {
	luaplus_assert(L);
	luaplus_assert(L == obj.L);
	lua_getglobal(L, "table");
	luaplus_assert(lua_istable(L, -1));
	lua_getfield(L, -1, "insert");
	luaplus_assert(lua_isfunction(L, -1));
	Push();
	obj.Push();
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
	Push();
	lua_pushinteger(L, index);
	obj.Push();
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
	Push();
	lua_pushinteger(L, index);
	if (lua_pcall(L, 2, 0, 0) != 0) {
		luaplus_assert(0);
	}
	lua_pop(L, 1);
}


inline void LuaObject::Sort() {
	luaplus_assert(L);
	lua_getglobal(L, "table");
	lua_getfield(L, -1, "sort");
	luaplus_assert(lua_isfunction(L, -1));
	Push();
	lua_call(L, 1, 0);
	lua_pop(L, 1);
}


inline size_t LuaObject::GetCount() const {
	luaplus_assert(L);
	Push();
	int count = lua_objlen(L, -1);
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
inline LuaObject LuaObject::CreateTable(const char* key, int narray, int nrec) {
	luaplus_assert(L);
	lua_pushstring(L, key);				// key
	lua_pushvalue(L, -1);				// key key
	lua_createtable(L, narray, nrec);	// key key table
	lua_settable(L, ref);				// key
	lua_gettable(L, ref);				// table
	return LuaObject(L, lua_fastref(L), true);
}


/**
	Creates a table called [key] within the current LuaStackObject.

	@param index The index of the table to create.
	@param size The size of the table.
	@return Returns the object representing the newly created table.
**/
inline LuaObject LuaObject::CreateTable(int key, int narray, int nrec) {
	luaplus_assert(L);
	lua_pushinteger(L, key);			// key
	lua_pushvalue(L, -1);				// key key
	lua_createtable(L, narray, nrec);	// key key table
	lua_settable(L, ref);				// key
	lua_gettable(L, ref);				// table
	return LuaObject(L, lua_fastref(L), true);
}


/**
	Creates a table called [key] within the current LuaStackObject.

	@param index The index of the table to create.
	@param size The size of the table.
	@return Returns the object representing the newly created table.
**/
inline LuaObject LuaObject::CreateTable(LuaObject& key, int narray, int nrec) {
	luaplus_assert(L);
	lua_pushvalue(L, key.ref);			// key
	lua_pushvalue(L, -1);				// key key
	lua_createtable(L, narray, nrec);	// key key table
	lua_settable(L, ref);				// key
	lua_gettable(L, ref);				// table
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
	luaplus_assert(L  &&  IsTable());
	lua_pushstring(L, key);				// key
	lua_gettable(L, ref);				// value
	return LuaObject(L, lua_fastref(L), true);
}


inline LuaObject LuaObject::Get(int key) const {
	luaplus_assert(L  &&  IsTable());
	lua_pushinteger(L, key);			// key
	lua_gettable(L, ref);				// value
	return LuaObject(L, lua_fastref(L), true);
}

inline LuaObject LuaObject::Get(const LuaObject& key) const {
	luaplus_assert(L  &&  IsTable());
	lua_pushvalue(L, key.ref);			// key
	lua_gettable(L, ref);				// value
	return LuaObject(L, lua_fastref(L), true);
}


inline LuaObject LuaObject::Get(const LuaStackObject& key) const {
	luaplus_assert(L  &&  IsTable());
	lua_pushvalue(L, key.m_stackIndex);	// key
	lua_gettable(L, ref);				// table
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
	luaplus_assert(L  &&  IsTable());
	lua_pushstring(L, key);				// key
	lua_rawget(L, ref);					// value
	return LuaObject(L, lua_fastref(L), true);
}

inline LuaObject LuaObject::RawGetByIndex(int key) const {
	luaplus_assert(L  &&  IsTable());
	lua_rawgeti(L, ref, key);			// value
	return LuaObject(L, lua_fastref(L), true);
}

inline LuaObject LuaObject::RawGetByObject(const LuaObject& key) const {
	luaplus_assert(L  &&  IsTable());
	lua_pushvalue(L, key.ref);			// key
	lua_rawget(L, ref);					// value
	return LuaObject(L, lua_fastref(L), true);
}


inline LuaObject LuaObject::RawGetByObject(const LuaStackObject& key) const {
	luaplus_assert(L  &&  IsTable());
	lua_pushvalue(L, key.m_stackIndex);	// key
	lua_rawget(L, ref);					// value
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


inline LuaObject& LuaObject::SetInteger(const char* key, int value) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetInteger(int key, int value) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetInteger(LuaObject& key, int value) {
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


#if LUA_WIDESTRING
inline LuaObject& LuaObject::SetWString(const char* key, const lua_WChar* value, int len) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetWString(int key, const lua_WChar* value, int len) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetWString(LuaObject& key, const lua_WChar* value, int len) {
	return Set(key, value);
}
#endif /* LUA_WIDESTRING */


inline LuaObject& LuaObject::SetUserData(const char* key, void* value) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetUserData(int key, void* value) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetUserData(LuaObject& key, void* value) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetLightUserData(const char* key, void* value) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetLightUserData(int key, void* value) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetLightUserData(LuaObject& key, void* value) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetObject(const char* key, LuaObject& value) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetObject(int key, LuaObject& value) {
	return Set(key, value);
}


inline LuaObject& LuaObject::SetObject(LuaObject& key, LuaObject& value) {
	return Set(key, value);
}


inline LuaObject& LuaObject::RawSetBoolean(const char* key, bool value) {
	luaplus_assert(L&&  IsTable());
	lua_pushstring(L, key);
	lua_pushboolean(L, value);
	lua_rawset(L, ref);
	return *this;
}


inline LuaObject& LuaObject::RawSetBoolean(int key, bool value) {
	luaplus_assert(L  &&  IsTable());
	lua_pushboolean(L, value);
	lua_rawseti(L, ref, key);
	return *this;
}


inline LuaObject& LuaObject::RawSetBoolean(LuaObject& key, bool value) {
	luaplus_assert(L  &&  IsTable());
	lua_pushvalue(L, key.GetRef());
	lua_pushboolean(L, value);
	lua_rawset(L, ref);
	return *this;
}


inline LuaObject& LuaObject::RawSetInteger(const char* key, int value) {
	luaplus_assert(L&&  IsTable());
	lua_pushstring(L, key);
	lua_pushinteger(L, value);
	lua_rawset(L, ref);
	return *this;
}


inline LuaObject& LuaObject::RawSetInteger(int key, int value) {
	luaplus_assert(L  &&  IsTable());
	lua_pushinteger(L, value);
	lua_rawseti(L, ref, key);
	return *this;
}


inline LuaObject& LuaObject::RawSetInteger(LuaObject& key, int value) {
	luaplus_assert(L  &&  IsTable());
	lua_pushvalue(L, key.GetRef());
	lua_pushinteger(L, value);
	lua_rawset(L, ref);
	return *this;
}


inline LuaObject& LuaObject::RawSetNumber(const char* key, lua_Number value) {
	luaplus_assert(L&&  IsTable());
	lua_pushstring(L, key);
	lua_pushnumber(L, value);
	lua_rawset(L, ref);
	return *this;
}


inline LuaObject& LuaObject::RawSetNumber(int key, lua_Number value) {
	luaplus_assert(L  &&  IsTable());
	lua_pushnumber(L, value);
	lua_rawseti(L, ref, key);
	return *this;
}


inline LuaObject& LuaObject::RawSetNumber(LuaObject& key, lua_Number value) {
	luaplus_assert(L  &&  IsTable());
	lua_pushvalue(L, key.GetRef());
	lua_pushnumber(L, value);
	lua_rawset(L, ref);
	return *this;
}


inline LuaObject& LuaObject::RawSetString(const char* key, const char* value, int len) {
	luaplus_assert(L&&  IsTable());
	lua_pushstring(L, key);
	lua_pushlstring(L, value, len == -1 ? strlen(value) : len);
	lua_rawset(L, ref);
	return *this;
}


inline LuaObject& LuaObject::RawSetString(int key, const char* value, int len) {
	luaplus_assert(L  &&  IsTable());
	lua_pushlstring(L, value, len == -1 ? strlen(value) : len);
	lua_rawseti(L, ref, key);
	return *this;
}


inline LuaObject& LuaObject::RawSetString(LuaObject& key, const char* value, int len) {
	luaplus_assert(L  &&  IsTable());
	lua_pushvalue(L, key.GetRef());
	lua_pushlstring(L, value, len == -1 ? strlen(value) : len);
	lua_rawset(L, ref);
	return *this;
}


#if LUA_WIDESTRING
inline LuaObject& LuaObject::RawSetWString(const char* key, const lua_WChar* value, int len) {
	luaplus_assert(L&&  IsTable());
	lua_pushstring(L, key);
	lua_pushlwstring(L, value, len == -1 ? lua_WChar_len(value) : len);
	lua_rawset(L, ref);
	return *this;
}


inline LuaObject& LuaObject::RawSetWString(int key, const lua_WChar* value, int len) {
	luaplus_assert(L  &&  IsTable());
	lua_pushlwstring(L, value, len == -1 ? lua_WChar_len(value) : len);
	lua_rawseti(L, ref, key);
	return *this;
}


inline LuaObject& LuaObject::RawSetWString(LuaObject& key, const lua_WChar* value, int len) {
	luaplus_assert(L  &&  IsTable());
	lua_pushvalue(L, key.GetRef());
	lua_pushlwstring(L, value, len == -1 ? lua_WChar_len(value) : len);
	lua_rawset(L, ref);
	return *this;
}
#endif /* LUA_WIDESTRING */


inline LuaObject& LuaObject::RawSetUserData(const char* key, void* value) {
	luaplus_assert(L&&  IsTable());
	lua_pushstring(L, key);
	lua_pushlightuserdata(L, value);
	lua_rawset(L, ref);
	return *this;
}


inline LuaObject& LuaObject::RawSetUserData(int key, void* value) {
	luaplus_assert(L  &&  IsTable());
	lua_pushlightuserdata(L, value);
	lua_rawseti(L, ref, key);
}


inline LuaObject& LuaObject::RawSetUserData(LuaObject& key, void* value) {
	luaplus_assert(L  &&  IsTable());
	lua_pushvalue(L, key.GetRef());
	lua_pushlightuserdata(L, value);
	lua_rawset(L, ref);
	return *this;
}


inline LuaObject& LuaObject::RawSetLightUserData(const char* key, void* value) {
	luaplus_assert(L&&  IsTable());
	lua_pushstring(L, key);
	lua_pushlightuserdata(L, value);
	lua_rawset(L, ref);
	return *this;
}


inline LuaObject& LuaObject::RawSetLightUserData(int key, void* value) {
	luaplus_assert(L  &&  IsTable());
	lua_pushlightuserdata(L, value);
	lua_rawseti(L, ref, key);
}


inline LuaObject& LuaObject::RawSetLightUserData(LuaObject& key, void* value) {
	luaplus_assert(L  &&  IsTable());
	lua_pushvalue(L, key.GetRef());
	lua_pushlightuserdata(L, value);
	lua_rawset(L, ref);
	return *this;
}


inline LuaObject& LuaObject::RawSetObject(const char* key, LuaObject& value) {
	luaplus_assert(L&&  IsTable());
	lua_pushstring(L, key);
	lua_pushvalue(L, value.GetRef());
	lua_rawset(L, ref);
	return *this;
}


inline LuaObject& LuaObject::RawSetObject(int key, LuaObject& value) {
	luaplus_assert(L  &&  IsTable());
	lua_pushvalue(L, value.GetRef());
	lua_rawseti(L, ref, key);
}


inline LuaObject& LuaObject::RawSetObject(LuaObject& key, LuaObject& value) {
	luaplus_assert(L  &&  IsTable());
	lua_pushvalue(L, key.GetRef());
	lua_pushvalue(L, value.GetRef());
	lua_rawset(L, ref);
	return *this;
}


inline void LuaObject::AssignNil() {
	luaplus_assert(L);
	lua_fastunref(L, ref);
	lua_pushnil(L);
	ref = lua_fastref(L);
}


inline void LuaObject::AssignNil(lua_State* _L) {
	if (L)
		lua_fastunref(L, ref);
	L = _L;
	lua_pushnil(L);
	ref = lua_fastref(L);
}


inline void LuaObject::AssignNil(LuaState* state) {
	if (L)
		lua_fastunref(L, ref);
	L = LuaState_to_lua_State(state);
	lua_pushnil(L);
	ref = lua_fastref(L);
}


inline void LuaObject::AssignBoolean(LuaState* state, bool value) {
	Assign(state, value);
}


inline void LuaObject::AssignInteger(LuaState* state, int value) {
	Assign(state, value);
}


inline void LuaObject::AssignNumber(LuaState* state, lua_Number value) {
	Assign(state, value);
}


inline void LuaObject::AssignString(LuaState* state, const char* value, int len) {
	Assign(state, value, len);
}


#if LUA_WIDESTRING
inline void LuaObject::AssignWString(LuaState* state, const lua_WChar* value, int len) {
	Assign(LuaState_to_lua_State(state), value, len);
}
#endif /* LUA_WIDESTRING */


inline void LuaObject::AssignUserData(LuaState* state, void* value) {
	Assign(state, value);
}


inline void LuaObject::AssignLightUserData(LuaState* state, void* value) {
	Assign(state, value);
}


inline void LuaObject::AssignObject(LuaObject& value) {
	*this = value;
}


inline void LuaObject::AssignNewTable(int narray, int nrec) {
	luaplus_assert(L);
	lua_fastunref(L, ref);
	lua_createtable(L, narray, nrec);
	ref = lua_fastref(L);
}


inline void LuaObject::AssignNewTable(lua_State* _L, int narray, int nrec) {
	if (L)
		lua_fastunref(L, ref);
	L = _L;
	lua_createtable(L, narray, nrec);
	ref = lua_fastref(L);
}


inline void LuaObject::AssignNewTable(LuaState* state, int narray, int nrec) {
	if (L)
		lua_fastunref(L, ref);
	L = LuaState_to_lua_State(state);
	lua_createtable(L, narray, nrec);
	ref = lua_fastref(L);
}


/**
	Assuming the current object is a table, registers a C function called
	[funcName] within the table.

	@param funcName The name of the function to register.
	@param function A pointer to the C function to register.
**/
inline void LuaObject::Register(const char* funcName, NAMESPACE_LUA_PREFIX lua_CFunction function, int nupvalues) {
	luaplus_assert(L);
	lua_pushcclosure(L, function, nupvalues);
	lua_setfield(L, ref, funcName);
}


inline void LuaObject::Register(const char* funcName, int (*function)(LuaState*), int nupvalues) {
	luaplus_assert(L);
	lua_pushcclosure(L, (lua_CFunction)function, nupvalues);
	lua_setfield(L, ref, funcName);
}


inline void LuaObject::RegisterHelper(const char* funcName, NAMESPACE_LUA_PREFIX lua_CFunction function, int nupvalues, const void* callee, unsigned int sizeofCallee, void* func, unsigned int sizeofFunc) {
	luaplus_assert(L);

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
	lua_setfield(L, ref, funcName);
}


/**
	Assuming the current object is a table, unregisters the function called
	[funcName].

	@param funcName The name of the function to unregister.
**/
inline void LuaObject::Unregister(const char* funcName)
{
	luaplus_assert(L);
	SetNil(funcName);
}


/**
**/
inline bool LuaObject::operator==(const LuaObject& right) const
{
	luaplus_assert(L);
	return lua_State_To_LuaState(L)->Equal(*this, right) != 0;
}

	
/**
**/
inline bool LuaObject::operator<(const LuaObject& right) const
{
	luaplus_assert(L);
	return lua_State_To_LuaState(L)->LessThan(*this, right) != 0;
}

	
/**
**/
inline LuaState* LuaObject::GetState() const
{
	return lua_State_To_LuaState(L);
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
		tableObj.AssignNewTable(lua_State_To_LuaState(L), 0, 0);
		tableObj.SetMetaTable(GetMetaTable());

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
	luaplus_assert(0);
#if TODO
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
#if LUA_WIDESTRING
				case LUA_TWSTRING:	keyObj.AssignWString(outObj.GetState(), it.GetKey().GetWString());		break;
#endif /* LUA_WIDESTRING */
			}

			switch (it.GetValue().Type())
			{
				case LUA_TBOOLEAN:	outObj.SetBoolean(keyObj, it.GetValue().GetBoolean());		break;
				case LUA_TNUMBER:		outObj.SetNumber(keyObj, it.GetValue().GetNumber());	break;
				case LUA_TSTRING:		outObj.SetString(keyObj, it.GetValue().GetString());	break;
#if LUA_WIDESTRING
				case LUA_TWSTRING:	outObj.SetWString(keyObj, it.GetValue().GetWString());		break;
#endif /* LUA_WIDESTRING */
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
#if LUA_WIDESTRING
			case LUA_TWSTRING:	outObj.AssignWString(outObj.GetState(), this->GetWString());		break;
#endif /* LUA_WIDESTRING */
			case LUA_TTABLE:	DeepClone(outObj);													break;
		}
	}
#endif
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

#if !defined(_MSC_VER)
            delete [] buf;
#endif
			return table[lastPos];
		}

		*curPos = 0;
		if (LuaObject_str2d(lastPos, &num)) {
			table = table[(int)num];
		} else {
			table = table[lastPos];
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


namespace LuaHelper {

inline bool GetBoolean( LuaObject& obj, int key, bool require, bool defaultValue ) {
	LuaObject boolObj = obj[ key ];
	if ( !boolObj.IsBoolean() ) {
		if ( require ) {
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return boolObj.GetBoolean();
}


inline bool GetBoolean( LuaObject& obj, const char* key, bool require, bool defaultValue ) {
	LuaObject boolObj = obj[ key ];
	if ( !boolObj.IsBoolean() ) {
		if ( require ) {
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return boolObj.GetBoolean();
}


inline int GetInteger( LuaObject& obj, int key, bool require, int defaultValue ) {
	LuaObject intObj = obj[ key ];
	if ( !intObj.IsInteger() ) {
		if ( require ) {
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return intObj.GetInteger();
}


inline int GetInteger( LuaObject& obj, const char* key, bool require, int defaultValue ) {
	LuaObject intObj = obj[ key ];
	if ( !intObj.IsInteger() ) {
		if ( require ) {
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return intObj.GetInteger();
}


inline float GetFloat( LuaObject& obj, int key, bool require, float defaultValue ) {
	LuaObject floatObj = obj[ key ];
	if ( !floatObj.IsNumber() ) {
		if ( require ) {
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return (float)floatObj.GetNumber();
}


inline float GetFloat( LuaObject& obj, const char* key, bool require, float defaultValue ) {
	LuaObject floatObj = obj[ key ];
	if ( !floatObj.IsNumber() ) {
		if ( require ) {
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return (float)floatObj.GetNumber();
}


inline void* GetLightUserData( LuaObject& obj, int key, bool require, void* defaultValue ) {
	LuaObject outObj = obj[ key ];
	if ( !outObj.IsLightUserData() ) {
		if ( require ) {
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return outObj.GetLightUserData();
}


inline void* GetLightUserData( LuaObject& obj, const char* key, bool require, void* defaultValue ) {
	LuaObject outObj = obj[ key ];
	if ( !outObj.IsLightUserData() ) {
		if ( require ) {
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return outObj.GetLightUserData();
}


inline const char* GetString( LuaObject& obj, int key, bool require, const char* defaultValue ) {
	LuaObject stringObj = obj[ key ];
	if ( !stringObj.IsString() ) {
		if ( require ) {
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return stringObj.GetString();
}


inline const char* GetString( LuaObject& obj, const char* key, bool require, const char* defaultValue ) {
	LuaObject stringObj = obj[ key ];
	if ( !stringObj.IsString() ) {
		if ( require ) {
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return stringObj.GetString();
}


inline LuaObject GetTable( LuaObject& obj, int key, bool require ) {
	LuaObject tableObj = obj[ key ];
	if ( !tableObj.IsTable() ) {
		if ( require ) {
			luaplus_assert( 0 );
		}
	}
	return tableObj;
}


inline LuaObject GetTable( LuaObject& obj, const char* key, bool require ) {
	LuaObject tableObj = obj[ key ];
	if ( !tableObj.IsTable() ) {
		if ( require ) {
			luaplus_assert( 0 );
		}
	}
	return tableObj;
}

} // namespace LuaHelper



/**
**/
inline void LuaObject::AssignCFunction(NAMESPACE_LUA_PREFIX lua_CFunction function, int nupvalues)
{
	AssignCFunctionHelper(function, nupvalues, NULL, 0, NULL, 0);
}


/**
**/
inline void LuaObject::AssignCFunction(int (*func)(LuaState*), int nupvalues)
{
	AssignCFunctionHelper(LPCD::LuaStateFunctionDispatcher, nupvalues, NULL, 0, &func, sizeof(func));
}


} // namespace LuaPlus

#endif // LUAOBJECT_INL
