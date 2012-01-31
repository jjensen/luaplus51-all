/*******************************************************************************

Copyright (c) 2001-2008, Perforce Software, Inc.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1.  Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2.  Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL PERFORCE SOFTWARE, INC. BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

/*******************************************************************************
* Name		: p4clientapi.cc
*
* Author	: Tony Smith <tony@perforce.com> or <tony@smee.org>
*
* Description	: Ruby bindings for the Perforce API. Main interface to the
* 		  Perforce API.
*
******************************************************************************/
extern "C" {
#include "lua.h"
#include "lauxlib.h"
}
#include "undefdups.h"
#include "p4/clientapi.h"
#include "p4/i18napi.h"
#include "p4/enviro.h"
#include "p4/hostenv.h"
#include "p4/spec.h"
#include "p4/strtable.h"
#include "p4result.h"
#include "p4luadebug.h"
#include "clientuserlua.h"
#include "specmgr.h"
#include "p4clientapi.h"


#define		M_TAGGED		0x01
#define		M_PARSE_FORMS		0x02
#define		IS_TAGGED(x)		(x & M_TAGGED )
#define		IS_PARSE_FORMS(x)	(x & M_PARSE_FORMS )


P4ClientAPI::P4ClientAPI( lua_State *L )
	: L( L )
	, specMgr( L )
	, ui( L, &specMgr )
{
	debug = 0;
	server2 = 0;
	depth = 0;
	exceptionLevel = 2;
	maxResults = 0;
	maxScanRows = 0;
	maxLockTime = 0;
	prog = "unnamed p4lua script";
	apiLevel = atoi( P4Tag::l_client );
	enviro = new Enviro;

//	cb.client = this;

	InitFlags();

	// Enable form parsing
	client.SetProtocol( "specstring", "" );

    //
    // Load the current working directory, and any P4CONFIG file in place
    //
	HostEnv henv;
	StrBuf cwd;

	henv.GetCwd( cwd, enviro );
	if( cwd.Length() )
		enviro->Config( cwd );

	//
	// Load the current ticket file. Start with the default, and then
	// override it if P4TICKETS is set.
	//
	const char *t;

  	henv.GetTicketFile( ticketFile );

	if( t = enviro->Get( "P4TICKETS" ) ) {
		ticketFile = t;
	}

    // 
    // Do the same for P4CHARSET
    //
    
    const char *lc;
    if( ( lc = enviro->Get( "P4CHARSET" )) ) {
        SetCharset(lc);
    }
}

P4ClientAPI::~P4ClientAPI()
{
	if ( IsConnected() ) {
		Error e;
		client.Final( &e );
		// Ignore errors
	}
	delete enviro;
}

int P4ClientAPI::SetTagged( int enable )
{
    if( enable )
	SetTag();
    else
	ClearTag();
	
    return 0;
}

int P4ClientAPI::GetTagged()
{
    return IsTag();
}

int P4ClientAPI::SetTrack( bool enable )
{
    if ( IsConnected() ) {
		if ( exceptionLevel )
			luaL_error( L, "Can't change tracking once you've connected.");
		return 0;
    }
	else {
		if( enable ) {
			SetTrackMode();
			ui.SetTrack(true);
		}
		else {
			ClearTrackMode();
			ui.SetTrack(false);
		}
	}
    return 0;
}

int P4ClientAPI::GetTrack()
{
    return IsTrackMode() != 0;
}

int P4ClientAPI::SetStreams( bool enable )
{
    if( enable )
	SetStreamsMode();
    else
	ClearStreamsMode();

    return 0;
}

int P4ClientAPI::GetStreams()
{
    return IsStreamsMode() != 0;
}

int P4ClientAPI::SetCwd( const char *c )
{
	client.SetCwd( c );
	enviro->Config( StrRef( c ) );
	return 0;
}

