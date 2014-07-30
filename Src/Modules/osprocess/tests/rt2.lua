#!/usr/bin/env lua
local osprocess = require "osprocess"

print"process.environ"
local e = assert(osprocess.environ())
for nam, val in pairs(e) do
    print(string.format("%s=%s", nam, val))
end
