#!/usr/bin/env lua
local process = require "ex.process"

print"process.environ"
local e = assert(process.environ())
for nam, val in pairs(e) do
    print(string.format("%s=%s", nam, val))
end
