#!/usr/bin/env lua51

require"lfs"

lfs.chdir("/path/to/toycms")

require"wsapi.fastcgi"

require"toycms"

wsapi.fastcgi.run(toycms.run)
