#!/usr/bin/env lua

require"lfs"

lfs.chdir("/home/mascarenhas/work/orbit/samples/songs")

require"wsapi.fastcgi"

require"songs"

wsapi.fastcgi.run(songs.run)

