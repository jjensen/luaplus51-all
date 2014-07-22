#!/usr/bin/env lua
local process = require "ex.process"
assert(arg[1], "need a command name")
print"process.pipe()"
local i, o = assert(process.pipe())
print("got", i, o)
print"process.spawn()"
local t = {command = arg[1], stdin = i}
print(t.stdin)
local proc = assert(process.spawn(t))
print"i:close()"
i:close()
print"o:write()"
o:write("Hello\nWorld\n")
print"o:close()"
o:close()
print"proc:wait()"
print("exit status:", assert(proc:wait()))
