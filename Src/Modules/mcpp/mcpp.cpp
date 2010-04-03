#include "../../LuaPlus/LuaPlus.h"
extern "C" {
#include "mcpp/src/mcpp_lib.h"
}

using namespace LuaPlus;

static int LS_preprocess( LuaState* state )
{
	LuaObject argsObj(state, 1);
	if (!argsObj.IsTable())
		luaL_typerror(state->GetCState(), 1, "table");
	
	int argc = argsObj.GetCount();
	for (int i = 1; i <= argc; ++i)
	{
		if (!argsObj[i].IsString())
			luaL_argerror(state->GetCState(), 1, "expected a table of separate string arguments");
	}

	char** argv = new char*[argc + 1];
	argv[0] = NULL;
	for (int i = 1; i <= argc; ++i)
	{
		LuaObject argObj = argsObj[i];
		argv[i] = new char[argObj.StrLen() + 1];
		strcpy(argv[i], argObj.GetString());
	}

    mcpp_use_mem_buffers( 1);           /* enable memory output */
	int result = mcpp_lib_main(argc + 1, argv);
	state->PushInteger(result);

    state->PushString(mcpp_get_mem_buffer( OUT)); /* get the output       */
    state->PushString(mcpp_get_mem_buffer( ERR)); /* get the diagnostics  */

    mcpp_use_mem_buffers(1);           /* free the memory */
    mcpp_use_mem_buffers(0);           /* enable memory output */

	for (int i = 1; i < argc; ++i)
	{
		delete[] argv[i];
	}
	delete argv;

	return 3;
}


extern "C" LUAMODULE_API int luaopen_mcpp(lua_State* L)
{
	LuaState* state = LuaState::CastState(L);
	LuaObject mcppObj = state->GetGlobals().CreateTable( "mcpp" );
	mcppObj.Register( "preprocess", LS_preprocess );
	return 0;
}


