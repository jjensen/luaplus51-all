#!/usr/bin/env lua

require"lfs"

lfs.chdir("/home/mascarenhas/work/orbit/samples/songs")

require"wsapi.cgi"

require"songs"

wsapi.cgi.run(songs.run)

