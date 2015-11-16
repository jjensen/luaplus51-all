///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2011 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUAPLUS__LUASTATE_H
#define LUAPLUS__LUASTATE_H

#include "LuaPlusInternal.h"

///////////////////////////////////////////////////////////////////////////////
// namespace LuaPlus
///////////////////////////////////////////////////////////////////////////////
namespace LuaPlus
{

/**
	A lua_State wrapper.
**/
class LuaState {
public:
	enum DumpObjectTypes {
		DUMP_ALPHABETICAL		= 0x00000001,
		DUMP_WRITEALL			= 0x00000002,
		DUMP_WRITETABLEPOINTERS = 0x00000004,
	};


	///////////////////////////////////////////////////////////////////////////
	static LuaState* Create();
	static LuaState* Create(bool initStandardLibrary);
	static LuaState* Create(lua_Alloc allocFunction, void* userdata);
	static LuaState* Create(lua_Alloc allocFunction, void* userdata, bool initStandardLibrary);
	static LuaObject CreateThread(LuaState* parentState);
	static void Destroy(LuaState* state);

	lua_CFunction AtPanic(lua_CFunction panicf);

	void OpenLibs();

	// Basic stack manipulation.
	LuaStackObject Stack(int index);
	LuaStackObject StackTop();

	int AbsIndex(int index);
	int GetTop();
	void SetTop(int index);
	void PushGlobalTable();
	void PushValue(int index);
	void PushValue(LuaStackObject& object);
#if LUA_VERSION_NUM >= 503
	void Rotate(int index, int n);
#endif
	void Remove(int index);
	void Insert(int index);
	void Replace(int index);
	void Copy(int fromindex, int toindex);
	int CheckStack(int size);

	void XMove(LuaState* to, int n);

	// access functions (stack -> C)
	int IsNumber(int index) const;
	int IsString(int index) const;
	int IsCFunction(int index) const;
	int IsInteger(int index) const;
	int IsUserdata(int index) const;
	int IsFunction(int index) const;
	int IsTable(int index) const;
	int IsLightUserdata(int index) const;
	int IsNil(int index) const;
	int IsBoolean(int index) const;
	int IsThread(int index) const;
	int IsNone(int index) const;
	int IsNoneOrNil(int index) const;
	int Type(int index) const;
	const char* TypeName(int type);

	lua_Number ToNumber(int index);
	lua_Number ToNumberX(int index, int *isnum);
	lua_Integer ToInteger(int index);
	lua_Integer ToIntegerX(int index, int *isnum);
#if LUA_VERSION_NUM == 501
	unsigned int ToUnsigned(int index);
	unsigned int ToUnsignedX(int index, int *isnum);
#elif LUA_VERSION_NUM >= 502
	lua_Unsigned ToUnsigned(int index);
	lua_Unsigned ToUnsignedX(int index, int *isnum);
#endif
	int ToBoolean(int index);
	const char* ToLString(int index, size_t* len);
	const char* ToString(int index);
	size_t RawLen(int index);
	size_t ObjLen(int index);
	lua_CFunction ToCFunction(int index);
	void* ToUserdata(int index);
	lua_State* ToThread(int index);
	const void* ToPointer(int index);

	// Comparison and arithmetic functions
	void Arith(int op);
	int RawEqual(int index1, int index2);
	int Compare(int index1, int index2, int op);

	int Equal(int index1, int index2);
 	int Equal(const LuaObject& o1, const LuaObject& o2);
	int LessThan(int index1, int index2);
	int LessThan(const LuaObject& o1, const LuaObject& o2);

	// push functions (C -> stack)
	LuaStackObject PushNil();
	LuaStackObject PushNumber(lua_Number n);
	LuaStackObject PushInteger(lua_Integer n);
#if LUA_VERSION_NUM == 501
	LuaStackObject PushUnsigned(unsigned int n);
#elif LUA_VERSION_NUM >= 502
	LuaStackObject PushUnsigned(lua_Unsigned n);
#endif
	LuaStackObject PushLString(const char *s, size_t len);
	LuaStackObject PushString(const char *s);
	const char* PushVFString(const char* fmt, va_list argp);
	const char* PushFString(const char* fmt, ...);

	LuaStackObject PushCClosure(lua_CFunction fn, int n);
	LuaStackObject PushCClosure(int (*f)(LuaState*), int n);

