#include "LuaPlus/LuaPlus.h"
using namespace LuaPlus;
#include "SimpleHeap.h"
#include "Timer.h"
#include <math.h>
#include <assert.h>
#include <string>
#include <time.h>
#include <crtdbg.h>
#include <vector>

class Foo
{
public:
	int check;
	Foo() { check = 0x12345678; }
	void bar() { assert(check == 0x12345678); }
};

void NewUserdataBoxTest()
{
	Foo f;

	LuaStateOwner lua;

	LuaObject obj = lua->NewUserdataBox(&f);
	lua->CreateTable();
    LuaObject meta = lua->StackTop();
	meta.RegisterObjectDirect("bar", (Foo*)0, &Foo::bar);
	obj.SetMetatable(meta);

	lua->GetGlobals().Set("f", obj);

	f.bar();   // OK
	lua->DoString("f:bar()");   // ASSERT
};

	
namespace LPCD
{
	template<> struct Type<std::string> {
		static inline void Push(lua_State* L, const std::string& value)						{  lua_pushstring(L, value.c_str());  }
		static inline bool Match(lua_State* L, int idx)										{  return lua_type(L, idx) == LUA_TSTRING;  }
		static inline std::string Get(lua_State* L, int idx)								{  return static_cast<const char*>(lua_tostring(L, idx));  }
	};
	template<> struct Type<std::string&> : public Type<std::string> {};
	template<> struct Type<const std::string&> : public Type<std::string> {};
}


std::string TestStdString1(std::string str)
{
    printf("%s\n", str.c_str());
    return "Hello";
}


using namespace LuaPlus;

void TestStdString()
{
    LuaStateOwner state;

	LuaObject globalsObj = state->GetGlobals();
	globalsObj.RegisterDirect("TestStdString1", TestStdString1);
    LuaFunction<std::string> funcCall(state, "TestStdString1");
    std::string value = funcCall("World");
    printf("%s\n", value.c_str());
}


/**
	The LuaState class is able to make Lua callbacks look like natural extensions
	of the LuaState class.
**/
static int LS_PrintNumber(LuaState* state)
{
	LuaStack args(state);

	// Verify it is a number and print it.
	if (args[1].IsNumber())
		printf("%f\n", args[1].GetNumber());

	// No return values.
	return 0;
}


/**
**/
class ObjectWrapper
{
public:
	void Zero()
	{
	}

	void Print(const char* str)
	{
		printf("%s\n", str);
	}

	float Add(float num1, float num2)
	{
		return num1 + num2;
	}

	float Add3(float num1, float num2, float num3)
	{
		return num1 + num2 + num3;
	}

	float Add4(float num1, float num2, float num3, float num4)
	{
		return num1 + num2 + num3 + num4;
	}

	int LS_Add(LuaState* state)
	{
		LuaStack args(state);

		// Verify it is a number and print it.
		if (args[1].IsNumber()  &&  args[2].IsNumber())
		{
			state->PushNumber(args[1].GetNumber() + args[2].GetNumber() );
		}
		else
		{
			state->PushNumber( 0.0 );
		}

		// 1 return value.
		return 1;
	}

	char buffer[1000];
};


void Add4NoReturn(float num1, float num2, float num3, float num4)
{
}

/**
**/
void DoScriptFormatTest()
{
	LuaStateOwner state;
	state->DoFile("ScriptVectorDump.lua");
}


int Test(int num)
{
	num;
	return 5;
}


/**
	Demonstrate registering callback functions for the Lua script.
**/
void DoScriptCallbackTest()
{
/*	LuaStateOwner state;

	state->GetGlobals().Register("PrintNumber", LS_PrintNumber);

	ObjectWrapper obj;
	state->GetGlobals().Register("Add", obj, ObjectWrapper::LS_Add);

	state->DoFile("ScriptCallbackTest.lua");
*/

	LuaStateOwner state;
/*
	state->GetGlobals().Register("PrintNumber", LS_PrintNumber);

	ObjectWrapper obj;
	state->GetGlobals().Register("Add", obj, ObjectWrapper::LS_Add);

	state->DoFile("ScriptCallbackTest.lua");

	// Now call it ourselves.
	LuaFunction<int> function(state, "PrintNumber");
	function(22);

	LuaFunction<float> add(state, "Add");
	float ret = add(22, 11);
*/
	LuaObject globalsObj = state->GetGlobals();

	LuaObject testTableObj = globalsObj.CreateTable("TestTable");
	testTableObj.Register("PrintNumber", LS_PrintNumber);
	state->DoString("TestTable.PrintNumber(1000)");

	// Test auto thing.
	int top = state->GetTop();
	globalsObj.RegisterDirect("Test", Test);
	int top2 = state->GetTop();
	globalsObj.RegisterDirect("sin", (double (*)(double))&sin);
	LuaFunction<int> funcCall(state, "Test");
	int value = funcCall(10);

	LuaFunction<float> funcCall2(state, "sin");
	float value2 = funcCall2(5);

	ObjectWrapper obj;
	globalsObj.RegisterDirect("Add", obj, &ObjectWrapper::Add);
	globalsObj.RegisterDirect("Add3", obj, &ObjectWrapper::Add3);
	globalsObj.RegisterDirect("Add4", obj, &ObjectWrapper::Add4);
	globalsObj.RegisterDirect("Add4NoReturn", Add4NoReturn);
	globalsObj.RegisterDirect("Zero", obj, &ObjectWrapper::Zero);
	globalsObj.RegisterDirect("Print", obj, &ObjectWrapper::Print);
	LuaFunction<double> addCall(state, "Add");
	LuaFunctionVoid printCall(state, "Print");
	LuaFunctionVoid zeroCall(state, "Zero");
	zeroCall();
	printCall("Stuff");
	double result = addCall(10, 2);

}


/**
	Demonstrate reading and saving a script.
**/
void DoScriptSaveTest()
{
	LuaStateOwner state;

	try
	{
		state->DoFile( "ScriptSaveTest.lua" );
	}
	catch (LuaException&)
	{
	}

//	state->DumpGlobalsFile("ScriptSaveTest.dmp");
}


/**
	Demonstrates walking an array table.
**/
void DoScriptArrayTest()
{
	LuaStateOwner state;
	if (state->DoFile("ScriptArrayTest.lua") != 0)
	{
		printf("%s\n", LuaStackObject(state, -1).GetString());
		printf("Unable to read ScriptArrayTest.lua.\n");
	}
	LuaObject testTableObj = state->GetGlobals()[ "TestArray" ];
	for (int i = 1; ; ++i)
	{
		LuaObject entryObj = testTableObj[ i ];
		if (entryObj.IsNil())
			break;
		if (entryObj.IsNumber())
			printf("%f\n", entryObj.GetNumber());
		else if (entryObj.IsString())
			printf("%s\n", entryObj.GetString());
	}
}


static int LS_LightUserdataCall( LuaState* state )
{
	LuaStack args(state);
	bool isLightUserdata = args[ 1 ].IsLightUserdata();
	const void* ptr = args[ 1 ].GetUserdata();
    state->PushNumber(10000);
	return 1;
}


void TestPointer()
{
	LuaStateOwner state;

	state->GetGlobals().Register("LightUserdataCall", LS_LightUserdataCall);
	LuaObject funcObj = state->GetGlobal("LightUserdataCall");
	LuaCall call = funcObj;
	LuaStackObject retObj = call << (void*)0xfedcba98 << LuaRun();
    printf("%f\n", retObj.GetNumber());
}


/*void LuaStackTableIteratorTest()
{
	LuaStateOwner state;

	state->DoString( "Table = { Hi = 5, Hello = 10, Yo = 6 }" );

	int origTop = state->GetTop();

	LuaStackObject obj = state->GetGlobal("Table");

	for ( LuaStackTableIterator it( obj ); it; it.Next() )
	{
		const char* key = it.GetKey().GetString();
		int num = it.GetValue().GetInteger();
	}

	LuaStackObject obj2 = state->GetGlobal("Table");

	for ( it.Reset(); it; ++it )
	{
		const char* key = it.GetKey().GetString();
	}
}
*/

