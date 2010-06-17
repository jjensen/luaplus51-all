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

#if LUAPLUS_EXTENSIONS

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
	ref = _ref;
}


inline LuaObject::LuaObject(lua_State* _L, bool popTop) throw()
	: L(_L)
{
	ref = lua_fastref(L);
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
	return lua_typename(L, lua_type(L, ref));
}


// Mirrors lua_type().
inline int LuaObject::Type() const {
	return lua_type(L, ref);
}


// Mirrors lua_isnil().
inline bool LuaObject::IsNil() const {
	return lua_isnil(L, ref);
}


// Mirrors lua_istable().
inline bool LuaObject::IsTable() const {
	return lua_istable(L, ref);
}


// Mirrors lua_isuserdata().
inline bool LuaObject::IsUserData() const {
	return lua_isuserdata(L, ref) != 0;
}


// Mirrors lua_iscfunction().
inline bool LuaObject::IsCFunction() const {
	return lua_iscfunction(L, ref) != 0;
}


// Behaves differently than lua_isinteger().  This function only tests for a value that is
// a real integer, not something that can be converted to a integer.
inline bool LuaObject::IsInteger() const {
	return lua_type(L, ref) == LUA_TNUMBER;
}


// Behaves differently than lua_isnumber().  This function only tests for a value that is
// a real number, not something that can be converted to a number.
inline bool LuaObject::IsNumber() const {
	return lua_type(L, ref) == LUA_TNUMBER;
}


// Behaves differently than lua_isstring().  This function only tests for a value that is
// a real string, not something that can be converted to a string.
inline bool LuaObject::IsString() const {
	return lua_type(L, ref) == LUA_TSTRING;
}


#if LUA_WIDESTRING
inline bool LuaObject::IsWString() const {
	return lua_type(L, ref) == LUA_TWSTRING;
}
#endif /* LUA_WIDESTRING */


// Mirrors lua_isinteger().
inline bool LuaObject::IsConvertibleToInteger() const {
	return lua_isnumber(L, ref) != 0;
}


// Mirrors lua_isnumber().
inline bool LuaObject::IsConvertibleToNumber() const {
	return lua_isnumber(L, ref) != 0;
}


// Mirrors lua_isstring().
inline bool LuaObject::IsConvertibleToString() const {
	return lua_isstring(L, ref) != 0;
}


// Mirrors lua_iswstring().
#if LUA_WIDESTRING
inline bool LuaObject::IsConvertibleToWString() const {
	return lua_iswstring(L, ref) != 0;
}
#endif /* LUA_WIDESTRING */


// Mirrors lua_isfunction().
inline bool LuaObject::IsFunction() const {
	return lua_isfunction(L, ref);
}


// Mirrors lua_isnone().
inline bool LuaObject::IsNone() const {
	return lua_isnone(L, ref);
}


// Mirrors lua_islightuserdata().
inline bool LuaObject::IsLightUserData() const {
	return lua_islightuserdata(L, ref);
}


// Mirrors lua_isboolean().
inline bool LuaObject::IsBoolean() const {
	return lua_isboolean(L, ref);
}


// Mirrors lua_tointeger()
inline int LuaObject::ToInteger() {
	return lua_tointeger(L, ref);
}


// Mirrors lua_tonumber()
inline lua_Number LuaObject::ToNumber() {
	return lua_tonumber(L, ref);
}


// Mirrors lua_tostring().
inline const char* LuaObject::ToString() {
	return lua_tostring(L, ref);
}


// Mirrors lua_towstring().
#if LUA_WIDESTRING
inline const lua_WChar* LuaObject::ToWString() {
	return lua_towstring(L, ref);
}
#endif /* LUA_WIDESTRING */


