#include "LuaPlus/LuaPlus.h"
using namespace LuaPlus;
#include <string>
#include "lstate.h"

namespace LPCD
{
	template<> struct Type<std::string> {
		static inline void Push(lua_State* L, const std::string& value)				{  lua_pushstring(L, value.c_str());  }
		static inline bool Match(lua_State* L, int idx)								{  return lua_type(L, idx) == LUA_TSTRING;  }
		static inline std::string Get(lua_State* L, int idx)						{  return lua_tostring(L, idx);  }
	};
	template<> struct Type<std::string&> : public Type<std::string> {};
	template<> struct Type<const std::string&> : public Type<std::string> {};
}


std::string TestStdString1(std::string str)
{
    printf("%s\n", str.c_str());
    return "Hello";
}



class VECTOR
{
public:
	double x,y,z;
};



class ENTITY {
public:
	virtual ~ENTITY() {
	}

	void printName() {
		printf("name: %s\n", name.c_str());
	}

	std::string name;
};


class POSITION_ENTITY : public ENTITY {
public:
	VECTOR position;
};


class MONSTER : public POSITION_ENTITY {
public:
	int alive;
	VECTOR attackPosition;

	void printMe() {
		printf("In printMe - alive:%d\n", alive);
	}
};


class HUMAN : public ENTITY {
public:
	int alive;

	void printMe() {
		printf("In printMe - alive:%d\n", alive);
	}
};

namespace LPCD {
	template<> struct Type<VECTOR> {
		static inline void Push(lua_State* L, const VECTOR& value) {
			LuaState* state = lua_State_to_LuaState(L);
			LuaObject obj = state->BoxPointer((void*)&value);
			obj.SetMetatable(state->GetRegistry()["VECTOR"]);
		}
		static inline bool Match(lua_State* L, int idx) {
			LuaState* state = lua_State_to_LuaState(L);
			LuaObject obj = state->Stack(idx);
			return obj.GetMetatable() == state->GetRegistry()["VECTOR"];
		}
		static inline VECTOR Get(lua_State* L, int idx) {
			LuaState* state = lua_State_to_LuaState(L);
			return *(VECTOR*)state->UnBoxPointer(idx);
		}
	};
	template<> struct Type<VECTOR&> : public Type<VECTOR> {};
	template<> struct Type<const VECTOR&> : public Type<VECTOR> {};
}


void PassVector(VECTOR& vec) {
	printf("Vector: %f, %f, %f\n", vec.x, vec.y, vec.z);
}


int MONSTER_new(LuaState* state){
	LuaObject monsterMetatableObj = state->GetRegistry()["MONSTER"];
	LuaObject monsterObj;
	monsterObj.AssignNewTable(state);
	MONSTER* monster = new MONSTER;
	monsterObj.SetLightUserdata("__object", monster);
	monsterObj.SetMetatable(monsterMetatableObj);
	monsterObj.Push(state);
	return 1;
}


MONSTER* get_MONSTER(const LuaObject& monsterObj) {
	return *(MONSTER**)lpcd_checkobject(monsterObj.GetCState(), monsterObj.GetRef(), "MONSTER");
}


using namespace LuaPlus;


int new_VECTOR2(lua_State* L) {
	VECTOR* vector = new VECTOR;
	vector->x = luaL_optnumber(L, 1, 0.0);
	vector->y = luaL_optnumber(L, 2, 0.0);
	vector->z = luaL_optnumber(L, 3, 0.0);
	*(void**)lua_newuserdata(L, sizeof(void*)) = vector;	// vector
	luaL_getmetatable(L, "VECTOR");			// vector VECTOR_metatable
	lua_setmetatable(L, -2);				// vector
	return 1;
}


int ENTITY___gc(lua_State* L) {
	ENTITY* entity = (ENTITY*)*(void**)lua_touserdata(L, 1);
	delete entity;
	return 0;
}


int new_MONSTER2(lua_State* L) {
	MONSTER* monster = new MONSTER;
	lua_newtable(L);										// monsterTable
	lua_pushstring(L, "__object");							// monsterTable "__object"
	*(void**)lua_newuserdata(L, sizeof(void*)) = monster;	// monsterTable "__object" monsterUD
	luaL_getmetatable(L, "MONSTER");						// monsterTable "__object" monsterUD MONSTER_metatable
	lua_pushvalue(L, -1);									// monsterTable "__object" monsterUD MONSTER_metatable MONSTER_metatable
	lua_setmetatable(L, -5);								// monsterTable "__object" monsterUD MONSTER_metatable
	lua_setmetatable(L, -2);								// monsterTable "__object" monsterUD
	lua_rawset(L, -3);										// monsterTable
	return 1;
}