void LuaTableIteratorTest()
{
	LuaStateOwner state;

	state->DoString( "Table = { Hi = 5, Hello = 10, Yo = 6 }" );

	int origTop = state->GetTop();

	LuaObject obj = state->GetGlobal("Table");

	for ( LuaTableIterator it( obj ); it; it.Next() )
	{
		const char* key = it.GetKey().GetString();
		lua_Integer num = it.GetValue().GetInteger();
	}
}


void TestNewCall()
{
	LuaStateOwner state;

	state->DoString("function Add(x, y) return x + y end");
	LuaObject funcObj = state->GetGlobal("Add");
	int top = state->GetTop();
	LuaCall call = funcObj;
	LuaObject retObj = call << 2 << 7 << LuaRun();
	int top2 = state->GetTop();
//	funcObj() << LuaRun();
}

using namespace SimpleMem;

SimpleHeap* heap;

#if 0  &&  LUA_MEMORY_STATS
void* ReallocFunction(void* ud, void* ptr, size_t osize, size_t nsize, const char* allocName, unsigned int flags)
#else
void* ReallocFunction(void* ud, void* ptr, size_t osize, size_t nsize)
#endif /* LUA_MEMORY_STATS */
{
	SimpleHeap* heap = (SimpleHeap*)ud;

	if (nsize == 0)
	{
		heap->Free(ptr);
		return NULL;
	}

#if 0  &&  LUA_MEMORY_STATS
	if (flags == LUA_ALLOC_TEMP)
	{
		return heap->Realloc(ptr, nsize, SimpleHeap::FIRSTFIT_TOP,
			allocName, __FILE__, __LINE__);
	}

	return heap->Realloc(ptr, nsize, SimpleHeap::FIRSTFIT_BOTTOM,
		allocName, __FILE__, __LINE__);
#else
	return heap->Realloc(ptr, nsize, SimpleHeap::FIRSTFIT_BOTTOM,
		"", __FILE__, __LINE__);
#endif /* LUA_MEMORY_STATS */
}

int checkpoint = 0;

class MemLogFile
{
public:
	MemLogFile()
	{
		Create("MemoryDump.txt");
	}

	~MemLogFile()
	{
		Close();
	}

	void Close()
	{
		if (m_file)
		{
			fclose(m_file);
			m_file = NULL;
		}
	}

	void Create(const char* fileName)
	{
		Close();

		strcpy(m_fileName, fileName);
		m_file = fopen(m_fileName, "wt");
		assert(m_file);
		Write("Heap Name\tBlock Type\tAddress\tRequested Size\tTotal Size\tIndex\tName\tFile Name\tLine Number\n");
	}

	void Open()
	{
		Close();
		m_file = fopen(m_fileName, "at");
		assert(m_file);
	}

	void Write(const char* msg, ...)
	{
		va_list args;
		va_start(args, msg);
		vfprintf(m_file, msg, args);
		va_end(args);
	}

	void Flush()
	{
		fflush(m_file);
	}

	operator FILE*()				{  return m_file;  }

protected:
	FILE* m_file;
	char m_fileName[_MAX_PATH];
};


MemLogFile& GetMemLogFile()
{
	static MemLogFile autoFile;
	return autoFile;
}


void DumpIt()
{
	heap->HeapDump(GetMemLogFile(), checkpoint, checkpoint);
}


int Chunkwriter(lua_State *L, const void* p,
                size_t sz, void* ud)
{
	FILE* file = (FILE*)ud;
	fwrite(p, sz, 1, file);
	return sz;
}


void TestHeap()
{
	SimpleHeap mainHeap("Test", false);
	mainHeap.Initialize(1 * 1024 * 1024);
	heap = &mainHeap;

	GetMemLogFile().Create("dump.txt");

	heap->SetAlignment(1);

	heap->Alloc(1);
	DumpIt();
	void* p0 = heap->Alloc(1);
	DumpIt();
	void* p1 = heap->Alloc(1);
	DumpIt();
	heap->Free(p0);
	DumpIt();
	heap->Free(p1);
	DumpIt();

	heap->Alloc(100);
	DumpIt();
	heap->Alloc(100);
	DumpIt();
	heap->Alloc(100);
	DumpIt();
	heap->Alloc(100, SimpleHeap::FIRSTFIT_TOP);
	DumpIt();
	heap->Alloc(100, SimpleHeap::FIRSTFIT_TOP);
	DumpIt();
	heap->Free(p1);
	DumpIt();
	heap->Alloc(50);
	DumpIt();
	heap->Alloc(50);
	DumpIt();
	heap->Alloc(1047300);
	DumpIt();
	heap->Alloc(2, SimpleHeap::FIRSTFIT_TOP);
	DumpIt();
}


void TestGCObject()
{
	TestHeap();
	SimpleHeap mainHeap("Test", false);
	mainHeap.Initialize(1 * 1024 * 1024);
	heap = &mainHeap;

	GetMemLogFile().Create("dump.txt");

	LuaStateOwner state(ReallocFunction, &mainHeap);
	state->GC(LUA_GCCOLLECT, 0);

	heap->LogStats();
	DumpIt();

//	checkpoint = heap->SetCheckpoint();

/*	{
		LuaObject stringObj(state);
		stringObj.Assign("Hello, world!");
		LuaObject globalsObj = state->GetGlobals();
	}

	state->CollectGarbage();
*/
//	state->DoString("Table = { 0, 1, 2, 'Hello', nil, 'Hi', Yo = 'My Stuff', NobodysHome = 5, NestedTable = { 1, 2, 3, { 'String', }, { 'Table2' } }, { 'String1' } }");
//	state->DoString("a = 5");
/*	FILE* inFile = fopen("dump.dat", "rb");
	fseek(inFile, 0, SEEK_END);
	int size = ftell(inFile);
	fseek(inFile, 0, SEEK_SET);
	BYTE* buf = new BYTE[size];
	fread(buf, size, 1, inFile);
	fclose(inFile);

	state->DoBuffer((const char*)buf, size, NULL);
	state->CollectGarbage();
*/
	LPCSTR strbuf = "a = 5";
//	state->CheckStack(1);
//	state->LoadBuffer(strbuf, strlen(strbuf), "Stuff");
	state->DoString(strbuf);
/*  FILE* file = fopen("dump.dat", "wb");
	state->CheckStack(1);
	state->Dump(Chunkwriter, file);
	fclose(file);*/
	state->GC(LUA_GCCOLLECT, 0);

	{
		LuaObject obj;

	}
}


void SetUserdata()
{
	SimpleHeap mainHeap("Test", false);
	mainHeap.Initialize(1 * 1024 * 1024);
	heap = &mainHeap;

	GetMemLogFile().Create("dump.txt");

	{
		LuaStateOwner state(ReallocFunction, &mainHeap);
		{
			LuaObject obj(state);
			obj.AssignNewTable(state);
			assert(obj.IsTable());
		}
		state->GC(LUA_GCCOLLECT, 0);
		DumpIt();
	}
}


