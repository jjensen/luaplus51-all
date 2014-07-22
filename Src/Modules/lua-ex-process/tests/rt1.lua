#!/usr/bin/env lua
local process = require "ex.process"

--print"os.sleep"
--os.sleep(2);

assert(process.setenv("foo", "42"))
print("expect foo= 42")
print("foo=", process.getenv("foo"))
assert(process.setenv("foo", nil))
print("expect foo= nil")
print("foo=", process.getenv("foo"))