MONSTER* get_MONSTER2(lua_State* L, int index, bool throwError = true) {
	void* ptr = lpcd_checkobject(L, index, "MONSTER", throwError);
	if (ptr)
		return *(MONSTER**)ptr;
	return NULL;
}



int ENTITY_inplace___gc(lua_State* L) {
	ENTITY* entity = (ENTITY*)*(void**)lua_touserdata(L, 1);
	entity->~ENTITY();
	return 0;
}


int new_MONSTER2_inplace(lua_State* L) {
	lua_newtable(L);										// monsterTable
	lua_pushstring(L, "__object");							// monsterTable "__object"
	MONSTER* monster = (MONSTER*)lua_newuserdata(L, sizeof(MONSTER));	// monsterTable "__object" monsterUD
	new(monster) MONSTER;
	luaL_getmetatable(L, "MONSTER_inplace");				// monsterTable "__object" monsterUD MONSTER_metatable
	lua_pushvalue(L, -1);									// monsterTable "__object" monsterUD MONSTER_metatable MONSTER_metatable
	lua_setmetatable(L, -5);								// monsterTable "__object" monsterUD MONSTER_metatable
	lua_setmetatable(L, -2);								// monsterTable "__object" monsterUD
	lua_rawset(L, -3);										// monsterTable
	return 1;
}


MONSTER* get_MONSTER2_inplace(lua_State* L, int index, bool throwError = true) {
	return (MONSTER*)lpcd_checkobject(L, index, "MONSTER_inplace", throwError);
}