int P4ClientAPI::SetCharset( const char *c )
{
	if( P4LUADEBUG_COMMANDS )
		fprintf( stderr, "[P4] Setting charset: %s\n", c );

    CharSetApi::CharSet cs = CharSetApi::NOCONV;

	if ( strlen(c) > 0 ) {
		cs = CharSetApi::Lookup( c );
		if( cs < 0 )
		{
			if( exceptionLevel )
			{
				StrBuf	m;
				m = "Unknown or unsupported charset: ";
				m.Append( c );
				return Except( "P4.charset", m.Text() );
			}
			return 0;
		}
	}

    if( CharSetApi::Granularity( cs ) != 1 ) {
		return Except( "P4.charset", "P4Lua does not support a wide charset!");
    }

	client.SetCharset(c);
	
    client.SetTrans( cs, cs, cs, cs );
	return 1;
}

int P4ClientAPI::SetTicketFile( const char *p )
{
	client.SetTicketFile( p );
	ticketFile = p;
	return 0;
}

int P4ClientAPI::SetDebug( int d )
{
	debug = d;
	ui.SetDebug( d );
	specMgr.SetDebug( d );

//	if( P4LUADEBUG_RPC )
//		p4debug.SetLevel( "rpc=5" );
//	else
//		p4debug.SetLevel( "rpc=0" );
	return 0;
}

int P4ClientAPI::SetApiLevel( int level )
{
	StrBuf	b;
	b << level;
	apiLevel = level;
	client.SetProtocol( "api", b.Text() );
    ui.SetApiLevel( level );
	return 0;
}

int P4ClientAPI::SetPort( const char *p ) 
{
    if ( IsConnected() ) {
		return Except( "P4.port", "Can't change port once you've connected.");
    }
    else {
		client.SetPort( p );
		return 0; 
    }
}

const char * P4ClientAPI::GetEnv( const char *v)
{
	return enviro->Get( v );
}


bool P4ClientAPI::SetEnv( const char *var, const char *value )
{
	Error e;
	enviro->Set( var, value, &e );
	return !e.Test();
}


int P4ClientAPI::Connect()
{
	if ( P4LUADEBUG_COMMANDS )
		fprintf( stderr, "[P4] Connecting to Perforce\n" );

    if ( IsConnected() )
	{
		lua_pushboolean( L, true );
		lua_pushstring( L, "P4:connect - Perforce client already connected!" );
		return 2;
	}

    return ConnectOrReconnect();
}


int P4ClientAPI::ConnectOrReconnect()
{
    if( IsTrackMode() )
	client.SetProtocol( "track", "" );

    Error e;

    ResetFlags();
    client.Init( &e );
    if ( e.Test() && exceptionLevel ) {
		return Except( "P4:connect()", &e );
    }

    if ( e.Test() )
		return 0;

    // If an iterator is defined, reset the break functionality
    // for the KeepAlive function

    if( ui.GetHandler() != LUA_NOREF )
    {
		client.SetBreak( &ui );
    }

    SetConnected();

	lua_pushboolean( L, true );
	return 1;
}


int P4ClientAPI::Connected()
{
    if ( IsConnected() && !client.Dropped()) {
		lua_pushboolean( L, true );
		return 1;
    }
    else if ( IsConnected() ) 
		Disconnect();

	lua_pushboolean( L, false );
	return 1;
}


int P4ClientAPI::Disconnect()
{
	if ( P4LUADEBUG_COMMANDS )
		fprintf( stderr, "[P4] Disconnect\n" );

    if ( ! IsConnected() )
	{
//		rb_warn( "P4:disconnect - not connected" );
		return 0;
	}

	Error	e;
	client.Final( &e );
    ResetFlags();

	// Clear the specdef cache.
	specMgr.Reset();

    // Clear out any results from the last command
    ui.Reset();

	return 0;
}

