#ifndef LUAINPYTHON_H
#define LUAINPYTHON_H

typedef struct {
	PyObject_HEAD
	int ref;
	int refiter;
} LuaObject;

extern PyTypeObject LuaObject_Type;

#define LuaObject_Check(op) PyObject_TypeCheck(op, &LuaObject_Type)

PyObject *LuaConvert(lua_State *L, int n);

extern lua_State *LuaState;

DL_EXPORT(void) initlua(void);

#endif