void Vector4MonsterMetatableTest() {
	lua_State* L = luaL_newstate();

	LPCD::Class(L, "VECTOR")
		.Property("x", &VECTOR::x)
		.Property("y", &VECTOR::y)
		.Property("z", &VECTOR::z)
	;

	lua_register(L, "Vector", &new_VECTOR2);

	// ENTITY
	LPCD::Class(L, "ENTITY")
		.Property("position", &MONSTER::position)
		.Property("name", &MONSTER::name)
		.ObjectDirect("printName", (ENTITY*)0, &ENTITY::printName)
		.MetatableFunction("__gc", &ENTITY___gc)
	;

	// POSITION_ENTITY
	LPCD::Class(L, "POSITION_ENTITY", "ENTITY")
		.Property("position", &MONSTER::position)
		.MetatableFunction("__gc", &ENTITY___gc)
	;

	// MONSTER
	LPCD::Class(L, "MONSTER", "POSITION_ENTITY")
		.ObjectDirect("printMe", (MONSTER*)0, &MONSTER::printMe)
		.Property("alive", &MONSTER::alive)
		.MetatableFunction("__gc", &ENTITY___gc)
	;



	///////////////////////////////////////////
	lua_register(L, "Monster", &new_MONSTER2);

	lua_getglobal(L, "Monster1");
	MONSTER* monster0 = get_MONSTER2(L, -1, false);
	lua_pop(L, 1);

	luaL_dostring(L, "Monster1 = Monster()");
	lua_getglobal(L, "Monster1");
	MONSTER* monster = get_MONSTER2(L, -1);
	lua_pop(L, 1);
	luaL_dostring(L, "Monster1.alive = 1");
	luaL_dostring(L, "Monster1.position.x = 5");
	luaL_dostring(L, "Monster1.position.y = 10");
	luaL_dostring(L, "Monster1.position.z = 15");
	luaL_dostring(L, "Monster1.name = 'Joe'");
	luaL_dostring(L, "Monster1:printName()");
	luaL_dostring(L, "print(Monster1.position.x)");
	luaL_dostring(L, "Monster1:printMe()");
	luaL_dostring(L, "Monster1 = nil");
	lua_gc(L, LUA_GCCOLLECT, 0);

	luaL_dostring(L, "Monster2 = Monster()");
	lua_getglobal(L, "Monster2");
	MONSTER* monster2 = get_MONSTER2(L, -1);
	lua_pop(L, 1);
	luaL_dostring(L, "Monster2.alive = 0");
	luaL_dostring(L, "Monster2.position = Vector(25, 35, 45)");
	luaL_dostring(L, "Monster2.name = 'Jack'");
	luaL_dostring(L, "print(Monster2.position.x)");
	luaL_dostring(L, "Monster2:printMe()");

	lpcd_pushdirectclosure(L, &PassVector);
	lua_setglobal(L, "PassVector");

	luaL_dostring(L, "PassVector(Monster1.position)");







	// ENTITY_inplace
	LPCD::InPlaceClass(L, "ENTITY_inplace")
		.Property("position", &MONSTER::position)
		.Property("name", &MONSTER::name)
		.ObjectDirect("printName", (ENTITY*)0, &ENTITY::printName)
		.MetatableFunction("__gc", &ENTITY_inplace___gc)
	;

	// POSITION_ENTITY_inplace
	LPCD::InPlaceClass(L, "POSITION_ENTITY_inplace", "ENTITY_inplace")
		.Property("position", &MONSTER::position)
		.MetatableFunction("__gc", &ENTITY_inplace___gc)
	;

	// MONSTER_inplace
	LPCD::InPlaceClass(L, "MONSTER_inplace", "POSITION_ENTITY_inplace")
		.ObjectDirect("printMe", (MONSTER*)0, &MONSTER::printMe)
		.Property("alive", &MONSTER::alive)
		.MetatableFunction("__gc", &ENTITY_inplace___gc)
	;

	///////////////////////////////////////////
	lua_register(L, "MonsterInPlace", &new_MONSTER2_inplace);

	luaL_dostring(L, "Monster1InPlace = MonsterInPlace()");
	lua_getglobal(L, "Monster1InPlace");
	MONSTER* monsterInPlace = get_MONSTER2_inplace(L, -1);
	lua_pop(L, 1);
	lua_getglobal(L, "Monster1InPlace");
	lua_pushstring(L, "alive");
	lua_pushnumber(L, 0);
	lua_settable(L, -3);
	luaL_dostring(L, "Monster1InPlace.alive = 1");
	luaL_dostring(L, "Monster1InPlace.position.x = 5");
	luaL_dostring(L, "Monster1InPlace.position.y = 10");
	luaL_dostring(L, "Monster1InPlace.position.z = 15");
	luaL_dostring(L, "Monster1InPlace.name = 'JoeInPlace'");
	luaL_dostring(L, "Monster1InPlace:printName()");
	luaL_dostring(L, "print(Monster1InPlace.position.x)");
	luaL_dostring(L, "Monster1InPlace:printMe()");

}


void Vector3MonsterMetatableTest() {
	lua_State* L = luaL_newstate();

	lpcd_newclassmetatable(L, "VECTOR", NULL);
	lpcd_propertycreate(L, -1, "x", &VECTOR::x);
	lpcd_propertycreate(L, -1, "y", &VECTOR::y);
	lpcd_propertycreate(L, -1, "z", &VECTOR::z);
	lua_pop(L, 1);

	lua_register(L, "Vector", &new_VECTOR2);

	// ENTITY
	lpcd_newclassmetatable(L, "ENTITY", NULL);		// ENTITY_metatable

	lpcd_propertymetatable_getfunctions(L, -1);		// ENTITY_metatable ENTITY_functions
	lpcd_pushobjectdirectclosure(L, (ENTITY*)0, &ENTITY::printName);
	lua_setfield(L, -2, "printName");
	lua_pop(L, 1);									// ENTITY_metatable

	lpcd_propertycreate(L, -1, "position", &MONSTER::position);
	lpcd_propertycreate(L, -1, "name", &MONSTER::name);

	lua_pop(L, 1);


	// POSITION_ENTITY
	lpcd_newclassmetatable(L, "POSITION_ENTITY", "ENTITY");		// POSITION_ENTITY_metatable
	lpcd_propertycreate(L, -1, "position", &MONSTER::position);
	lua_pop(L, 1);


	// MONSTER
	lpcd_newclassmetatable(L, "MONSTER", "POSITION_ENTITY");	// MONSTER_metatable

	lpcd_propertymetatable_getfunctions(L, -1);	// MONSTER_metatable MONSTER_functions
	lpcd_pushobjectdirectclosure(L, (MONSTER*)0, &MONSTER::printMe);
	lua_setfield(L, -2, "printMe");
	lua_pop(L, 1);								// MONSTER_metatable

	lpcd_propertycreate(L, -1, "alive", &MONSTER::alive);

	///////////////////////////////////////////
	lua_register(L, "Monster", &new_MONSTER2);

	lua_getglobal(L, "Monster1");
	MONSTER* monster0 = get_MONSTER2(L, -1, false);
	lua_pop(L, 1);

	luaL_dostring(L, "Monster1 = Monster()");
	lua_getglobal(L, "Monster1");
	MONSTER* monster = get_MONSTER2(L, -1);
	lua_pop(L, 1);
	luaL_dostring(L, "Monster1.alive = 1");
	luaL_dostring(L, "Monster1.position.x = 5");
	luaL_dostring(L, "Monster1.position.y = 10");
	luaL_dostring(L, "Monster1.position.z = 15");
	luaL_dostring(L, "Monster1.name = 'Joe'");
	luaL_dostring(L, "Monster1:printName()");
	luaL_dostring(L, "print(Monster1.position.x)");
	luaL_dostring(L, "Monster1:printMe()");

	luaL_dostring(L, "Monster2 = Monster()");
	lua_getglobal(L, "Monster2");
	MONSTER* monster2 = get_MONSTER2(L, -1);
	lua_pop(L, 1);
	luaL_dostring(L, "Monster2.alive = 0");
	luaL_dostring(L, "Monster2.position = Vector(25, 35, 45)");
	luaL_dostring(L, "Monster2.name = 'Jack'");
	luaL_dostring(L, "print(Monster2.position.x)");
	luaL_dostring(L, "Monster2:printMe()");

	lpcd_pushdirectclosure(L, &PassVector);
	lua_setglobal(L, "PassVector");

	luaL_dostring(L, "PassVector(Monster1.position)");
}