int P4ClientAPI::Run( const char *cmd, int argc, char * const *argv )
{
	// Save the entire command string for our error messages. Makes it
	// easy to see where a script has gone wrong.
	StrBuf	cmdString;
	cmdString << "\"p4 " << cmd;
	for( int i = 0; i < argc; i++ )
		cmdString << " " << argv[ i ];
	cmdString << "\"";

	if ( P4LUADEBUG_COMMANDS )
		fprintf( stderr, "[P4] Executing %s\n", cmdString.Text()  );

	if ( depth )
	{
		lua_pushboolean( L, false );
		lua_pushstring( L, "Can't execute nested Perforce commands." );
		return 2;
	}

	// Clear out any results from the previous command
	ui.Reset();

	// Tell the UI which command we're running.
	ui.SetCommand( cmd );

    if ( ! IsConnected() && exceptionLevel ) {
		Except( "P4.run()", "not connected." );
		return 0;
    }
    
    if ( ! IsConnected()  ) {
		lua_pushboolean( L, false );
		return 1;
	}

	depth++;
	RunCmd( cmd, &ui, argc, argv );
	depth--;

    if( ui.GetHandler() != LUA_NOREF ) {
		if( client.Dropped() && ! ui.IsAlive() ) {
			Disconnect();
			ConnectOrReconnect();
		}
	}

	P4Result &results = ui.GetResults();

	if ( results.ErrorCount() && exceptionLevel )
		return Except( "P4:run", "Errors during command execution", cmdString.Text() );

	if ( results.WarningCount() && exceptionLevel > 1 )
		return Except( "P4:run", "Warnings during command execution",cmdString.Text());

	lua_rawgeti( L, LUA_REGISTRYINDEX, results.GetOutput() );
	return 1;
}


int P4ClientAPI::SetInput( int input )
{
	if ( P4LUADEBUG_COMMANDS )
		fprintf( stderr, "[P4] Received input for next command\n" );

	if ( ! ui.SetInput( input ) )
	{
		if ( exceptionLevel )
			Except( "P4#input", "Error parsing supplied data." );
		else {
			lua_pushboolean( L, false );
			return 1;
		}
	}
	lua_pushboolean( L, true );
	return 1;
}


int P4ClientAPI::GetInput()
{
    return ui.GetInput();
}

//
// Parses a string supplied by the user into a hash. To do this we need
// the specstring from the server. We try to cache those as we see them,
// but the user may not have executed any commands to allow us to cache
// them so we may have to fetch the spec first.
//

int
P4ClientAPI::ParseSpec( const char * type, const char *form )
{
	if ( !specMgr.HaveSpecDef( type ) )
	{
		if( exceptionLevel )
		{
			StrBuf m;
			m = "No spec definition for ";
			m.Append( type );
			m.Append( " objects." );
			return Except( "P4:parse_spec", m.Text() );
		}
	}
	else
	{
		lua_pushboolean( L, false );
		return 1;
	}

	// Got a specdef so now we can attempt to parse it.
	Error e;
	int v = specMgr.StringToSpec( type, form, &e );

	if ( e.Test() )
	{
		if ( exceptionLevel ) {
			return Except( "P4:parse_spec()", &e );
		} else {
			lua_pushboolean( L, false );
			return 1;
		}
	}

	return 1;
}


//
// Converts a table supplied by the user into a string using the specstring
// from the server. We may have to fetch the specstring first.
//

int P4ClientAPI::FormatSpec( const char * type, int table )
{
	if ( !specMgr.HaveSpecDef( type ) )
	{
		if( exceptionLevel )
		{
			StrBuf m;
			m = "No spec definition for ";
			m.Append( type );
			m.Append( " objects." );
			return Except( "P4:format_spec", m.Text() );
		}
		else
		{
			lua_pushboolean( L, false );
			return 0;
		}
	}

	// Got a specdef so now we can attempt to convert.
	StrBuf	buf;
	Error	e;

	specMgr.SpecToString( type, table, buf, &e );
	if( !e.Test() ) {
		lua_pushstring( L, buf.Text() );
		return 1;
	}

    if( exceptionLevel )
    {
		StrBuf m;
		m = "Error converting hash to a string.";
		if( e.Test() ) e.Fmt( m, EF_PLAIN );
		return Except( "P4:format_spec", m.Text() );
	}

	return 0;
}

