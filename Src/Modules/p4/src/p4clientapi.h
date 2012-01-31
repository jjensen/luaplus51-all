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
 * Name		: p4clientapi.h
 *
 * Author	: Tony Smith <tony@perforce.com> or <tony@smee.org>
 *
 * Description	: Ruby bindings for the Perforce API. Definitions of our
 * 		  main interface to Perforce
 *
 ******************************************************************************/


/*******************************************************************************
 * P4ClientAPI class - where we register our Ruby classes and plumb together
 * the components
 ******************************************************************************/

class Enviro;
class P4ClientAPI
{
public:
    P4ClientAPI( lua_State *L );
    ~P4ClientAPI();

	// Tagged mode - can be enabled/disabled on a per-command basis
    int SetTagged( int enable );
    int	GetTagged();

    // Set track mode - track usage of this command on the server

    int SetTrack( bool enable );
    int GetTrack();

    // Set streams mode 

    int SetStreams( bool enable );
    int GetStreams();

    // Set API level for backwards compatibility
    int SetApiLevel( int level );
    int SetMaxResults( int v )		{ maxResults = v; return 0; }
    int SetMaxScanRows( int v )		{ maxScanRows = v; return 0; }
    int SetMaxLockTime( int v )		{ maxLockTime = v; return 0; }
    //
    // Debugging support. Debug levels are:
    //
    //     1:	Debugs commands being executed
    //     2:	Debug UI method calls
    //     3:	Show garbage collection ??? 
    //
    int SetDebug( int d );

    // Returns 0 on success, otherwise -1 and might raise exception
    int SetCharset( const char *c );

    int SetClient( const char *c )	{ client.SetClient( c ); return 0; }
    int SetCwd( const char *c );
    bool SetEnv( const char *var, const char *value );
    int SetHost( const char *h )	{ client.SetHost( h ); return 0; }
    int SetLanguage( const char *l )	{ client.SetLanguage( l ); return 0; }
    int SetPassword( const char *p )	{ client.SetPassword( p ); return 0; }
    int SetPort( const char *p );
    int SetProg( const char *p )	{ prog = p; return 0; }
    int SetTicketFile( const char *p );
    int SetUser( const char *u )	{ client.SetUser( u ); return 0; }
    int SetVersion( const char *v )	{ version = v; return 0; }

    int	 GetApiLevel()				{ return apiLevel;		}
    const char * GetCharset()		{ return client.GetCharset().Text(); }
    const char * GetClient()		{ return client.GetClient().Text();}
    const char * GetConfig()		{ return client.GetConfig().Text();}
    const char * GetCwd()		{ return client.GetCwd().Text(); }
    const char * GetEnv( const char *var );
    const char * GetHost()		{ return client.GetHost().Text(); }
    const char * GetLanguage()		{ return client.GetLanguage().Text(); }
    const char * GetPassword()		{ return client.GetPassword().Text(); }
    const char * GetPort()		{ return client.GetPort().Text(); }
    const char * GetProg()		{ return prog.Text(); }
    const char * GetTicketFile()	{ return ticketFile.Text(); }
    const char * GetUser()		{ return client.GetUser().Text(); }
    const char * GetVersion()		{ return version.Text(); }
//    const char * GetPatchlevel()	{ return ID_PATCH; }
    const char * GetOs()		{ return client.GetOs().Text(); }
    int GetMaxResults()			{ return maxResults; }
    int GetMaxScanRows()		{ return maxScanRows; }
    int GetMaxLockTime()		{ return maxLockTime; }
    int GetDebug()			{ return debug; }

    // Session management
    int Connect();		// P4Exception on error
    int Connected();		// Return true if connected and not dropped.
    int Disconnect();

    int GetServerLevel();  // P4Exception if asked when disconnected
    int GetServerCaseInsensitive(); // P4Exception if asked when disconnected
    int GetServerUnicode(); // P4Exception if asked when disconnected

	// Executing commands.
    int Run( const char *cmd, int argc, char * const *argv );
    int SetInput( int input );
    int GetInput();

	int SetResolver( int resolver ){ ui.SetResolver( resolver ); return 0; }
    int GetResolver()				{ return ui.GetResolver(); }

