#include "../../LuaPlus/LuaPlus.h"
#include "FileGlobBase.h"

using namespace LuaPlus;

class LuaFileGlob : public FileGlobBase
{
public:
	LuaFileGlob() :
		m_numEntries( 0 )
	{
	}

	virtual void FoundMatch( const char* name )
	{
		m_numEntries++;
		m_tableObj.SetString( m_numEntries, name );
	}

	LuaStackObject m_tableObj;
	int m_numEntries;
};


static int LS_Glob_MatchPattern( LuaState* state )
{
	LuaStack args(state);
	LuaFileGlob fileGlob;

	state->CreateTable();
    fileGlob.m_tableObj = state->StackTop();
	fileGlob.MatchPattern( args[ 1 ].GetString() );

	fileGlob.m_tableObj.Push();

	return 1;
}


extern "C" LUAMODULE_API int luaopen_glob(lua_State* L)
{
	LuaState* state = LuaState::CastState(L);
	LuaObject globObj = state->GetGlobals().CreateTable( "glob" );
	globObj.Register( "match", LS_Glob_MatchPattern );
	return 0;
}


