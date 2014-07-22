#!/usr/bin/env lua
local process = require "ex.process"
assert(arg[1], "argument required")
local proc = assert(process.spawn(arg[1]))
print(proc)
print(assert(proc:wait()))