void CloneTest()
{
	SimpleHeap mainHeap("Test", false);
	mainHeap.Initialize(1 * 1024 * 1024);
	heap = &mainHeap;

	GetMemLogFile().Create("dump.txt");

	LuaStateOwner state(ReallocFunction, &mainHeap);

	LuaObject valueObj(state);
	valueObj.Assign(state, true);

	LuaObject cloneObj = valueObj.Clone();

	Timer timer;
	timer.Start();
	state->DoString("Table = { 0, 1, 2, 'Hello', nil, 'Hi', Yo = 'My Stuff', NobodysHome = 5, NestedTable = { 1, 2, 3, { 'String', }, { 'Table2' } }, { 'String1' } }");
	timer.Stop();
	printf("DoString: %f\n", timer.GetMillisecs());

	LuaObject tableObj = state->GetGlobal("Table");
	timer.Reset();
	timer.Start();
	LuaObject clonedTableObj = tableObj.Clone();
	timer.Stop();
	printf("Clone: %f\n", timer.GetMillisecs());

	clonedTableObj.SetNil("Yo");

#if LUAPLUS_DUMPOBJECT
	state->DumpObject("test1.lua", "Table", tableObj, false);
	state->DumpObject("test2.lua", "Table", clonedTableObj, false);
#endif // LUAPLUS_DUMPOBJECT
}


int CallbackA(LuaState* state)
{
	for (int i = 0; i < 10; ++i)
	{
		printf("Hello%d\n", i);
//		state->CoYield(0);
	}

	return 0;
}


int CallbackB(LuaState* state)
{
	for (int i = 0; i < 15; ++i)
	{
		printf("Hi%d\n", i);
//		state->CoYield(0);
	}

	return 0;
}


void TestThreeThreads()
{
	LuaStateOwner state;
	LuaObject thread1(LuaState::CreateThread(state));
	LuaObject thread2(LuaState::CreateThread(state));

	LuaObject obj(thread1);
	obj.AssignNewTable(state);

	LuaObject obj2(thread2);
	obj2.Assign(state, "Hi");

#if LUA_WIDESTRING
	LuaObject obj3(thread2);
	obj3.Assign(state, (const lua_WChar*)L"Hello");
#endif /* LUA_WIDESTRING */

	state->GetGlobals().Register("CallbackA", CallbackA);
	state->GetGlobals().Register("CallbackB", CallbackB);

	LuaFunctionVoid callbackA = thread1.GetState()->GetGlobal("CallbackA");
	LuaFunctionVoid callbackB = thread2.GetState()->GetGlobal("CallbackB");

	callbackA();
	callbackB();

	state->GC(LUA_GCCOLLECT, 0);
}


void DumpTest()
{
	LuaStateOwner state(false);
	LuaObject complexObj = state->GetGlobals().CreateTable("Complex");
	complexObj.Set("d:\\Test\\Stuff\\\xff\xfe", "An entry");
#if LUAPLUS_DUMPOBJECT
	state->DumpObject("test.lua", "Complex", complexObj);
#endif // LUAPLUS_DUMPOBJECT
}


#if LUAPLUS_EXTENSIONS  &&  LUA_VERSION_NUM <= 501

void TestANSIFile()
{
	LuaStateOwner state(true);
	int ret = state->DoFile("TestANSI.lua");

	LuaObject sObj = state->GetGlobal("s");
	const char* str = sObj.GetString();
}


void TestUnicodeFile()
{
#if LUA_WIDESTRING
	LuaStateOwner state(true);
	state->DoFile("TestUnicode.lua");

	LuaObject sObj = state->GetGlobal("s");
	const lua_WChar* str = sObj.GetWString();
#endif /* LUA_WIDESTRING */
}


void ReadUnicodeFile()
{
    LuaStateOwner state(true);
    state->DoFile("ReadUnicodeFile.lua");
}

#endif // LUAPLUS_EXTENSIONS

void IntegerTest()
{
	LuaStateOwner state;
	state->OpenLibs();
	state->DoString("i1 = 10");
	state->DoString("i2 = 20; f1 = 15.5; f2 = .5");
	state->DoString("i3 = -10");
	state->DoString("print(type(i1), type(i2), type(i3), type(f1), type(f2))");
	state->DoString("res1 = i1 + i2");
	state->DoString("print(type(res1), res1)");
	state->DoString("res1 = i1 + f1");
	state->DoString("print(type(res1), res1)");
	state->DoString("res = -i1");
	state->DoString("print(type(res), res)");
	state->DoString("print(i1 | 3)");
	state->DoString("print(i1 & 2)");
	state->DoString("print(i1 ^ 12)");
	state->DoString("print(1 << 4)");
	state->DoString("print(32 >> 4)");

	state->DoString("i1 = 0x1000");
	state->DoString("print(i1, type(i1))");
	state->DoString("print(i1 | 0x80000000)");

	state->DoString("for index = 1, 10, 0.5 do print(index) end");
	state->DoString("for index = 1, 10, 2 do print(index) end");
}


void LookupTest()
{
	LuaStateOwner state;
    state->DoString("s = { { Hi = { abc = 10 } }, { Hello = { xyz = 20 } } }");

    LuaObject obj = state->GetGlobals().Lookup("s.1.Hi.x");
    obj = state->GetGlobals().Lookup("s.1.Hi.abc");
    obj = state->GetGlobals().Lookup("s.2.Hello.xyz");
}


class MultiObject
{
public:
	MultiObject(int num) :
		m_num(num)
	{
	}

	int Print(LuaState* state)
	{
		printf("%d\n", m_num);
		return 0;
	}

	void Print2(int num)
	{
		printf("%d %d\n", m_num, num);
	}

protected:
	int m_num;
};


void MultiObjectTest()
{
	LuaStateOwner state;

	LuaObject metaTableObj = state->GetGlobals().CreateTable("MultiObjectMetatable");
	metaTableObj.Set("__index", metaTableObj);
	metaTableObj.RegisterObjectFunctor("Print", &MultiObject::Print);
	metaTableObj.RegisterObjectDirect("Print2", (MultiObject*)0, &MultiObject::Print2);

	MultiObject obj1(10);
	LuaObject obj1Obj = state->BoxPointer(&obj1);
	obj1Obj.SetMetatable(metaTableObj);
	state->GetGlobals().Set("obj1", obj1Obj);

	MultiObject obj2(20);
	LuaObject obj2Obj = state->BoxPointer(&obj2);
	obj2Obj.SetMetatable(metaTableObj);
	state->GetGlobals().Set("obj2", obj2Obj);

	state->DoString("obj1:Print()");
	state->DoString("obj2:Print()");
	state->DoString("obj1:Print2(5)");
	state->DoString("obj2:Print2(15)");

	LuaObject table1Obj = state->GetGlobals().CreateTable("table1");
	table1Obj.SetLightUserdata("__object", &obj1);
	table1Obj.SetMetatable(metaTableObj);

	LuaObject table2Obj = state->GetGlobals().CreateTable("table2");
	table2Obj.SetLightUserdata("__object", &obj2);
	table2Obj.SetMetatable(metaTableObj);

	state->DoString("table1:Print()");
	state->DoString("table2:Print()");
	state->DoString("table1:Print2(5)");
	state->DoString("table2:Print2(15)");
}


class lua_StateObjectHelper
{
public:
	lua_StateObjectHelper() :
		m_str("Me")
	{
	}

	int PrintSomething(lua_State* L)
	{
		printf("Hello, world! %s\n", m_str);
		return 0;
	}

protected:
	const char* m_str;
};


int PrintSomethingGlobal(lua_State* L)
{
	printf("In global PrintSomething.\n");
	return 0;
}

/*
void lua_StateCallbackTest()
{
	lua_State* L = luaL_newstate();

	lua_StateObjectHelper helper;

	lua_pushstring(L, "PrintSomething");
	lua_pushfunctorclosureex(L, helper, &lua_StateObjectHelper::PrintSomething, 0);
	lua_settable(L, LUA_GLOBALSINDEX);

	lua_pushstring(L, "PrintSomethingGlobal");
	lua_pushfunctorclosure(L, PrintSomethingGlobal, 0);
	lua_settable(L, LUA_GLOBALSINDEX);

	luaL_dostring(L, "PrintSomething()");
	luaL_dostring(L, "PrintSomethingGlobal()");

	lua_close(L);
}
*/