//
// Returns a hash whose keys contain the names of the fields in a spec of the
// specified type. Not yet exposed to Ruby clients, but may be in future.
//
int
P4ClientAPI::SpecFields( const char * type )
{
	if ( !specMgr.HaveSpecDef( type ) )
	{
		if( exceptionLevel )
		{
			StrBuf m;
			m = "No spec definition for ";
			m.Append( type );
			m.Append( " objects." );
			return Except( "P4:spec_fields", m.Text() );
		}
		else
		{
			lua_pushboolean( L, false );
			return 0;
		}
	}

	return specMgr.SpecFields( type );
}

//
// Sets a server protocol value
//
int P4ClientAPI::SetProtocol( const char * var, const char * val )
{
    client.SetProtocol( var, val );

	return 0;
}

//
// Gets a protocol value
//
int P4ClientAPI::GetProtocol( const char * var ) 
{
    StrPtr *pv = client.GetProtocol( var );
    if ( pv ) {
		lua_pushstring( L, pv->Text() );
		return 1;
    }
    return 0;
}

//
// Returns the server level as provided by the server
//

int P4ClientAPI::GetServerLevel()
{
    if( !IsConnected() ) {
		return Except( "P4:run", "Not connected to a Perforce server");
    }
    
    if( !IsCmdRun() ) {
		lua_pushnil( L );
		Run( "info", 0, 0 );
		lua_pop( L, 1 );
	}

	lua_pushnumber( L, server2 );
	return 1;
}

// 
// Returns true if the server is case-insensitive
// Might throw exception if the information is not available yet
//
int P4ClientAPI::GetServerCaseInsensitive() 
{
    if( !IsConnected() ) {
		return Except( "P4:run", "Not connected to a Perforce server");
    }
    
    if( !IsCmdRun() ) {
		lua_pushnil( L );
		Run( "info", 0, 0 );
		lua_pop( L, 1 );
	}

    lua_pushboolean( L, IsCaseFold() );
	return 1;
}

// 
// Returns true if the server is case-insensitive
// Might throw exception if the information is not available yet
//
int P4ClientAPI::GetServerUnicode() 
{
    if( !IsConnected() ) {
		return Except( "P4:run", "Not connected to a Perforce server");
    }
    
    if( !IsCmdRun() ) {
		lua_pushnil( L );
		Run( "info", 0, 0 );
		lua_pop( L, 1 );
	}

    lua_pushboolean( L, IsUnicode() );
	return 1;
}

int P4ClientAPI::Except( const char *func, const char *msg )
{
	StrBuf	m;
	StrBuf	errors;
	StrBuf	warnings;
	bool	terminate = false;

	m << "[" << func << "] " << msg;

	// Now append any errors and warnings to the text
	ui.GetResults().FmtErrors( errors );
	ui.GetResults().FmtWarnings( warnings );

	if( errors.Length() )
	{
		m << "\n" << errors;
		terminate= true;
	}

	if( exceptionLevel > 1 && warnings.Length() )
	{
		m << "\n" << warnings;
		terminate= true;
	}

	if( terminate )
		m << "\n\n";

	if ( exceptionLevel )
	{
		luaL_error( L, m.Text() );
		return 0;
	}

	lua_pushnil( L );

	if( apiLevel < 68 ) {
		lua_pushstring( L, m.Text() );
	} else {
		// return a list with three elements:
		// the string value, the list of errors and list of warnings
		// P4Exception will sort out what's what
		lua_newtable( L );

		lua_pushstring( L, m.Text() );
		lua_rawseti( L, -2, 1 );

		lua_rawgeti( L, LUA_REGISTRYINDEX, ui.GetResults().GetErrors() );
		lua_rawseti( L, -2, 2 );

		lua_rawgeti( L, LUA_REGISTRYINDEX, ui.GetResults().GetWarnings() );
		lua_rawseti( L, -2, 3 );
	}

	return 2;
}

