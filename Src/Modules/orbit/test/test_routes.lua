
require "orbit.routes"

local R = orbit.routes.R

do
   local r = R('/foo')
   local t = r:match("/foo")
   assert(t)
end

do
   local r = R('/foo')
   local t = r:match("/bar")
   assert(not t)
end

do
   local r = R('/foo')
   local t = r:match("/foobar")
   assert(not t)
end

do
   local r = R("/foo/bar/:baz")
   local t = r:match("/foo/bar/boo")
   assert(t.baz == "boo")
end

do
   local r = R("/foo/bar/:baz")
   local t = r:match("/bar/boo")
   assert(not t)
end

do
   local r = R("/foo/bar/:baz")
   local t = r:match("/foo/bar/boo/bloo")
   assert(not t)
end

do
   local r = R("/say/:msg/to/:to")
   local t = r:match("/say/hello/to/world")
   assert(t.msg == "hello")
   assert(t.to == "world")
end

do
   local r = R('/say/*/to/*')
   local t = r:match('/say/hello/to/world')
   assert(#t.splat == 2)
   assert(t.splat[1] == "hello")
   assert(t.splat[2] == "world")
end

do
   local r = R('/download/*.*')
   local t = r:match('/download/path/to/file.xml')
   assert(#t.splat == 2)
   assert(t.splat[1] == "path/to/file")
   assert(t.splat[2] == "xml")
end

do
   local r = R('/*/foo/*/*')
   local t = r:match('/bar/foo/bling/baz/boom')
   assert(#t.splat == 3)
   assert(t.splat[1] == "bar")
   assert(t.splat[2] == "bling")
   assert(t.splat[3] == "baz/boom")
end

do
   local r = R('/:foo/*')
   local t = r:match('/foo/bar/baz')
   assert(#t.splat == 1)
   assert(t.foo == "foo")
   assert(t.splat[1] == "bar/baz")
end

do
   local r = R('/:foo/:bar')
   local t = r:match('/user@example.com/name')
   assert(t.foo == "user@example.com")
   assert(t.bar == "name")
end

do
   local r = R('/:foo.:bar')
   local t = r:match('/user@example.com')
   assert(t.foo == "user@example")
   assert(t.bar == "com")
end

do
   local r = R('/*')
   local t = r:match("/foo/bar/baz")
   assert(t.splat[1] == "foo/bar/baz")
end

do
   local r = R('/?:foo?/?:bar?')
   local t = r:match('/hello/world')
   assert(t.foo == 'hello')
   assert(t.bar == 'world')
end

do
   local r = R('/?:foo?/?:bar?')
   local t = r:match('/hello')
   assert(t.foo == 'hello')
   assert(not t.bar)
end

do
   local r = R('/?:foo?/?:bar?')
   local t = r:match('/')
   assert(not t.foo)
   assert(not t.bar)
end

do
   local r = R('/:foo/*')
   local t = r:match('/hello%20world/how%20are%20you')
   assert(t.foo == "hello world")
   assert(t.splat[1] == "how are you")
end 