void MemoryTest()
{
	SimpleHeap mainHeap("Test", false);
	mainHeap.Initialize(1 * 1024 * 1024);
	heap = &mainHeap;

	GetMemLogFile().Create("dump.txt");

	{
		LuaStateOwner state(ReallocFunction, &mainHeap);
		{
//			state->DoFile("s:\\svndb\\lua-5.0.1\\test\\sieve.lua");
			DumpIt();
		}
		state->GC(LUA_GCCOLLECT, 0);
		DumpIt();
	}
}


void RCTest()
{
	LuaStateOwner state(false);
	LuaObject stringObj(state);
	stringObj.Assign(state, "Test");
#if LUA_WIDESTRING
	stringObj.Assign(state, (const lua_WChar*)L"Wide String");
#endif /* LUA_WIDESTRING */
}

#if LUA_EXCEPTIONS

void ExceptionTest()
{
	LuaStateOwner state;
	state->DoString("MyTable = { 1, 2, 3, 4 }");

	try
	{
		void* data = state->GetGlobal("MyTable")[1].GetUserdata();
	}
	catch (const LuaException& /*e*/)
	{
		int hi = 5; (void)hi;
	}

	try
	{
		LuaObject obj = state->GetGlobals()["MyTable"][1][2];
	}
	catch (const LuaException& /*e*/)
	{
		int hi = 5; (void)hi;
	}
}

#endif // LUA_EXCEPTIONS


void BisectTest()
{
	LuaStateOwner state;
	state->DoFile("../../Test/bisect.lua");
}


class MyPropertyTest
{
public:
	MyPropertyTest()
	{
	}

	void SetVar(int index)
	{
		m_var = index;
	}

	int m_var;
	int m_var2;
};


int aGlobalVariable = 10;


void PropertyTest()
{
	LuaStateOwner state;
	
	MyPropertyTest test;

	LuaObject metaTableObj = state->GetRegistry().CreateTable("MyPropertyClassMetatable");
	LPCD::Metatable_IntegratePropertySupport(metaTableObj);
	metaTableObj.RegisterObjectDirect("SetVar", (MyPropertyTest*)0, &MyPropertyTest::SetVar);

	LuaObject table1Obj = state->GetGlobals().CreateTable("table1");
	table1Obj.SetLightUserdata("__object", &test);
	table1Obj.SetMetatable(metaTableObj);

	LPCD::PropertyCreate(metaTableObj, "var", &MyPropertyTest::m_var);

	state->DoString("table1.var = 20");
	state->DoString("print(table1.var)");

	test.m_var2 = 100;

	LPCD::Register_MemberPropertyGetFunction(table1Obj, "GetTheVar2", &MyPropertyTest::m_var2);
	LPCD::Register_MemberPropertySetFunction(table1Obj, "SetTheVar2", &MyPropertyTest::m_var2);
	state->DoString("print(table1:GetTheVar2())");
	state->DoString("print(table1:SetTheVar2(50))");

	LPCD::PropertyCreate(metaTableObj, "var2", &MyPropertyTest::m_var2);

	state->DoString("print(table1.var2)");
	state->DoString("table1.var2 = 60");
	state->DoString("print(table1.var2)");

	try
	{
		state->DoString("table1.var3 = 60");
	}
	catch (LuaException&)
	{
	}

	try
	{
		state->DoString("print(table1.var3)");
	}
	catch (LuaException&)
	{
	}

	state->DoString("table1:SetVar(600)");

	LPCD::Register_GlobalPropertyGetFunction(state->GetGlobals(), "GetAGlobalVariable", &aGlobalVariable);
	LPCD::Register_GlobalPropertySetFunction(state->GetGlobals(), "SetAGlobalVariable", &aGlobalVariable);

	state->DoString("print(GetAGlobalVariable())");
	state->DoString("SetAGlobalVariable(50)");
	state->DoString("print(GetAGlobalVariable())");

	int end = 5; (void)end;
}


int PropertyMetatable_newindex(LuaState* state)
{
	LuaStack args(state);
	LuaObject propsObj = args[1].GetMetatable()["__props"];
	if (propsObj.IsTable())
	{
		LuaObject propObj = propsObj[args[2]];
		if (propObj.IsNil())
		{
			luaL_argerror(*state, 1, "The property is not available.");
		}

		LuaObject funcObj = propObj[2];
		args[1].Push();
		args[3].Push();
		state->Call(2, 1);
		return 1;
	}

	return 0;
}


void SetTest()
{
	LuaStateOwner state;

	state->DoString("abs = 5");
#if _MSC_VER > 1300
	state->GetGlobals().Set("abs", 10);
#endif
	state->GetGlobals().SetNil("abs");
}


void BadDoString()
{
	LuaStateOwner state;

	try
	{
		state->DoString("MyTable = { Name1 = 5, Name2 = 10 Name3 = 15 }");
	}
	catch (LuaException&)
	{
	}
}


void LoadCompiledScript()
{
	LuaStateOwner state;
	int ret = state->DoFile("CompileMe.lc");
	ret;
}



class VECTOR 
{
public:
	double x,y,z; 
}; 


class MONSTER 
{
public:
	int alive;
	void *mesh; 
	VECTOR position; 
	char name[32];
}; 

