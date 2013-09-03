///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2010 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUASTATECD_H
#define LUASTATECD_H

#include "LuaPlusInternal.h"
#include "LuaPlusCD.h"

namespace LPCD
{
	using namespace LuaPlus;

	template<> struct Type<int (*)(LuaState*)> {
		static void Push(lua_State* L, int (*value)(LuaState*));
		static inline bool Match(lua_State* L, int idx)									{  return lua_type(L, idx) == LUA_TFUNCTION;  }
		static inline void* Get(lua_State* L, int /*idx*/)								{  return lua_State_to_LuaState(L);  }
	};

	inline int LuaStateFunctionDispatcher(lua_State* L) {
		typedef int (*Functor)(LuaPlus::LuaState*);
		unsigned char* buffer = LPCD::GetFirstUpvalueAsUserdata(L);
		Functor& func = *(Functor*)(buffer);
 		return (*func)(lua_State_to_LuaState(L));
	}

	template <typename Callee>
	class LuaStateMemberDispatcherHelper {
	public:
		static inline int LuaStateMemberDispatcher(lua_State* L) {
			typedef int (Callee::*Functor)(LuaPlus::LuaState*);
            unsigned char* buffer = LPCD::GetFirstUpvalueAsUserdata(L);
			Callee& callee = **(Callee**)buffer;
			Functor& func = *(Functor*)(buffer + sizeof(Callee*));
			return (callee.*func)(lua_State_to_LuaState(L));
		}
	};

	template <typename Callee>
	class Object_MemberDispatcher_to_LuaStateHelper {
	public:
		static inline int Object_MemberDispatcher_to_LuaState(lua_State* L) {
			typedef int (Callee::*Functor)(LuaPlus::LuaState*);
            unsigned char* buffer = GetFirstUpvalueAsUserdata(L);
			Functor& func = *(Functor*)(buffer);
			Callee& callee = *(Callee*)GetObjectUserdata(L);
			return (callee.*func)(lua_State_to_LuaState(L));
		}
	};
} // namespace LPCD

#endif // LUASTATECD_H
