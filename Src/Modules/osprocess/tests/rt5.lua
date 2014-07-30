#!/usr/bin/env lua
local osprocess = require "osprocess"
assert(arg[1], "argument required")
local proc = assert(osprocess.spawn(arg[1]))
print(proc)
print(assert(proc:wait()))