    // OutputHandler interface
    int SetHandler( int handler );
    int GetHandler();

	// Result handling
    int GetErrors()		{ return ui.GetResults().GetErrors();}
    int GetWarnings()		{ return ui.GetResults().GetWarnings();}
    int GetMessages()		{ return ui.GetResults().GetMessages();}
    int GetTrackOutput()	{ return ui.GetResults().GetTrack();}

    // Spec parsing
    int ParseSpec( const char * type, const char *form );
    int FormatSpec( const char *type, int table );
    int SpecFields( const char * type );

    // Protocol
    int SetProtocol( const char * var, const char *val );
    int GetProtocol( const char * var );

    // Exception levels:
    //
    // 		0 - No exceptions raised
    // 		1 - Exceptions raised for errors
    // 		2 - Exceptions raised for errors and warnings
    //
    int  SetExceptionLevel( int i )	{ exceptionLevel = i; return 0;	}
    int  GetExceptionLevel()		{ return exceptionLevel; }

    int Except( const char *func, const char *msg );
    int Except( const char *func, Error *e );
    int Except( const char *func, const char *msg, const char *cmd );

private:

    void RunCmd(const char *cmd, ClientUser *ui, int argc, char * const *argv);
    int ConnectOrReconnect();

private:
    enum {
	S_TAGGED 	= 0x0001,
	S_CONNECTED	= 0x0002,
	S_CMDRUN	= 0x0004,
	S_UNICODE	= 0x0008,
	S_CASEFOLDING	= 0x0010,
	S_TRACK		= 0x0020,
	S_STREAMS	= 0x0040,

	S_INITIAL_STATE	= 0x0041,
	S_RESET_MASK	= 0x001E,
    };

    void	InitFlags()		{ flags = S_INITIAL_STATE;	}
    void	ResetFlags()		{ flags &= ~S_RESET_MASK;	}

    void	SetTag()		{ flags |= S_TAGGED;		}
    void	ClearTag()		{ flags &= ~S_TAGGED;		}
    int		IsTag()			{ return flags & S_TAGGED;	}

    void	SetConnected()		{ flags |= S_CONNECTED;		}
    void	ClearConnected() 	{ flags &= ~S_CONNECTED;	}
    int		IsConnected()		{ return flags & S_CONNECTED;	}

    void	SetCmdRun()		{ flags |= S_CMDRUN;		}
    void	ClearCmdRun() 		{ flags &= ~S_CMDRUN;		}
    int		IsCmdRun()		{ return flags & S_CMDRUN;	}

    void	SetUnicode()		{ flags |= S_UNICODE;		}
    void	ClearUnicode() 		{ flags &= ~S_UNICODE;		}
    int		IsUnicode()		{ return flags & S_UNICODE;	}

    void	SetCaseFold()		{ flags |= S_CASEFOLDING;	}
    void	ClearCaseFold()		{ flags &= ~S_CASEFOLDING;	}
    int		IsCaseFold()		{ return flags & S_CASEFOLDING;	}

    void	SetTrackMode()		{ flags |= S_TRACK;		}
    void	ClearTrackMode()	{ flags &= ~S_TRACK;		}
    int		IsTrackMode()		{ return flags & S_TRACK;	}

    void	SetStreamsMode()	{ flags |= S_STREAMS;		}
    void	ClearStreamsMode()	{ flags &= ~S_STREAMS;		}
    int		IsStreamsMode()		{ return flags & S_STREAMS;	}

	// subclass KeepAlive to implement a customized IsAlive function.
	class MyKeepAlive : public KeepAlive
	{
	public:
		virtual int IsAlive();

		P4ClientAPI* client;
	} ;

	lua_State *		L;
    ClientApi		client;
    ClientUserLua	ui;
    Enviro *		enviro;
    SpecMgr		specMgr;
    StrBufDict		specDict;
    StrBuf		prog;
    StrBuf		version;
    StrBuf		ticketFile;
    int			depth;
    int			apiLevel;
    int			debug;
    int			exceptionLevel;
    int			server2;
    int			flags;
    int			maxResults;
    int			maxScanRows;
    int			maxLockTime;
//	MyKeepAlive	cb;
	int			handlerRef;
};

