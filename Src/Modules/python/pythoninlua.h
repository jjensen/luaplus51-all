#ifndef PYTHONINLUA_H
#define PYTHONINLUA_H

#define POBJECT "POBJECT"

int py_convert(lua_State *L, PyObject *o, int withnone);

typedef struct {
	PyObject *o;
	int asindx;
} py_object;

LUAMODULE_API int luaopen_python(lua_State *L);

#endif
