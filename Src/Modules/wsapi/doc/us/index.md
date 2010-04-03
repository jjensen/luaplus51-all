## Overview

*WSAPI* is an API that abstracts the web server from Lua web applications. By coding
against WSAPI your application can run on any of the supported servers and
interfaces (currently CGI, FastCGI and Xavante, on Windows and UNIX-based systems).

WSAPI provides a set of helper libraries that help with request processing
and output buffering. You can also write applications that act as filters that
provide some kind of service to other applications, such as authentication,
file uploads, request isolation, or multiplexing.

WSAPI's main influence is Ruby's [Rack](http://rack.rubyforge.org/) framework, but it was
also influenced by Python's [WSGI](http://wsgi.org/wsgi) (PEP 333). It's not a direct
clone of either of them, though, and tries to follow standard Lua idioms.

WSAPI is free software and uses the same license as Lua 5.1

## Status

Current version is 1.2. It was developed for Lua 5.1.

## Download

You can get WSAPI using [LuaRocks](http://luarocks.org):

<pre class="example">
luarocks install wsapi-xavante
</pre>

You can also get an installer script that installs Lua+LuaRocks+WSAPI 
[here](http://cloud.github.com/downloads/keplerproject/wsapi-install-1.2.0.tar.gz). See
the [manual](manual.html) for installation instructions.

## Latest Sources and Bug Tracker

WSAPI sources and bug tracker are available at its [Github](http://github.com/keplerproject/wsapi/) page.

## History

**WSAPI 1.2** [27/Oct/2009]

* Adds time-based collection of Lua states to FCGI and Xavante launchers
* Adds "wsapi" laucher script, to start a Xavante WSAPI server
* Fixed "undefined media type" error
* Added is_empty utility function to check if a string is nil or ''
* Fixed bug with empty bodies in wsapi.xavante, and added full http status codes to responses 
* Changing order of evaluating PATH\_TRANSLATED and SCRIPT\_FILENAME, to make non-wrapped launchers work in OSX Apache
* Reload support for load\_isolated\_launcher

**WSAPI 1.1** [04/Feb/2009]

* Adds *options* table to **wsapi.request.new**, *delay_post* option delays
POST processing until **req:parse_post_data()** is called
* Moves call to **lfs.setmode** from wsapi.common to wsapi.cgi
* Adds **wsapi.util.make\_rewindable(*wsapi\_env*)** method - wraps *wsapi\_env* in a new
environment that lets you process the POST data more than once.
* Correctly handles PATH\_TRANSLATED and SCRIPT\_FILENAME in case the web server gets creative
* Statically links the FastCGI version on Windows

[**WSAPI 1.0**](http://wsapi.luaforge.net/1.0/) [18/May/2008]

* First public version.
* Includes CGI, FastCGI and Xavante WSAPI connectors.

## Credits

WSAPI was designed and developed by Fabio Mascarenhas and
Andr&eacute; Carregal, and is maintained by Fabio Mascarenhas.

## Contact Us

For more information please [contact us](mailto:info-NO-SPAM-THANKS@keplerproject.org).
Comments are welcome!

You can also reach us and other developers and users on the Kepler Project 
[mailing list](http://luaforge.net/mail/?group_id=104). 