	LuaStackObject PushCFunction(lua_CFunction f);
	LuaStackObject PushBoolean(int value);
	LuaStackObject PushLightUserdata(void* p);
	LuaStackObject PushThread();

	LuaStackObject Push(const LuaObject& obj);

	// get functions (Lua -> stack)
	void GetTable(int index);
	void GetField(int index, const char* key);
	void GetI(int index, lua_Integer key);
	void RawGet(int index);
	void RawGetI(int index, lua_Integer n);
	void RawGetP(int index, const void* p);
	LuaStackObject CreateTable(int narr = 0, int nrec = 0);
	LuaStackObject NewUserdata(size_t size);
	LuaStackObject GetMetatable(int objindex);
	LuaStackObject GetUservalue(int index);
	LuaStackObject GetFEnv(int index);

	// LuaPlus ---->
	LuaObject GetGlobals() throw();
	LuaObject GetRegistry();
	LuaObject GetGlobal(const char *name);

	LuaStackObject GetGlobals_Stack();					// Backward compatible.
	LuaStackObject GetRegistry_Stack();
	LuaStackObject GetGlobal_Stack(const char *name);

	// set functions(stack -> Lua)
	void SetGlobal(const char* key);
	void SetTable(int index);
	void SetField(int index, const char* key);
	void SetI(int index, lua_Integer key);
	void RawSet(int index);
	void RawSetI(int index, lua_Integer n);
	void RawSetP(int index, const void* p);
	void SetMetatable(int index);
	void SetUservalue(int index);
	void SetFEnv(int index);

	// `load' and `call' functions (load and run Lua code)
#if LUA_VERSION_NUM == 501  ||  LUA_VERSION_NUM == 502
	void CallK(int nargs, int nresults, int ctx, lua_CFunction k);
#else
	void CallK(int nargs, int nresults, lua_KContext ctx, lua_KFunction k);
#endif
	void Call(int nargs, int nresults);
#if LUA_VERSION_NUM == 501  ||  LUA_VERSION_NUM == 502
	int GetCtx(int *ctx);
#endif
#if LUA_VERSION_NUM == 501  ||  LUA_VERSION_NUM == 502
	int PCallK(int nargs, int nresults, int errfunc, int ctx, lua_CFunction k);
#else
	int PCallK(int nargs, int nresults, int errfunc, lua_KContext ctx, lua_KFunction k);
#endif
	int PCall(int nargs, int nresults, int errfunc);
	int CPCall(lua_CFunction func, void* ud);
	int Load(lua_Reader reader, void* data, const char* chunkname, const char* mode);

#if LUA_ENDIAN_SUPPORT
	int Dump(lua_Writer writer, void* data, int strip, char endian);
#else
	int Dump(lua_Writer writer, void* data, int strip = 0);
#endif /* LUA_ENDIAN_SUPPORT */

	/*
	** coroutine functions
	*/
#if LUA_VERSION_NUM == 501  ||  LUA_VERSION_NUM == 502
	int YieldK(int nresults, int ctx, lua_CFunction k);
#else
	int YieldK(int nresults, lua_KContext ctx, lua_KFunction k);
#endif
	int Yield_(int nresults);
	int Resume(lua_State *from, int narg);
	int Resume(LuaState *from, int narg);
	int Status();
#if LUA_VERSION_NUM >= 503
	int IsYieldable();
#endif

	/*
	** garbage-collection function and options
	*/
	int GC(int what, int data);

	/*
	** miscellaneous functions
	*/
	int Error();

	int Next(int index);

	void Concat(int n);

	void Len(int index);

	size_t StringToNumber(const char *s);

	lua_Alloc GetAllocF(void **ud);
	void SetAllocF(lua_Alloc f, void *ud);

	// Helper functions
	void Pop();
	void Pop(int amount);
	void NewTable();

	void Register(const char* key, lua_CFunction f);
	size_t StrLen(int index);


	// debug functions
	int GetStack(int level, lua_Debug* ar);
	int GetInfo(const char* what, lua_Debug* ar);
	const char* GetLocal(const lua_Debug* ar, int n);
	const char* SetLocal(const lua_Debug* ar, int n);
	const char* GetUpvalue(int funcindex, int n);
	const char* SetUpvalue(int funcindex, int n);
	void *UpvalueID(int fidx, int n);
	void UpvalueJoin(int fidx1, int n1, int fidx2, int n2);

