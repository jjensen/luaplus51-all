# Introduction

The original introduction follows:

A recent (Jan 2006) discussion on the mailing list has prompted me to attempt to design an extended API which extends the Lua API by adding functions to the os and io namespaces.

This is not a proposal to modify the Lua core, but a design proposal for an API which extends the Lua core. This API is meant to provide a more complete programming environment for stand-alone Lua programs on today's popular operating systems (Windows, MacOSX and POSIX platforms).






# Credits

The primary contents of this document come from <http://lua-users.org/wiki/ExtensionProposal>.





# ex API

Note that all these functions return the standard (nil,"error message") on failure and that, unless otherwise specified, they return (true) on success.