inline size_t LuaObject::ObjLen() {
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


inline int LuaObject::StrLen() {
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
inline void* LuaObject::GetUserData() {
	luaplus_assert(L  &&  IsUserData());
	return lua_touserdata(L, ref);
}


// Mirrors lua_topointer.
inline const void* LuaObject::GetLuaPointer() {
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


inline LuaObject LuaObject::GetMetaTable() {
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
/*jj	luaplus_assert(L);
	luaplus_assert(L == obj.L);
	LuaAutoBlock autoBlock(L);
	LuaState* state = lua_State_To_LuaState(L);
	LuaObject tableObj = state->GetGlobal("table");
	LuaObject funcObj = tableObj["insert"];
	luaplus_assert(funcObj.IsFunction());
    LuaCall callObj(funcObj);
	callObj << *this << obj << LuaRun();
*/
}


inline void LuaObject::Insert(int index, LuaObject& obj) {
/*jj	luaplus_assert(L);
	luaplus_assert(L == obj.L);
	LuaAutoBlock autoBlock(L);
	LuaState* state = lua_State_To_LuaState(L);
	LuaObject tableObj = state->GetGlobal("table");
	LuaObject funcObj = tableObj["insert"];
	luaplus_assert(funcObj.IsFunction());
    LuaCall callObj(funcObj);
	callObj << *this << index << obj << LuaRun();
*/
}


inline void LuaObject::Remove(int index) {
/*jj	luaplus_assert(L);
	LuaAutoBlock autoBlock(L);
	LuaState* state = lua_State_To_LuaState(L);
	LuaObject tableObj = state->GetGlobal("table");
	LuaObject funcObj = tableObj["remove"];
	luaplus_assert(funcObj.IsFunction());
    LuaCall callObj(funcObj);
	callObj << *this << index << LuaRun();
*/
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


inline int LuaObject::GetCount() {
	luaplus_assert(L);
	Push();
	int count = lua_objlen(L, -1);
	lua_pop(L, 1);
	return count;
}


inline int LuaObject::GetTableCount() {
	int count = 0;
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


inline LuaObject LuaObject::GetByName(const char* key) {
	luaplus_assert(L  &&  IsTable());
	lua_pushstring(L, key);				// key
	lua_gettable(L, ref);				// value
	return LuaObject(L, lua_fastref(L), true);

#if FASTER
	luaplus_assert(L);

	api_check(L, ttistable(&m_object));

	TValue str;
	setnilvalue2n(L, &str);

	// It's safe to assume that if name is not in the hash table, this function can return nil.
	size_t l = strlen(key);
	GCObject *o;
	unsigned int h = (unsigned int)l;  /* seed */
	size_t step = (l>>5)+1;  /* if string is too long, don't hash all its chars */
	size_t l1;
	for (l1=l; l1>=step; l1-=step)  /* compute hash */
		h = h ^ ((h<<5)+(h>>2)+(unsigned char)(key[l1-1]));
	for (o = G(L)->strt.hash[lmod(h, G(L)->strt.size)];
		o != NULL;
		o = o->gch.next)
	{
		TString *ts = rawgco2ts(o);
		if (ts->tsv.tt == LUA_TSTRING && ts->tsv.len == l && (memcmp(key, getstr(ts), l) == 0))
		{
			/* string may be dead */
			if (isdead(G(L), o)) changewhite(o);
			setsvalue2n(L, &str, ts);
			break;
		}
	}

	if (ttype(&str) == LUA_TNIL)
		return LuaObject(L);

	TValue v;
	luaV_gettable(L, &m_object, &str, &v);
	setnilvalue(&str);
	return LuaObject(L, &v);
#endif
}

inline LuaObject LuaObject::GetByIndex(int key) {
	luaplus_assert(L  &&  IsTable());
	lua_pushinteger(L, key);			// key
	lua_gettable(L, ref);				// value
	return LuaObject(L, lua_fastref(L), true);
}

inline LuaObject LuaObject::GetByObject(const LuaObject& key) {
	luaplus_assert(L  &&  IsTable());
	lua_pushvalue(L, key.ref);			// key
	lua_gettable(L, ref);				// value
	return LuaObject(L, lua_fastref(L), true);
}


inline LuaObject LuaObject::GetByObject(const LuaStackObject& key) {
	luaplus_assert(L  &&  IsTable());
	lua_pushvalue(L, key.m_stackIndex);	// key
	lua_gettable(L, ref);				// table
	return LuaObject(L, lua_fastref(L), true);
}


inline LuaObject LuaObject::RawGetByName(const char* key) {
	return (*this)[key];
}

inline LuaObject LuaObject::RawGetByIndex(int key) {
	return (*this)[key];
}

inline LuaObject LuaObject::RawGetByObject(const LuaObject& key) {
	return (*this)[key];
}


inline LuaObject LuaObject::RawGetByObject(const LuaStackObject& key) {
	return (*this)[key];
}


inline LuaObject LuaObject::operator[](const char* key) {
	luaplus_assert(L  &&  IsTable());
	lua_pushstring(L, key);				// key
	lua_rawget(L, ref);					// value
	return LuaObject(L, lua_fastref(L), true);

#if FASTER
	luaplus_assert(L);
	api_check(L, ttistable(&m_object));

	TValue str;
	setnilvalue2n(L, &str);

	// It's safe to assume that if name is not in the hash table, this function can return nil.
	size_t l = strlen(name);
	GCObject *o;
	unsigned int h = (unsigned int)l;  /* seed */
	size_t step = (l>>5)+1;  /* if string is too long, don't hash all its chars */
	size_t l1;
	for (l1=l; l1>=step; l1-=step)  /* compute hash */
		h = h ^ ((h<<5)+(h>>2)+(unsigned char)(name[l1-1]));
	for (o = G(L)->strt.hash[lmod(h, G(L)->strt.size)];
		o != NULL;
		o = o->gch.next)
	{
		TString *ts = rawgco2ts(o);
		if (ts->tsv.tt == LUA_TSTRING && ts->tsv.len == l && (memcmp(name, getstr(ts), l) == 0))
		{
			/* string may be dead */
			if (isdead(G(L), o)) changewhite(o);
			setsvalue2n(L, &str, ts);
			break;
		}
	}

	if (ttype(&str) == LUA_TNIL)
		return LuaObject(L);

//	setsvalue(&str, luaS_newlstr(L, name, strlen(name)));
	const TValue* v = luaH_get(hvalue(&m_object), &str);
	setnilvalue(&str);
	return LuaObject(L, v);
#endif
}

inline LuaObject LuaObject::operator[](int index) {
	luaplus_assert(L  &&  IsTable());
	lua_rawgeti(L, ref, index);			// value
	return LuaObject(L, lua_fastref(L), true);
}

inline LuaObject LuaObject::operator[](const LuaObject& obj) {
	luaplus_assert(L  &&  IsTable());
	lua_pushvalue(L, obj.ref);			// key
	lua_rawget(L, ref);					// value
	return LuaObject(L, lua_fastref(L), true);
}

inline LuaObject LuaObject::operator[](const LuaStackObject& obj) {
	luaplus_assert(L  &&  IsTable());
	lua_pushvalue(L, obj.m_stackIndex);	// key
	lua_rawget(L, ref);					// value
	return LuaObject(L, lua_fastref(L), true);
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
	lua_pushcclosure(L, function, nupvalues);
	lua_setfield(L, ref, funcName);
}


inline void LuaObject::Register(const char* funcName, int (*function)(LuaState*), int nupvalues) {
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

inline int LuaObject::GetRef() const
{
	return ref;
}


/**
**/
inline LuaObject LuaObject::Clone() {
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


inline void LuaObject::DeepClone(LuaObject& outObj)
{
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


inline LuaObject LuaObject::Lookup(const char* key) {
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



} // namespace LuaPlus

#endif // LUAPLUS_EXTENSIONS

#endif // LUAOBJECT_INL