	void SetHook(lua_Hook func, int mask, int count);
	lua_Hook GetHook();
	int GetHookMask();
	int GetHookCount();

	// fastref support
#if LUA_FASTREF_SUPPORT
	int FastRef();
	int FastRefIndex(int index);
	void FastUnref(int ref);
	void GetFastRef(int ref);
#endif /* LUA_FASTREF_SUPPORT */

	// lauxlib functions.
	void OpenLib(const char *libname, const luaL_Reg *l, int nup);
	void NewLib(const luaL_Reg *l, int nup);
	void LRegister(const char *libname, const luaL_Reg *l);

	int GetMetaField(int obj, const char *e);
	int CallMeta(int obj, const char *e);
	int TypeError(int narg, const char* tname);
	int ArgError(int numarg, const char* extramsg);
	const char* CheckLString(int numArg, size_t* len);
	const char* OptLString(int numArg, const char *def, size_t* len);
	lua_Number CheckNumber(int numArg);
	lua_Number OptNumber(int nArg, lua_Number def);
	lua_Integer CheckInteger(int numArg);
	lua_Integer OptInteger(int nArg, lua_Integer def);
#if LUA_VERSION_NUM == 501
	unsigned int CheckUnsigned(int numArg);
	unsigned int OptUnsigned(int nArg, unsigned int def);
#elif LUA_VERSION_NUM >= 502
	lua_Unsigned CheckUnsigned(int numArg);
	lua_Unsigned OptUnsigned(int nArg, lua_Unsigned def);
#endif
	lua_Integer CheckBoolean(int narg);
	lua_Integer OptBoolean(int narg, lua_Integer def);
	void CheckStack(int sz, const char* msg);
	void CheckType(int narg, int t);
	void CheckAny(int narg);

	LuaObject NewMetatable(const char* tname);
	LuaObject GetMetatable(const char* metatableName);
	LuaObject SetMetatable(const char* metatableName);
	void* TestUData(int ud, const char *tname);
	void* CheckUData(int ud, const char* tname);

	void Where(int lvl);
	int Error(const char* fmt, ...);

	int CheckOption(int narg, const char *def, const char *const lst[]);

	int FileResult(int stat, const char *fname);
	int ExecResult(int stat);

	int Ref(int t);
	void Unref(int t, int ref);

	int LoadFileX(const char* filename, const char* mode);
	int LoadFile(const char* filename);
	int LoadBufferX(const char* buff, size_t size, const char* name, const char* mode);
	int LoadBuffer(const char* buff, size_t size, const char* name);
	int LoadString(const char* str);

	lua_Integer L_len(int index);

	const char* GSub(const char *s, const char *p, const char *r);

	void SetFuncs(const luaL_Reg *l, int nup);

	int GetSubTable(int idx, const char *fname);

	void Traceback(lua_State *L1, const char *msg, int level);

	void RequireF(const char *modname, lua_CFunction openf, int glb);

	const char* FindTable(int idx, const char *fname, int szhint);

	void ArgCheck(bool condition, int numarg, const char* extramsg);
	const char* CheckString(int numArg);
	const char* OptString(int numArg, const char* def);
	int CheckInt(int numArg);
	int OptInt(int numArg, int def);
	long CheckLong(int numArg);
	long OptLong(int numArg, int def);

	int DoFile(const char *filename);
	int DoString(const char *str);
	int DoBuffer(const char *buff, size_t size, const char *name);

	int DoFile(const char *filename, LuaObject& fenvObj);
	int DoString(const char *str, LuaObject& fenvObj);
	int DoBuffer(const char *buff, size_t size, const char *name, LuaObject& fenvObj);

	const char* LTypeName(int index);

	LuaObject NewUserdataBox(void* u);

#if LUAPLUS_DUMPOBJECT
	LUAPLUS_API bool DumpObject(const char* filename, const char* name, LuaObject& value, unsigned int flags = DUMP_ALPHABETICAL,
					int indentLevel = 0, unsigned int maxIndentLevel = 0xffffffff);
	LUAPLUS_API bool DumpObject(const char* filename, LuaObject& key, LuaObject& value, unsigned int flags = DUMP_ALPHABETICAL,
					int indentLevel = 0, unsigned int maxIndentLevel = 0xffffffff);