void Vector2MonsterMetatableTest() {
	lua_State* L = luaL_newstate();

	luaL_newmetatable(L, "VECTOR");				// vector_metatable
	lpcd_integratepropertysupport(L, -1);
	lpcd_propertycreate(L, -1, "x", &VECTOR::x);
	lpcd_propertycreate(L, -1, "y", &VECTOR::y);
	lpcd_propertycreate(L, -1, "z", &VECTOR::z);
	lua_pop(L, 1);

	lua_register(L, "Vector", &new_VECTOR2);

	// ENTITY
	luaL_newmetatable(L, "ENTITY");			// ENTITY_metatable
	lpcd_integratepropertysupport(L, -1);

	lpcd_propertymetatable_getfunctions(L, -1);	// ENTITY_metatable ENTITY_functions
	lpcd_pushobjectdirectclosure(L, (ENTITY*)0, &ENTITY::printName);
	lua_setfield(L, -2, "printName");
	lua_pop(L, 1);								// ENTITY_metatable

	lpcd_propertycreate(L, -1, "position", &MONSTER::position);
	lpcd_propertycreate(L, -1, "name", &MONSTER::name);
//	lua_pop(L, 1);

	// POSITION_ENTITY
	luaL_newmetatable(L, "POSITION_ENTITY");	// POSITION_ENTITY_metatable
	lpcd_integratepropertysupport(L, -1);

	lpcd_propertycreate(L, -1, "position", &MONSTER::position);
//	lua_pop(L, 1);

	lua_pushvalue(L, -2);						// ENTITY_metatable POSITION_ENTITY_metatable ENTITY_metatable
	lua_setmetatable(L, -2);					// ENTITY_metatable POSITION_ENTITY_metatable


	// MONSTER
	luaL_newmetatable(L, "MONSTER");			// MONSTER_metatable
	lpcd_integratepropertysupport(L, -1);

	lpcd_propertymetatable_getfunctions(L, -1);	// MONSTER_metatable MONSTER_functions
	lpcd_pushobjectdirectclosure(L, (MONSTER*)0, &MONSTER::printMe);
	lua_setfield(L, -2, "printMe");
	lua_pop(L, 1);								// MONSTER_metatable

	lpcd_propertycreate(L, -1, "alive", &MONSTER::alive);

	lua_pushvalue(L, -2);						// ENTITY_metatable MONSTER_metatable ENTITY_metatable
	lua_setmetatable(L, -2);					// ENTITY_metatable MONSTER_metatable

	lua_pop(L, 1);

	///////////////////////////////////////////
	lua_register(L, "Monster", &new_MONSTER2);

	lua_getglobal(L, "Monster1");
	MONSTER* monster0 = get_MONSTER2(L, -1, false);
	lua_pop(L, 1);

	luaL_dostring(L, "Monster1 = Monster()");
	lua_getglobal(L, "Monster1");
	MONSTER* monster = get_MONSTER2(L, -1);
	lua_pop(L, 1);
	luaL_dostring(L, "Monster1.alive = 1");
	luaL_dostring(L, "Monster1.position.x = 5");
	luaL_dostring(L, "Monster1.position.y = 10");
	luaL_dostring(L, "Monster1.position.z = 15");
	luaL_dostring(L, "Monster1.name = 'Joe'");
	luaL_dostring(L, "Monster1:printName()");
	luaL_dostring(L, "print(Monster1.position.x)");
	luaL_dostring(L, "Monster1:printMe()");

	luaL_dostring(L, "Monster2 = Monster()");
	lua_getglobal(L, "Monster2");
	MONSTER* monster2 = get_MONSTER2(L, -1);
	lua_pop(L, 1);
	luaL_dostring(L, "Monster2.alive = 0");
	luaL_dostring(L, "Monster2.position = Vector(25, 35, 45)");
	luaL_dostring(L, "Monster2.name = 'Jack'");
	luaL_dostring(L, "print(Monster2.position.x)");
	luaL_dostring(L, "Monster2:printMe()");

	lpcd_pushdirectclosure(L, &PassVector);
	lua_setglobal(L, "PassVector");

	luaL_dostring(L, "PassVector(Monster1.position)");
}