int P4ClientAPI::Except( const char *func, Error *e )
{
	StrBuf	m;

	e->Fmt( &m );
	return Except( func, m.Text() );
}

int P4ClientAPI::Except( const char *func, const char *msg, const char *cmd )
{
	StrBuf m;

	m << msg;
	m << "( " << cmd << " )";
	return Except( func, m.Text() );
}
/*
int P4ClientAPI::MyKeepAlive::IsAlive()
{
	int ret = 0;

	if ( client->keepAliveRef != LUA_NOREF ) {
		lua_rawgeti( client->L, LUA_REGISTRYINDEX, client->keepAliveRef );
		if ( lua_isfunction( client->L, -1 ) ) {
			if ( lua_pcall( client->L, 0, 1, 0 ) == 0 )
			{
				ret = lua_toboolean( client->L, -1 );
				lua_pop( client->L, 1 );
			}
		}
		else
			lua_pop( client->L, 1 );
	}
	else
		ret = 1;

	return ret;
}
*/

//
// RunCmd is a private function to work around an obscure protocol
// bug in 2000.[12] servers. Running a "p4 -Ztag client -o" messes up the
// protocol so if they're running this command then we disconnect and
// reconnect to refresh it. For efficiency, we only do this if the
// server2 protocol is either 9 or 10 as other versions aren't affected.
//

void P4ClientAPI::RunCmd( const char *cmd, ClientUser *ui, int argc, char * const *argv )
{
// #if P4APIVER_ID >= 513026
    // ClientApi::SetProg() was introduced in 2004.2
	client.SetProg( &prog );
// #endif

// #if P4APIVER_ID >= 513282
    // ClientApi::SetVersion() was introduced in 2005.2
	if( version.Length() )
		client.SetVersion( &version );
// #endif

    if( IsTag() )
		client.SetVar( "tag" );

    if( IsStreamsMode() && apiLevel >= 70 )
		client.SetVar( "enableStreams" );

    // If maxresults or maxscanrows is set, enforce them now
    if( maxResults  )	client.SetVar( "maxResults",  maxResults  );
    if( maxScanRows )	client.SetVar( "maxScanRows", maxScanRows );
    if( maxLockTime )	client.SetVar( "maxLockTime", maxLockTime );

//	client.SetBreak( &cb );
	client.SetArgv( argc, argv );
	client.Run( cmd, ui );

	// Have to request server2 protocol *after* a command has been run. I
	// don't know why, but that's the way it is.

    if ( ! IsCmdRun() )
    {
		StrPtr *pv = client.GetProtocol( "server2" );
		if ( pv )
			server2 = pv->Atoi();

		pv = client.GetProtocol( P4Tag::v_nocase );
		if ( pv ) 
			SetCaseFold();
	    
		pv = client.GetProtocol( P4Tag::v_unicode );
		if ( pv && pv->Atoi() )
			SetUnicode();
    }
    SetCmdRun();
}


//
// Raises an exception or returns Qfalse on bad input
//

int P4ClientAPI::SetHandler( int handler )
{
	if ( P4LUADEBUG_CALLS )
		fprintf( stderr, "[P4] SetKeepAlive()\n" );

    if ( ! ui.SetHandler( handler ) )
    {
		return -1;
    }

	lua_rawgeti( L, LUA_REGISTRYINDEX, handler );
	if ( lua_isnil( L, -1 ) )
		client.SetBreak(NULL);
    else
		client.SetBreak(&ui);
	lua_pop( L, -1 );

	return 0;
}

int P4ClientAPI::GetHandler()
{
    return ui.GetHandler();
}

