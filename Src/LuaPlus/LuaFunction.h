///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2010 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUAFUNCTION_H
#define LUAFUNCTION_H

#include "LuaPlusInternal.h"
#include "LuaAutoBlock.h"

#if LUAPLUS_EXTENSIONS

namespace LuaPlus {

/**
**/
template <typename RT>
class LuaFunction
{
public:
	LuaFunction(LuaObject& _functionObj)
		: functionObj(_functionObj) {
	}

	LuaFunction(LuaState* state, const char* functionName) {
		functionObj = state->GetGlobals()[functionName];
	}

	RT operator()() {
		lua_State* L = functionObj.GetCState();
		LuaAutoBlock autoBlock(L);
		functionObj.Push();

		if (lua_pcall(L, 0, 1, 0)) {
			const char* errorString = lua_tostring(L, -1);  (void)errorString;
			luaplus_assert(0);
		}
		return LPCD::Get(LPCD::TypeWrapper<RT>(), L, -1);
	}

	template <typename P1>
	RT operator()(P1 p1) {
		lua_State* L = functionObj.GetCState();
		LuaAutoBlock autoBlock(L);
		functionObj.Push();

		LPCD::Push(L, p1);

		if (lua_pcall(L, 1, 1, 0)) {
			const char* errorString = lua_tostring(L, -1);  (void)errorString;
			luaplus_assert(0);
		}
		return LPCD::Get(LPCD::TypeWrapper<RT>(), L, -1);
	}

	template <typename P1, typename P2>
	RT operator()(P1 p1, P2 p2) {
		lua_State* L = functionObj.GetCState();
		LuaAutoBlock autoBlock(L);
		functionObj.Push();

		LPCD::Push(L, p1);
		LPCD::Push(L, p2);

		if (lua_pcall(L, 2, 1, 0)) {
			const char* errorString = lua_tostring(L, -1);  (void)errorString;
			luaplus_assert(0);
		}
		return LPCD::Get(LPCD::TypeWrapper<RT>(), L, -1);
	}

	template <typename P1, typename P2, typename P3>
	RT operator()(P1 p1, P2 p2, P3 p3) {
		lua_State* L = functionObj.GetCState();
		LuaAutoBlock autoBlock(L);
		functionObj.Push();

		LPCD::Push(L, p1);
		LPCD::Push(L, p2);
		LPCD::Push(L, p3);

		if (lua_pcall(L, 3, 1, 0)) {
			const char* errorString = lua_tostring(L, -1);  (void)errorString;
			luaplus_assert(0);
		}
		return LPCD::Get(LPCD::TypeWrapper<RT>(), L, -1);
	}

	template <typename P1, typename P2, typename P3, typename P4>
	RT operator()(P1 p1, P2 p2, P3 p3, P4 p4) {
		lua_State* L = functionObj.GetCState();
		LuaAutoBlock autoBlock(L);
		functionObj.Push();

		LPCD::Push(L, p1);
		LPCD::Push(L, p2);
		LPCD::Push(L, p3);
		LPCD::Push(L, p4);

		if (lua_pcall(L, 4, 1, 0)) {
			const char* errorString = lua_tostring(L, -1);  (void)errorString;
			luaplus_assert(0);
		}
		return LPCD::Get(LPCD::TypeWrapper<RT>(), L, -1);
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5>
	RT operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) {
		lua_State* L = functionObj.GetCState();
		LuaAutoBlock autoBlock(L);
		functionObj.Push();

		LPCD::Push(L, p1);
		LPCD::Push(L, p2);
		LPCD::Push(L, p3);
		LPCD::Push(L, p4);
		LPCD::Push(L, p5);

		if (lua_pcall(L, 5, 1, 0)) {
			const char* errorString = lua_tostring(L, -1);  (void)errorString;
			luaplus_assert(0);
		}
		return LPCD::Get(LPCD::TypeWrapper<RT>(), L, -1);
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	RT operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6) {
		lua_State* L = functionObj.GetCState();
		LuaAutoBlock autoBlock(L);
		functionObj.Push();

		LPCD::Push(L, p1);
		LPCD::Push(L, p2);
		LPCD::Push(L, p3);
		LPCD::Push(L, p4);
		LPCD::Push(L, p5);
		LPCD::Push(L, p6);

		if (lua_pcall(L, 6, 1, 0)) {
			const char* errorString = lua_tostring(L, -1);  (void)errorString;
			luaplus_assert(0);
		}
		return LPCD::Get(LPCD::TypeWrapper<RT>(), L, -1);
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	RT operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7) {
		lua_State* L = functionObj.GetCState();
		LuaAutoBlock autoBlock(L);
		functionObj.Push();

		LPCD::Push(L, p1);
		LPCD::Push(L, p2);
		LPCD::Push(L, p3);
		LPCD::Push(L, p4);
		LPCD::Push(L, p5);
		LPCD::Push(L, p6);
		LPCD::Push(L, p7);

		if (lua_pcall(L, 7, 1, 0)) {
			const char* errorString = lua_tostring(L, -1);  (void)errorString;
			luaplus_assert(0);
		}
		return LPCD::Get(LPCD::TypeWrapper<RT>(), L, -1);
	}

protected:
	LuaObject functionObj;
};


/**
**/
class LuaFunctionVoid
{
public:
	LuaFunctionVoid(const LuaObject& functionObj)
		: functionObj(functionObj) {
	}