void VectorMonsterMetatableTest()
{
	LuaStateOwner state(true);

	LuaObject vectorMetatableObj = state->GetRegistry().CreateTable("VECTOR");
	LPCD::Metatable_IntegratePropertySupport(vectorMetatableObj);

	LPCD::PropertyCreate(vectorMetatableObj, "x", &VECTOR::x);
	LPCD::PropertyCreate(vectorMetatableObj, "y", &VECTOR::y);
	LPCD::PropertyCreate(vectorMetatableObj, "z", &VECTOR::z);

	LuaObject monsterMetatableObj = state->GetRegistry().CreateTable("MONSTER");

	LPCD::Metatable_IntegratePropertySupport(monsterMetatableObj);

	LuaObject monsterMetatableFunctionsObj = LPCD::PropertyMetatable_GetFunctions(monsterMetatableObj);
	monsterMetatableFunctionsObj.RegisterObjectDirect("printMe", (MONSTER*)0, &MONSTER::printMe);

	LPCD::PropertyCreate(monsterMetatableObj, "alive", &MONSTER::alive);
	LPCD::PropertyCreate(monsterMetatableObj, "position", &MONSTER::position);
	LPCD::PropertyCreate(monsterMetatableObj, "name", &MONSTER::name);

	state->GetGlobals().Register("Monster", &MONSTER_new);

	state->DoString("Monster1 = Monster()");
	LuaObject monsterObj = state->GetGlobals()["Monster1"];
	MONSTER* monster = get_MONSTER(monsterObj);
	state->DoString("Monster1.alive = 1");
	state->DoString("Monster1.position.x = 5");
	state->DoString("Monster1.position.y = 10");
	state->DoString("Monster1.position.z = 15");
	state->DoString("Monster1.name = 'Joe'");
	state->DoString("print(Monster1.position.x)");
	state->DoString("Monster1:printMe()");

	state->DoString("Monster2 = Monster()");
	LuaObject monster2Obj = state->GetGlobals()["Monster2"];
	MONSTER* monster2 = get_MONSTER(monster2Obj);
	state->DoString("Monster2.alive = 0");
	state->DoString("Monster2.position.x = 25");
	state->DoString("Monster2.position.y = 35");
	state->DoString("Monster2.position.z = 45");
	state->DoString("Monster2.name = 'Jack'");
	state->DoString("print(Monster2.position.x)");
	state->DoString("Monster2:printMe()");

	state->GetGlobals().RegisterDirect("PassVector", &PassVector);
	state->DoString("PassVector(Monster1.position)");
}


int __cdecl main(int argc, char* argv[])
{
	Vector4MonsterMetatableTest();
	Vector3MonsterMetatableTest();
	Vector2MonsterMetatableTest();
	VectorMonsterMetatableTest();
	return 0;
}


