## Introduction

P4Lua, a Lua interface to the Perforce API, enables you to write Lua code that interacts with a Perforce server. P4Lua enables your Lua scripts to:

* Get Perforce data and forms in Lua tables
* Edit Perforce forms by modifying Lua tables
* Provide optional exception-based error handling and optionally ignore warnings
* Issue multiple commands on a single connection (performs better than spawning single commands and parsing the results)





## System Requirements

P4Lua is supported on Windows.

To build P4Lua from source, your development machine must also have:

* Lua 5.1.4 development files
* The 2008.2 Perforce C/C++ API for your target platform
* The same C++ compiler used to build the Perforce C++ API on your target platform.  (If you get “unresolved symbol” errors when building or running P4Lua, you probably used the wrong compiler or the wrong Perforce API build. )

	


## Credits

The primary contents of this document come from Perforce 2008.2 APIs for Scripting.





## Programming with P4Lua

P4Lua provides an object-oriented interface to Perforce that is intended to be intuitive for Lua programmers. Data is loaded and returned in Lua tables. Each P4 object represents a connection to the Perforce Server.

When instantiated, the P4 instance is set up with the default environment settings just as the command line client p4, that is, using environment variables, the registry on Windows and, if defined, the `P4CONFIG` file. The settings can be checked and changed before the
connection to the server is established with the `connect()` method. After your script connects, it can send multiple commands to the Perforce Server with the same P4 instance. After the script is finished, it should disconnect from the Server by calling the
`disconnect()` method.

The following example illustrates the basic structure of a P4Lua script. The example establishes a connection, issues a command, and tests for errors resulting from the command.

<pre>
    require 'p4'                              -- Import the module
    p4 = P4.P4()                              -- Create the P4 instance
    p4.port = "1666"
    p4.user = "fred"
    p4.client = "fred-ws"                     -- Set some environment variables
    ret, err = pcall( function()              -- Catch exceptions
    	p4:connect()                          -- Connect to the Perforce Server
    	info = p4:run("info")                 -- Run "p4 info" (returns a table)
    	for key, value in pairs(info[1]) do   -- and display all key-value pairs
            print(key .. '=' .. value)
	    end
        p4:run("edit", "file.txt")            -- Run "p4 edit file.txt"
    end)

    for _, e in ipairs(p4.errors) do          -- Display errors
        print(e)
    end

    p4:disconnect()                       -- Disconnect from the Server
</pre>





This example creates a client workspace from a template and syncs it:.

<pre>
    require 'p4'

    template = "my-client-template"
    client_root = "c:\\work\\my-root"

    p4 = P4.P4()

    pcall(function()
        p4:connect()
        -- Convert client spec into a Lua dictionary
        client = p4:fetch_client("-t", template)
        client._root = client_root
        p4:save_client(client)
        p4:run_sync()
    end)
</pre>




### Submitting a Changelist

This example creates a changelist, modifies it and then submits it:

<pre>
    require 'p4'
    p4 = P4.P4()
    p4:connect()
    change = p4:fetch_change()

    -- Files were opened elsewhere and we want to
    -- submit a subset that we already know about.
    myfiles = { '//depot/some/path/file1.c', '//depot/some/path/file1.h' }
    change._description = "My changelist\nSubmitted from P4Lua\n"
    change._files = myfiles -- This attribute takes a Lua list
    p4:run_submit(change)
</pre>




### Logging into Perforce using ticket-based authentication

On some servers, users might need to log in to Perforce before issuing commands. The
following example illustrates login using Perforce tickets.

<pre>
    require 'p4'
    p4 = P4.P4()
    p4.user = "Sven"
    p4.password = "my_password"
    p4:connect()
    p4:run_login()
    opened = p4:run_opened()
    [...]
</pre>



### Changing your password

You can use P4Lua to change your password, as shown in the following example:

<pre>
    require 'p4'
    p4 = P4.P4()
    p4.user = "Joshua"
    p4.password = "MyOldPassword"
    p4:connect()
    p4:run_password("MyOldPassword", "MyNewPassword")
    -- p4.password is automatically updated with the encoded password
</pre>


