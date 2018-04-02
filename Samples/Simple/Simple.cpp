#include "LuaPlus/LuaPlus.h"

using namespace LuaPlus;

int main()
{
	LuaStateOwner state(true);
	int ret = state->DoString("MyTable = { Value = 'Hello', 10, 20, 30 }; print('Hello!')");
    return 0;
}

