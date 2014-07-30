#!/usr/bin/env lua
local ospath = require 'ospath'
local f = assert(io.open("hullo.test", "w+"))
ospath.lock(f, "w")
f:write("Hello\n")
ospath.unlock(f)
f:seek("set")
print(f:read())
f:close()
os.remove("hullo.test")

