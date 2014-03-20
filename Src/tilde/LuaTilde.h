#ifndef LUATILDE_H
#define LUATILDE_H

#include <lua.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* LuaTildeHost;

#if defined(WIN32)  ||  defined(_WIN64)
__declspec(dllexport)
#endif
LuaTildeHost* LuaTilde_Command(LuaTildeHost* host, const char* command, void* param1, void* param2);

#ifdef __cplusplus
}
#endif

#endif /* LUATILDE_H */

