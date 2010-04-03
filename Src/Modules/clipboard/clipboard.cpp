#include <windows.h>
#include "LuaPlus/LuaPlus.h"

using namespace LuaPlus;

static int LS_GetText( LuaState* state )
{
	if (OpenClipboard(NULL))
	{
		HANDLE hData = ::GetClipboardData(CF_TEXT);
		if (hData)
		{
			char* data = (char*)::GlobalLock(hData);
			state->PushString(data);

			::GlobalUnlock(hData);
		}
		else
		{
			state->PushString("");
		}

		CloseClipboard();
	}
	else
	{
		state->PushString("");
	}

	return 1;
}


static int LS_PutText( LuaState* state )
{
	const char* str = luaL_checkstring(state->GetCState(), 1);

	if (OpenClipboard(NULL))
	{
		EmptyClipboard();

		size_t len = strlen(str) + 1;
        HANDLE hGlobalMemory = GlobalAlloc(GHND, len);
        if (hGlobalMemory)
        {
            LPSTR lpGlobalMemory = (LPSTR)GlobalLock(hGlobalMemory);

            memcpy(lpGlobalMemory, str, len);

            GlobalUnlock(hGlobalMemory);
        }

        SetClipboardData(CF_TEXT, hGlobalMemory);

		CloseClipboard();
	}

	return 0;
}


extern "C" LUAMODULE_API int luaopen_clipboard(lua_State* L)
{
	LuaState* state = LuaState::CastState(L);
	LuaObject obj = state->GetGlobals().CreateTable( "clipboard" );
	obj.Register( "gettext", LS_GetText );
	obj.Register( "puttext", LS_PutText );
	return 0;
}