	LuaFunctionVoid(LuaState* state, const char* functionName) {
		functionObj = state->GetGlobals()[functionName];
	}

	void operator()() {
		lua_State* L = functionObj.GetCState();
		LuaAutoBlock autoBlock(L);
		functionObj.Push();

		if (lua_pcall(L, 0, 0, 0)) {
			const char* errorString = lua_tostring(L, -1);  (void)errorString;
			luaplus_assert(0);
		}
	}

	template <typename P1>
	void operator()(P1 p1) {
		lua_State* L = functionObj.GetCState();
		LuaAutoBlock autoBlock(L);
		functionObj.Push();

		LPCD::Push(L, p1);

		if (lua_pcall(L, 1, 0, 0)) {
			const char* errorString = lua_tostring(L, -1);  (void)errorString;
			luaplus_assert(0);
		}
	}

	template <typename P1, typename P2>
	void operator()(P1 p1, P2 p2) {
		lua_State* L = functionObj.GetCState();
		LuaAutoBlock autoBlock(L);
		functionObj.Push();

		LPCD::Push(L, p1);
		LPCD::Push(L, p2);

		if (lua_pcall(L, 2, 0, 0)) {
			const char* errorString = lua_tostring(L, -1);  (void)errorString;
			luaplus_assert(0);
		}
	}

	template <typename P1, typename P2, typename P3>
	void operator()(P1 p1, P2 p2, P3 p3) {
		lua_State* L = functionObj.GetCState();
		LuaAutoBlock autoBlock(L);
		functionObj.Push();

		LPCD::Push(L, p1);
		LPCD::Push(L, p2);
		LPCD::Push(L, p3);

		if (lua_pcall(L, 3, 0, 0)) {
			const char* errorString = lua_tostring(L, -1);  (void)errorString;
			luaplus_assert(0);
		}
	}

	template <typename P1, typename P2, typename P3, typename P4>
	void operator()(P1 p1, P2 p2, P3 p3, P4 p4) {
		lua_State* L = functionObj.GetCState();
		LuaAutoBlock autoBlock(L);
		functionObj.Push();

		LPCD::Push(L, p1);
		LPCD::Push(L, p2);
		LPCD::Push(L, p3);
		LPCD::Push(L, p4);

		if (lua_pcall(L, 4, 0, 0)) {
			const char* errorString = lua_tostring(L, -1);  (void)errorString;
			luaplus_assert(0);
		}
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5>
	void operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) {
		lua_State* L = functionObj.GetCState();
		LuaAutoBlock autoBlock(L);
		functionObj.Push();

		LPCD::Push(L, p1);
		LPCD::Push(L, p2);
		LPCD::Push(L, p3);
		LPCD::Push(L, p4);
		LPCD::Push(L, p5);

		if (lua_pcall(L, 5, 0, 0)) {
			const char* errorString = lua_tostring(L, -1);  (void)errorString;
			luaplus_assert(0);
		}
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	void operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6) {
		lua_State* L = functionObj.GetCState();
		LuaAutoBlock autoBlock(L);
		functionObj.Push();

		LPCD::Push(L, p1);
		LPCD::Push(L, p2);
		LPCD::Push(L, p3);
		LPCD::Push(L, p4);
		LPCD::Push(L, p5);
		LPCD::Push(L, p6);

		if (lua_pcall(L, 6, 0, 0)) {
			const char* errorString = lua_tostring(L, -1);  (void)errorString;
			luaplus_assert(0);
		}
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	void operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7) {
		lua_State* L = functionObj.GetCState();
		LuaAutoBlock autoBlock(L);
		functionObj.Push();

		LPCD::Push(L, p1);
		LPCD::Push(L, p2);
		LPCD::Push(L, p3);
		LPCD::Push(L, p4);
		LPCD::Push(L, p5);
		LPCD::Push(L, p6);
		LPCD::Push(L, p7);

		if (lua_pcall(L, 7, 0, 0)) {
			const char* errorString = lua_tostring(L, -1);  (void)errorString;
			luaplus_assert(0);
		}
	}

protected:
	LuaObject functionObj;
};

} // namespace LuaPlus

#endif // LUAPLUS_EXTENSIONS

#endif // LUAFUNCTION_H
