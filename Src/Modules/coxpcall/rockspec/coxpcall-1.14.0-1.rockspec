package = "Coxpcall"

version = "1.14.0-1"

source = {
  url = "http://www.keplerproject.org/files/coxpcall-1.14.0.tar.gz",
}

description = {
  summary = "Coroutine safe xpcall and pcall",
  detailed = [[
 Encapsulates the protected calls with a coroutine based loop, so errors can
 be dealed without the usual Lua 5.x pcall/xpcall issues with coroutines
 yielding inside the call to pcall or xpcall.
  ]],
  license = "MIT/X11",
  homepage = "http://coxpcall.luaforge.net"
}

dependencies = { }

build = {
   type = "builtin",
   modules = { coxpcall = "src/coxpcall.lua" }
}
