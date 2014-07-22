#!/usr/bin/env lua
local path = require 'ex.path'

print"path.chdir"
assert(path.chdir("tests"))
print(path.getcwd())

print"path.mkdir"
assert(path.mkdir("Foo.test/"))
assert(path.chdir("Foo.test"))
