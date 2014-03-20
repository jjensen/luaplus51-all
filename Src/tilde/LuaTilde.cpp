#include "LuaTilde.h"

#include "LuaHostWindows.h"
#include <string.h>

using namespace tilde;

#if !defined(_MSC_VER)
#define stricmp strcasecmp
#endif

LuaTildeHost* LuaTilde_Command(LuaTildeHost* host, const char* command, void* param1, void* param2) {
    if (!command)
        return NULL;

    if (stricmp(command, "poll") == 0) {
        if (!host)
            return NULL;
        ((LuaHostWindows*)host)->Poll();
        return host;
    }
    if (stricmp(command, "isconnected") == 0) {
        if (!host)
            return NULL;
        if (!param1)
            return NULL;
        *(int*)param1 = ((LuaHostWindows*)host)->IsConnected() ? 1 : 0;
        return host;
    }
    if (stricmp(command, "waitfordebuggerconnection") == 0) {
        if (!host)
            return NULL;
        ((LuaHostWindows*)host)->WaitForDebuggerConnection();
        return host;
    }
    if (stricmp(command, "registerstate") == 0) {
        if (!host  ||  !param1  ||  !param2)
            return NULL;
        ((LuaHostWindows*)host)->RegisterState((const char*)param1, (lua_State*)param2);
        return host;
    }
    if (stricmp(command, "create") == 0) {
        return (LuaTildeHost*)new LuaHostWindows((intptr_t)param1);
    }
    if (stricmp(command, "destroy") == 0) {
        delete (LuaHostWindows*)host;
        return NULL;
    }

    return NULL;
}
