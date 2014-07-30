#!/usr/bin/env lua
local osprocess = require "osprocess"
assert(arg[1], "need a command name")
print"osprocess.pipe()"
local i, o = assert(osprocess.pipe())
print("got", i, o)
print"osprocess.spawn()"
local t = {command = arg[1], stdin = i}
print(t.stdin)
local proc = assert(osprocess.spawn(t))
print"i:close()"
i:close()
print"o:write()"
o:write("Hello\nWorld\n")
print"o:close()"
o:close()
print"proc:wait()"
print("exit status:", assert(proc:wait()))