	LUAPLUS_API bool DumpObject(LuaStateOutFile& file, const char* name, LuaObject& value, unsigned int flags = DUMP_ALPHABETICAL,
					int indentLevel = 0, unsigned int maxIndentLevel = 0xffffffff);
	LUAPLUS_API bool DumpObject(LuaStateOutFile& file, LuaObject& key, LuaObject& value, unsigned int flags = DUMP_ALPHABETICAL,
					int indentLevel = 0, unsigned int maxIndentLevel = 0xffffffff);
#endif // LUAPLUS_DUMPOBJECT

	operator lua_State*()						{  return (lua_State*)this;  }
	lua_State* GetCState()						{  return (lua_State*)this;  }

	// Extra
	LuaStackObject BoxPointer(void* u);
	void* UnBoxPointer(int stackIndex);

	int UpvalueIndex(int i);

	LuaObject GetLocalByName( int level, const char* name );

protected:
	LuaState();
	~LuaState();
	LuaState& operator=(LuaState& src);		// Not implemented.

#if LUAPLUS_DUMPOBJECT
	bool CallFormatting(LuaObject& tableObj, LuaStateOutFile& file, int indentLevel,
			bool writeAll, bool alphabetical, bool writeTablePointers,
			unsigned int maxIndentLevel);
#endif // LUAPLUS_DUMPOBJECT
};


class LuaStateAuto {
public:
    operator LuaState*()							{  return m_state;  }
    operator const LuaState*() const				{  return m_state;  }
    operator LuaState*() const						{  return m_state;  }
    LuaState& operator*()							{  return *m_state; }
    const LuaState& operator*() const				{  return *m_state; }
    LuaState* operator->()							{  return m_state;  }
    const LuaState* operator->() const				{  return m_state;  }
	LuaState* Get() const							{  return m_state;  }

	LuaStateAuto() : m_state(NULL) {}
    LuaStateAuto(LuaState* newState) : m_state(newState) {}
	LuaStateAuto& operator=(LuaState* newState) {
		Assign(newState);
		return *this;
	}

	~LuaStateAuto() {
		Assign(NULL);
	}

    LuaState* m_state;

protected:
    LuaStateAuto(const LuaStateAuto&);					// Not implemented.
    LuaStateAuto& operator=(const LuaStateAuto&);		// Not implemented.

	void Assign(LuaState* state) {
		if (m_state)
			LuaState::Destroy(m_state);
		m_state = state;
	}
};


class LuaStateOwner : public LuaStateAuto {
public:
    LuaStateOwner() {
		m_state = LuaState::Create();
	}

    LuaStateOwner(bool initStandardLibrary) {
		m_state = LuaState::Create(initStandardLibrary);
	}

    LuaStateOwner(lua_Alloc allocFunction, void* userdata) {
		m_state = LuaState::Create(allocFunction, userdata);
	}

    LuaStateOwner(lua_Alloc allocFunction, void* userdata, bool initStandardLibrary) {
		m_state = LuaState::Create(allocFunction, userdata, initStandardLibrary);
	}

    LuaStateOwner(LuaState* newState) : LuaStateAuto(newState) {}
	LuaStateOwner& operator=(LuaState* newState) {
		Assign(newState);
		return *this;
	}

	~LuaStateOwner() {
		Assign(NULL);
	}

private:
    LuaStateOwner(const LuaStateOwner&);				// Not implemented.
    LuaStateOwner& operator=(const LuaStateOwner&);		// Not implemented.
};


} // namespace LuaPlus


namespace LPCD {
	using namespace LuaPlus;

	template<> struct Type<LuaPlus::LuaState*> {
		static inline void Push(lua_State* L, LuaPlus::LuaState* value)						{  lua_pushlightuserdata(L, value);  }
		static inline bool Match(lua_State* L, int idx)										{  (void)L, (void)idx;  return true;  }
		static inline LuaPlus::LuaState* Get(lua_State* L, int idx)							{  return lua_State_to_LuaState(L);  }
	};
} // namespace LPCD




//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "LuaState.inl"

#endif // LUAPLUS__LUASTATE_H
