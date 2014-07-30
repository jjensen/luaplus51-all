#!/usr/bin/env lua
local osprocess = require "osprocess"

--print"osprocess.sleep"
--osprocess.sleep(2);

assert(osprocess.setenv("foo", "42"))
print("expect foo= 42")
print("foo=", osprocess.getenv("foo"))
assert(osprocess.setenv("foo", nil))
print("expect foo= nil")
print("foo=", osprocess.getenv("foo"))


