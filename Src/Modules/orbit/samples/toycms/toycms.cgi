#!/usr/bin/env lua51

require"lfs"

lfs.chdir("/path/to/toycms")

require"wsapi.cgi"

require"toycms"

wsapi.cgi.run(toycms.run)