namespace LPCD
{
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


void PassVector(VECTOR& vec)
{
	printf("Vector: %f, %f, %f\n", vec.x, vec.y, vec.z);
}

using namespace LuaPlus;

void VectorMonsterMetatableTest()
{
	LuaStateOwner state;
	state->OpenLibs();

	LuaObject vectorMetatableObj = state->GetRegistry().CreateTable("VECTOR"); 
	LPCD::Metatable_IntegratePropertySupport(vectorMetatableObj); 

	LPCD::PropertyCreate(vectorMetatableObj, "x", &VECTOR::x); 
	LPCD::PropertyCreate(vectorMetatableObj, "y", &VECTOR::y); 
	LPCD::PropertyCreate(vectorMetatableObj, "z", &VECTOR::z); 

	LuaObject monsterMetatableObj = state->GetRegistry().CreateTable("MONSTER"); 
	LPCD::Metatable_IntegratePropertySupport(monsterMetatableObj); 

	LPCD::PropertyCreate(monsterMetatableObj, "alive", &MONSTER::alive); 
	LPCD::PropertyCreate(monsterMetatableObj, "mesh", &MONSTER::mesh); 

	LPCD::PropertyCreate(monsterMetatableObj, "position", &MONSTER::position);
//	LPCD::PropertyCreate(monsterMetatableObj, "name", &MONSTER::name);

	MONSTER monster;

	LuaObject monsterObj = state->GetGlobals().CreateTable("Monster");
	monsterObj.SetLightUserdata("__object", &monster);
	monsterObj.SetMetatable(monsterMetatableObj);

	state->DoString("Monster.alive = 1");
	state->DoString("Monster.position.x = 5");
	state->DoString("Monster.position.y = 10");
	state->DoString("Monster.position.z = 15");
	state->DoString("print(Monster.position.x)");

	state->GetGlobals().RegisterDirect("PassVector", &PassVector);
	state->DoString("PassVector(Monster.position)");
}


void GlobalErrorTest()
{
    LuaStateOwner state(false);
    try
    {
        state->DoString("Foo = nil; Foo.x = 9");
    }
    catch (LuaException&)
    {
    }
}


void DoStringErrorTest()
{
    // init luastate with all libs open
    LuaStateOwner state;
    if (state->DoString("invalid code:") != 0)
	{
		printf("%s\n", state->Stack(-1).GetString());
	}
}


void RCBlowup()
{
	LuaStateOwner state;
	state->DoString("function Test()\nlocal file = io.open('TestScript.cpp')\nend\nTest()\n");
}


void ForBlowup()
{
    LuaStateOwner state;
    state->DoFile("ForBlowup.lua");
}


void TestClass()
{
    LuaStateOwner state;
    state->DoString("package.path='../../test/?.lua;' .. package.path");
    int ret = state->DoFile("../../test/animal.lua");
    assert(ret == 0);
    
    LuaObject createFunctionObj = state->GetGlobal("Dog");
    createFunctionObj.Push(state);
    state->PushString("Golden Retriever");
    state->PCall(1, 1, 0);
    LuaObject myObject(state, -1);
    state->GetGlobals().Set("GoldenRetriever", myObject);

    state->DoString("print(GoldenRetriever)");
}


static int delay(LuaState* pState)
{
    // delay (length)
    LuaStack tStack(pState);

    if (tStack[1].IsInteger())
    {
        printf("Delaying for %d seconds...\n", tStack[1].GetInteger());
    }

    return 0;
}


void CoroutineTest()
{
    LuaStateOwner pState;
    LuaObject tGlobals = pState->GetGlobals();
    tGlobals.Register("delay", delay);

    pState->DoString("function TestCoroutine()\n"
"    delay(4)\n"
"end\n");

    pState->DoString("co = coroutine.create(TestCoroutine)");
    pState->DoString("coroutine.resume(co)");
}

void MemoryKeyTest()
{
	SimpleHeap mainHeap("Test", false);
	mainHeap.Initialize(1 * 1024 * 1024);
	heap = &mainHeap;

	{
		LuaStateOwner state(ReallocFunction, &mainHeap);
		mainHeap.IntegrityCheck();

		LuaObject string1;
		LuaObject string2;
		LuaObject string3;
		LuaObject string4;
		LuaObject string5;
		LuaObject string6;
		LuaObject string7;
		LuaObject string8;
		LuaObject string1_1;
		LuaObject string2_1;
		LuaObject string3_1;
		LuaObject string4_1;
		LuaObject string5_1;
		LuaObject string6_1;
		LuaObject string7_1;
		LuaObject string8_1;
		string1.Assign(state, "String1");
		string2.Assign(state, "String2");
		string3.Assign(state, "String3");
		string4.Assign(state, "String4");
		string5.Assign(state, "String5");
		string6.Assign(state, "String6");
		string7.Assign(state, "String7");
		string8.Assign(state, "String8");
		string1_1.Assign(state, "String1_1");
		string2_1.Assign(state, "String2_1");
		string3_1.Assign(state, "String3_1");
		string4_1.Assign(state, "String4_1");
		string5_1.Assign(state, "String5_1");
		string6_1.Assign(state, "String6_1");
		string7_1.Assign(state, "String7_1");
		string8_1.Assign(state, "String8_1");

		LuaObject globalsObj = state->GetGlobals();
		globalsObj.Set(string1, "Stuff1");
		globalsObj.Set(string2, "Stuff2");
		globalsObj.Set(string3, "Stuff3");
		globalsObj.Set(string4, "Stuff4");
		globalsObj.Set(string5, "Stuff5");
		globalsObj.Set(string6, "Stuff6");
		globalsObj.Set(string7, "Stuff7");
		globalsObj.Set(string8, "Stuff8");
		globalsObj.Set(string1_1, "Stuff1_1");
		globalsObj.Set(string2_1, "Stuff2_1");
		globalsObj.Set(string3_1, "Stuff3_1");
		globalsObj.Set(string4_1, "Stuff4_1");
		globalsObj.Set(string5_1, "Stuff5_1");
		globalsObj.Set(string6_1, "Stuff6_1");
		globalsObj.Set(string7_1, "Stuff7_1");
		globalsObj.Set(string8_1, "Stuff8_1");

		globalsObj.SetNil(string8_1);
		globalsObj.SetNil(string7_1);
		globalsObj.SetNil(string6_1);
		globalsObj.SetNil(string5_1);
		globalsObj.SetNil(string4_1);
		globalsObj.SetNil(string3_1);
		globalsObj.SetNil(string2_1);
		globalsObj.SetNil(string1_1);
		globalsObj.SetNil(string8);
		globalsObj.SetNil(string7);
		globalsObj.SetNil(string6);
		globalsObj.SetNil(string5);
		globalsObj.SetNil(string4);
		globalsObj.SetNil(string3);
		globalsObj.SetNil(string2);
		globalsObj.SetNil(string1);

		globalsObj.Set(string1, "Stuff1");
		globalsObj.Set(string2, "Stuff2");
		globalsObj.Set(string3, "Stuff3");
		globalsObj.Set(string4, "Stuff4");
		globalsObj.Set(string5, "Stuff5");
		globalsObj.Set(string6, "Stuff6");
		globalsObj.Set(string7, "Stuff7");
		globalsObj.Set(string8, "Stuff8");
		globalsObj.Set(string1_1, "Stuff1_1");
		globalsObj.Set(string2_1, "Stuff2_1");
		globalsObj.Set(string3_1, "Stuff3_1");
		globalsObj.Set(string4_1, "Stuff4_1");
		globalsObj.Set(string5_1, "Stuff5_1");
		globalsObj.Set(string6_1, "Stuff6_1");
		globalsObj.Set(string7_1, "Stuff7_1");
		globalsObj.Set(string8_1, "Stuff8_1");

		globalsObj.SetNil(string8_1);
		globalsObj.SetNil(string7_1);
		globalsObj.SetNil(string6_1);
		globalsObj.SetNil(string5_1);
		globalsObj.SetNil(string4_1);
		globalsObj.SetNil(string3_1);
		globalsObj.SetNil(string2_1);
		globalsObj.SetNil(string1_1);
		globalsObj.SetNil(string8);
		globalsObj.SetNil(string7);
		globalsObj.SetNil(string6);
		globalsObj.SetNil(string5);
		globalsObj.SetNil(string4);
		globalsObj.SetNil(string3);
		globalsObj.SetNil(string2);
		globalsObj.SetNil(string1);
	}
}


void WeakTableTest()
{
	LuaStateOwner state;
	state->OpenLibs();

	LuaObject stuffObj = state->GetGlobals().CreateTable("Stuff");
	{
		LuaObject numObj = state->GetGlobals().CreateTable("Num");

		{
			LuaObject obj;
			obj.AssignNewTable(state);
	//		stuffObj.SetObject("Hi", obj);

			numObj.Insert(obj);
		}

	//	stuffObj.SetNil("Hi");
		numObj.Remove(1);
	}
	state->GetGlobals().SetNil("Num");
	state->GC(LUA_GCCOLLECT, 0);
}

extern "C"
{
#include "lualib.h"
}

void MiniTest()
{
	LuaStateOwner state;
	state->OpenLibs();
	int ret = state->DoString("a = 5");

	LuaObject obj = state->GetGlobals()["a"];
	printf("%g\n", obj.GetNumber());

	ret = state->DoString("a = true");
	obj = state->GetGlobals()["a"];

	ret = state->DoString("a = 'Hello'");
	obj = state->GetGlobals()["a"];

	ret = state->DoString("a = L'Hello'");
	obj = state->GetGlobals()["a"];

	obj.Assign(state, (void*)0x12345678);

	ret = state->DoString("a = function() end");
	obj = state->GetGlobals()["a"];

	obj.Assign(state, (void*)0x12345678);

	ret = state->DoString("a = { { 'hello', 'hi', 10 }, { { key = 'value', me = true, }, 'one more', [1000] = 'stuff' } }");
	obj = state->GetGlobals()["a"];
}


void TestParser()
{
    {
        LuaStateOwner state;
        assert(_CrtCheckMemory());
        state->DoString("x = 0x12345678; print(x);");
        assert(_CrtCheckMemory());
    }
    {
        LuaStateOwner state;
        assert(_CrtCheckMemory());
        state->DoString("str = L'ab\\x1234\\xabcdef'");
        assert(_CrtCheckMemory());
    }
    {
        LuaStateOwner state;
        assert(_CrtCheckMemory());
        state->DoString("str = L'Wide string'");
        assert(_CrtCheckMemory());
    }
    {
        LuaStateOwner state;
        assert(_CrtCheckMemory());
        state->DoString("str2 = L'Wide string'");
        assert(_CrtCheckMemory());
    }
}


void TestGC()
{
    LuaStateOwner state(false);
    lua_State* L = state->GetCState();
    lua_pushstring(L, "Hello");
    while (state->GC(LUA_GCSTEP, 1) != 1)
    {
    }
    lua_pop(L, 1);
    state->GC(LUA_GCRESTART, 1);
    while (state->GC(LUA_GCSTEP, 1) != 1)
    {
    }
    state->GC(LUA_GCRESTART, 1);
    lua_pushstring(L, "SomeValue");
    lua_setglobal(L, "SomeKey");
    while (state->GC(LUA_GCSTEP, 1) != 1)
    {
    }
    lua_pushnil(state->GetCState());
    lua_setglobal(L, "SomeKey");
    while (state->GC(LUA_GCSTEP, 1) != 1)
    {
    }

    state->GC(LUA_GCRESTART, 1);
    LuaObject strObj;
    strObj.Assign(state, "Hello");
    while (state->GC(LUA_GCSTEP, 1) != 1)
    {
    }
    strObj.Reset();
    state->GC(LUA_GCRESTART, 1);
    while (state->GC(LUA_GCSTEP, 1) != 1)
    {
    }
}


void TestRC1()
{
	lua_State* L = luaL_newstate();
	lua_pushnumber(L, 10);
	lua_pop(L, 1);
	lua_pushstring(L, "Hello");
	lua_pop(L, 1);
	lua_newuserdata(L, sizeof(unsigned int));
	lua_pop(L, 1);
	lua_newtable(L);
	lua_pop(L, 1);
	lua_close(L);
}


static void l_message (const char *pname, const char *msg) {
  if (pname) fprintf(stderr, "%s: ", pname);
  fprintf(stderr, "%s\n", msg);
}


static int report (lua_State* L, int status) {
  const char *msg;
  if (status) {
    msg = lua_tostring(L, -1);
    if (msg == NULL) msg = "(error with no message)";
    l_message("rctest", msg);
    lua_pop(L, 1);
  }
  return status;
}


int GCFunc(lua_State* L)
{
	printf("The GC Function.\n");
	return 0;
}

void SimpleGCTest()
{
	lua_State* L = luaL_newstate();
	lua_pushcclosure(L, GCFunc, 0);
	lua_pop(L, 1);
	lua_close(L);

	L = luaL_newstate();
	lua_pushnumber(L, 5);
	lua_pop(L, 1);

	lua_pushstring(L, "Hello");
	lua_pop(L, 1);

	lua_newtable(L);				// t
	lua_pushstring(L, "MyTable");	// t s
	lua_pushstring(L, "Test");		// t s s
	lua_settable(L, -3);			// t

	lua_pushstring(L, "MyTable");	// t s
	lua_pushnil(L);					// t s n
	lua_settable(L, -3);			// t

	lua_pop(L, 1);					//

	// Create the metatable.
	lua_newtable(L);				// m

	// GCFunc
	lua_pushstring(L, "__gc");
	lua_pushcclosure(L, GCFunc, 0);
	lua_settable(L, -3);			// m

	// User data
	void* buf = lua_newuserdata(L, 4);		// m u
	*(unsigned int*)buf = 0x12345678;

	lua_pushvalue(L, -2);			// m u m
	lua_setmetatable(L, -2);		// m u

	lua_pop(L, 1);					// m
	lua_pop(L, 1);					//

	lua_gc(L, LUA_GCCOLLECT, 0);

	lua_close(L);
}


void UserTest()
{
	lua_State* L = luaL_newstate();

	int top1 = lua_gettop(L);

	// Create the metatable.
	lua_newtable(L);				// m

	int top2 = lua_gettop(L);

	// GCFunc
	lua_pushstring(L, "__gc");
	lua_pushcclosure(L, GCFunc, 0);
	lua_settable(L, -3);			// m

	int top3 = lua_gettop(L);

	// User data
	lua_newuserdata(L, 100);		// m u

	int top4 = lua_gettop(L);

	lua_pushvalue(L, -2);			// m u m
	lua_setmetatable(L, -2);

	int top5 = lua_gettop(L);

	lua_pop(L, 1);					// m

	int top6 = lua_gettop(L);

	lua_pop(L, 1);					//

	lua_gc(L, LUA_GCCOLLECT, 0);

	lua_close(L);
}


static int GCTest_pmain (lua_State *L)
{
    lua_pushcfunction(L, luaopen_base);
    lua_pushstring(L, "");
    lua_call(L, 1, 0);

    lua_pushcfunction(L, luaopen_io);
    lua_pushstring(L, LUA_IOLIBNAME);
    lua_call(L, 1, 0);

    lua_pushcfunction(L, luaopen_string);
    lua_pushstring(L, LUA_STRLIBNAME);
    lua_call(L, 1, 0);

    lua_pushcfunction(L, luaopen_math);
    lua_pushstring(L, LUA_MATHLIBNAME);
    lua_call(L, 1, 0);

    lua_pushcfunction(L, luaopen_debug);
    lua_pushstring(L, LUA_DBLIBNAME);
    lua_call(L, 1, 0);

#if LUA_WIDESTRING
    lua_pushcfunction(L, luaopen_wstring);
    lua_pushstring(L, LUA_WSTRLIBNAME);
    lua_call(L, 1, 0);
#endif /* LUA_WIDESTRING */

	return 0;
}


#define lua_cpcall(L,f,u)  \
	(lua_pushcfunction(L, (f)), \
	 lua_pushlightuserdata(L,(u)), \
	 lua_pcall(L,1,0,0))

void GCTest()
{
	lua_State* L = luaL_newstate();
	lua_cpcall(L, &GCTest_pmain, NULL);

	luaL_dofile(L, "../../test/sieve.lua");
//	lua_setgcthreshold(L, 0);
//	lua_dofile(L, "../../test/sort.lua");
	luaL_dofile(L, "../../test/bisect.lua");

	lua_pushstring(L, "Hello");
	lua_pop(L, 1);

	luaL_dostring(L, "table1 = { str = 'Hi', num = 5 }");
	luaL_dostring(L, "table1 = nil");
	luaL_dostring(L, "table2 = table1");
	luaL_dostring(L, "table2 = nil");

	clock_t c = clock();
	luaL_dostring(L, "for i = 1, 1000000 do myTable = {} end");
	printf("%f\n", (double)(clock() - c) / CLOCKS_PER_SEC);

	lua_close(L);
}

void SimpleTest()
{
    lua_State* L = luaL_newstate();

    unsigned* idPtr = (unsigned*)lua_newuserdata( L, sizeof(unsigned) );
    *idPtr = 0x12345678;

    lua_newtable( L );
    lua_setmetatable( L, -2 );

	lua_pop(L, 1);

	int top = lua_gettop(L);

	lua_close(L);
}


int myrawset( lua_State* L )
{
    lua_rawset(L, 1);
    return 1;
}

int GlobalNewIndexRawSetTest(void)
{
    lua_State* L = luaL_newstate();

    lua_newtable( L );
    lua_pushliteral( L, "__newindex" );
    lua_pushcclosure( L, myrawset, 0 );
    lua_rawset( L, -3 );

#if LUA_VERSION_NUM == 501
    lua_setmetatable( L, LUA_GLOBALSINDEX );
#elif LUA_VERSION_NUM >= 502
	lua_pushglobaltable( L );
#endif
    lua_setmetatable( L, -1 );

    lua_pushnumber( L, 42 );
    lua_setglobal( L, "abc" );

#if LUA_VERSION_NUM == 501
    lua_pushnil( L );
    while( lua_next( L, LUA_GLOBALSINDEX ) != 0 ) /* assertion failure occurs in lua_next */
#elif LUA_VERSION_NUM >= 502
	lua_pushglobaltable( L );
    lua_pushnil( L );
    while( lua_next( L, -2 ) != 0 ) /* assertion failure occurs in lua_next */
#endif
    {
        lua_pop( L, 1 );
    }

    return 0;
}


static int NullOutNext_Inner(lua_State* L)
{

    int tblIdx;

    lua_newtable( L );						// t
    tblIdx = lua_gettop( L );

    lua_pushstring( L, "abc" );				// t abc
    lua_pushboolean( L, true );				// t abc true
    lua_settable( L, tblIdx );				// t

    lua_pushstring( L, "def" );				// t def
    lua_pushboolean( L, true );				// t def true
    lua_settable( L, tblIdx );				// t

    lua_pushnil( L );						// t nil
    while( lua_next( L, tblIdx ) != 0 )		// t key value
    {
        lua_pop( L, 1 );					// t key

        lua_pushvalue( L, -1 );				// t key key
        lua_pushnil( L );					// t key key nil
        lua_settable( L, tblIdx );			// t key
    }

	return 0;
}


int NullOutNext()
{
    lua_State* L = luaL_newstate();
	int status = lua_cpcall(L, NullOutNext_Inner, 0);
	report(L, status);
	lua_close(L);

	return 0;
}


int __newindexMetatableDoNothing( lua_State* L )
{
    return 1;
}

int GlobalNewIndexDoNothingTest(void)
{
    lua_State* L = luaL_newstate();

    lua_newtable( L );
    lua_pushliteral( L, "__newindex" );
    lua_pushcclosure( L, __newindexMetatableDoNothing, 0 );
    lua_rawset( L, -3 );

#if LUA_VERSION_NUM == 501
    lua_setmetatable( L, LUA_GLOBALSINDEX );
#elif LUA_VERSION_NUM >= 502
	lua_pushglobaltable( L );
#endif
    lua_setmetatable( L, -1 );

    lua_pushnumber( L, 42 );
    lua_setglobal( L, "abc" );

#if LUA_VERSION_NUM == 501
    lua_pushnil( L );
    while( lua_next( L, LUA_GLOBALSINDEX ) != 0 ) /* assertion failure occurs in lua_next */
#elif LUA_VERSION_NUM >= 502
	lua_pushglobaltable( L );
    lua_pushnil( L );
    while( lua_next( L, -2 ) != 0 ) /* assertion failure occurs in lua_next */
#endif
    {
        lua_pop( L, 1 );
    }

    return 0;
}

void FEnvTest()
{
    lua_State* L = luaL_newstate();

	luaopen_base(L);

	lua_getglobal(L, "_ALERT");
	int top1 = lua_gettop(L);

	const char* buf = "var = 5  function f() print(var) end  t = {}  setfenv(f, t)  f()";
	luaL_loadbuffer(L, buf, strlen(buf), buf);

//	lua_newtable(L);
//	lua_setfenv(L, -2);
//	lua_newtable(L);
//	lua_setfenv(L, -2);

	lua_pcall(L, 0, 0, top1 - 1);

	int top = lua_gettop(L);

	lua_close(L);
}

void MiniTests()
{
	lua_State* L = luaL_newstate();

	lua_pushstring(L, "Hello");
	lua_pushstring(L, "Hi");
	lua_pushstring(L, "Everyone");

	lua_remove(L, 2);

	lua_pop(L, 1);
	lua_pop(L, 1);

	lua_pushstring(L, "Test1");
	lua_pushstring(L, "Test2");
	lua_replace(L, 1);

	lua_pop(L, 1);

	lua_newtable(L);

	for (int i = 1; i <= 8; ++i)
	{
		char str[50];
		sprintf(str, "Entry %d", i);
		lua_pushstring(L, str);
		lua_rawseti(L, -2, i);
	}

	assert(lua_gettop(L) == 1);

	for (int i = 3; i <= 8; ++i)
	{
		if (i != 5)
		{
			lua_pushnil(L);
			lua_rawseti(L, -2, i);
		}
	}

	// Cause it to rehash
	lua_pushnumber(L, 16);
	lua_pushnumber(L, 50);
	lua_rawset(L, -3);

	lua_pop(L, 1);

	lua_close(L);
}


void NextTest()
{
	lua_State* L = luaL_newstate();

	luaL_dostring(L, "MyTable = { Key1 = 5, KeyA = 10, KeyQ = 15 }");

	lua_getglobal(L, "MyTable");
	lua_pushnil(L);
	while (lua_next(L, -2) != 0)
	{
		const char* key = lua_tostring(L, -2);
		int value = (int)lua_tonumber(L, -1);
		printf("%s = %d\n", key, value);
		lua_pop(L, 1);
	}

	lua_pushstring(L, "Key1");		// Should have a ref count of 2.
	lua_pop(L, 1);
	lua_pushstring(L, "KeyA");		// Should have a ref count of 2.
	lua_pop(L, 1);
	lua_pushstring(L, "KeyQ");		// Should have a ref count of 2.
	lua_pop(L, 1);

	lua_close(L);
}


void ConcatTest()
{
	lua_State* L = luaL_newstate();

	luaL_dostring(L, "s = 'Hello' .. '*' .. 'Everybody'");

	lua_close(L);
}


void SetTopTest()
{
	lua_State* L = luaL_newstate();

	lua_pushstring(L, "Hello");
	lua_pushstring(L, "Hi");
	lua_pushstring(L, "Yo");
	lua_settop(L, -2);

	lua_close(L);
}


void RCTests()
{
	SimpleGCTest();

	SetTopTest();
	ConcatTest();
	NextTest();
	MiniTests();

	FEnvTest();
	NullOutNext();
	GlobalNewIndexRawSetTest();
	GlobalNewIndexDoNothingTest();
	GCTest();
	SimpleTest();
	UserTest();
}


void SimpleConstructor()
{
	LuaStateOwner state;
	state->OpenLibs();
	state->DoFile("Progression.blua");
}


void TestCoroutine()
{
	LuaStateOwner state;
	state->OpenLibs();
	state->DoFile("coroutine.lua");

	LuaObject threadObj = LuaState::CreateThread(state);
	LuaState* threadState = threadObj.GetState();
	LuaObject functionObj = threadState->GetGlobal("TestCoroutine");
	functionObj.Push(threadState);
	int ret = threadState->Resume((LuaState*)NULL, 0);
	ret = threadState->Resume((LuaState*)NULL, 0);
	ret = threadState->Resume((LuaState*)NULL, 0);
	ret = threadState->Resume((LuaState*)NULL, 0);
	ret = threadState->Resume((LuaState*)NULL, 0);
	ret = threadState->Resume((LuaState*)NULL, 0);
	ret = threadState->Resume((LuaState*)NULL, 0);
}


void TableTest()
{
	LuaStateOwner state;
	state->DoString( "t = { a = { 25 }, b = 'stringy' }");
	state->GetGlobals().SetNil("t");
}


void TestState()
{
	LuaStateOwner state;
}

void TestGCLuaScript()
{
	LuaStateOwner state;
	state->OpenLibs();
//	state->DoFile("testgc.lua");
}


void RefTest()
{
	LuaStateOwner state;
	state->OpenLibs();
	lua_State *L = *state;

	for (int loop = 0; loop < 3; ++loop)
	{
		std::vector<LuaObject> refs;
		DWORD time = GetTickCount();
		refs.reserve(1000000);
		for (int index = 1; index < 1000000; ++index)
		{
			LuaObject obj = state->GetGlobal("table");
			refs.push_back(obj);
		}
		for (int index = 1; index < 1000000; ++index)
		{
			refs[index - 1].Type();
		}
		for (int index = 1; index < 1000000; ++index)
		{
			refs.pop_back();
		}
		time = GetTickCount() - time;
		printf("LuaObject(%d): %d\n", loop, time);
	}

	for (int loop = 0; loop < 3; ++loop)
	{
		std::vector<int> refs;
		DWORD time = GetTickCount();
		refs.reserve(1000000);
		for (int index = 1; index < 1000000; ++index)
		{
			lua_getglobal(L, "table");
//			luafast_getfield(L, LUA_GLOBALSINDEX, "table");
			refs.push_back(lua_fastref(L));
		}
		for (int index = 1; index < 1000000; ++index)
		{
#if LUA_FASTREF_SUPPORT
			lua_type(L, refs[index - 1]);
#else
			lua_getfastref(L, refs[index - 1]);
			lua_type(L, -1);
			lua_pop(L, 1);
#endif // LUA_FASTREF_SUPPORT
//			lua_pop(L, 1);
		}
		for (int index = 1; index < 1000000; ++index)
		{
			lua_fastunref(L, refs[index - 1]);
		}
		time = GetTickCount() - time;
		printf("fastref(%d): %d\n", loop, time);
	}

	for (int loop = 0; loop < 3; ++loop)
	{
		std::vector<int> refs;
		DWORD time = GetTickCount();
		refs.reserve(1000000);
		for (int index = 1; index < 1000000; ++index)
		{
			lua_getglobal(L, "table");
			refs.push_back(luaL_ref(L, LUA_REGISTRYINDEX));
		}
		for (int index = 1; index < 1000000; ++index)
		{
			lua_rawgeti(L, LUA_REGISTRYINDEX, refs[index - 1]);
			lua_type(L, -1);
			lua_pop(L, 1);
		}
		for (int index = 1; index < 1000000; ++index)
		{
			luaL_unref(L, LUA_REGISTRYINDEX, refs[index - 1]);
		}
		time = GetTickCount() - time;
		printf("luaref(%d): %d\n", loop, time);
	}

	for (int loop = 0; loop < 3; ++loop)
	{
		DWORD time = GetTickCount();
		for (int index = 1; index < 1000000; ++index)
		{
			LuaObject obj = state->GetGlobal_Stack("table");
			obj.Type();
			state->Pop(1);
		}
		time = GetTickCount() - time;
		printf("LuaObject-oneloop(%d): %d\n", loop, time);
	}

	for (int loop = 0; loop < 3; ++loop)
	{
		DWORD time = GetTickCount();
		for (int index = 1; index < 1000000; ++index)
		{
			int ref;
			lua_getglobal(L, "table");
//			luafast_getfield(L, LUA_GLOBALSINDEX, "table");
			ref = lua_fastref(L);

//			lua_getfastref(L, ref);
//			lua_type(L, -1);
//			lua_pop(L, 1);
			lua_type(L, ref);

			lua_fastunref(L, ref);
		}
		time = GetTickCount() - time;
		printf("fastref-oneloop(%d): %d\n", loop, time);
	}

	for (int loop = 0; loop < 3; ++loop)
	{
		DWORD time = GetTickCount();
		for (int index = 1; index < 1000000; ++index)
		{
			int ref;
			lua_getglobal(L, "table");
			ref = luaL_ref(L, LUA_REGISTRYINDEX);

			lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
			lua_type(L, -1);
			lua_pop(L, 1);

			luaL_unref(L, LUA_REGISTRYINDEX, ref);
		}
		time = GetTickCount() - time;
		printf("luaref-oneloop(%d): %d\n", loop, time);
	}

	{
		lua_getglobal(L, "table");
		int tableRef = lua_fastref(L);
		lua_getglobal(L, "string");
		int stringRef = lua_fastref(L);

		lua_getfastref(L, tableRef);
		lua_typename(L, -1);

		lua_fastunref(L, stringRef);
		lua_fastunref(L, tableRef);
	}

	{
		LuaObject tableObj = state->GetGlobal("table");
		LuaObject stringObj = state->GetGlobal("string");
	}
	assert(_CrtCheckMemory());
}


void TestSet()
{
	LuaStateOwner state;
	state->OpenLibs();
	LuaObject tableObj(state->CreateTable());
	tableObj.RawSetString(1, "Hello");
}


#if _WIN32_WCE
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
#else // _WIN32_WCE
int __cdecl main(int argc, char* argv[])
#endif // _WIN32_WCE
{
	TestSet();

//	RefTest();
//	if (1) return 0;

	TestState();
	TestGCLuaScript();
	TableTest();
	TestCoroutine();
	SimpleConstructor();
	SetUserdata();
	WeakTableTest();
    TestGC();
    TestParser();
    MiniTest();
	MemoryKeyTest();
    CoroutineTest();
//    TestClass();
    ForBlowup();
	NewUserdataBoxTest();
	DoStringErrorTest();
    GlobalErrorTest();
    TestStdString();
	VectorMonsterMetatableTest();

#if LUAPLUS_EXTENSIONS  &&  LUA_VERSION_NUM <= 501
	TestANSIFile();
	TestUnicodeFile();
    ReadUnicodeFile();
#endif // LUAPLUS_EXCEPTIONS

	LoadCompiledScript();
	BadDoString();
	SetTest();

	PropertyTest();
	BisectTest();
	RCTest();
#if LUA_EXCEPTIONS
	ExceptionTest();
#endif // LUA_EXCEPTIONS

	int i;

	{
		LuaStateOwner state(false);
		state->DoString("s = \"12345678901234567890123456789012345678901234567890\"");
		state->DoString("s2 = L\"abcdefghijklmnopqrstuvwxyzzyxwvutsrqponmlkjihgfedcba\"");
	}
	LookupTest();
	MemoryTest();
//	lua_StateCallbackTest();
	MultiObjectTest();
	IntegerTest();

	LuaState* state = LuaState::Create();
	state->OpenLibs();

	DWORD count = GetTickCount();
	luaL_dostring(*state, "a = true;  if a then print(\"Hi\") end");

	luaL_dostring(*state, "theGlobal = 5; function ReturnTrue() if theGlobal == 5 then return true else return false end end");

	LuaObject obj = state->GetGlobals().CreateTable("Stuff");
	for (i = 0; i < 100000; ++i)
	{
//		obj.CreateTable(i);
	}

		lua_getglobal(*state, "ReturnTrue");
	for (i = 0; i < 1000000; ++i)
	{
		lua_pushvalue(*state, -1);
//		char s[80];
//		sprintf(s, "%d", i);
//		lua_pushstring(*state, s);
		lua_pushnumber(*state, (lua_Number)i);
		lua_call(*state, 1, 1);
		lua_pop(*state, 1);
	}

	state->GC(LUA_GCCOLLECT, 0);

	count = GetTickCount() - count;
	printf("%d\n", count);


	DumpTest();
//	TestDumpGlobals();
	TestGCObject();
	TestThreeThreads();
	TestNewCall();
	CloneTest();

	TestPointer();
	{
		LuaStateOwner state;
		state->DoString("Table = { 0, 1, 2, 'Hello', nil, 'Hi', Yo = 'My Stuff', NobodysHome = 5, NestedTable = { 1, 2, 3, { 'String', }, { 'Table2' } }, { 'String1' } }");
		LuaObject globalsObj = state->GetGlobals();
		LuaObject tableObj = state->GetGlobal("Table");
		LuaObject numObj = tableObj[2];
	}
	LuaTableIteratorTest();
//	LuaStackTableIteratorTest();

	DoScriptFormatTest();

	DoScriptCallbackTest();
	DoScriptSaveTest();
	DoScriptArrayTest();
	TestPointer();
	RCBlowup();
	TestRC1();
	RCTests();

	return 0;
}
