package = "Coxpcall"

version = "scm-1"

description = {
  summary = "Coroutine safe xpcall and pcall",
  detailed = [[
 Encapsulates the protected calls with a coroutine based loop, so errors can
 be handled without the usual Lua 5.x pcall/xpcall issues with coroutines
 yielding inside the call to pcall or xpcall.
  ]],
  license = "MIT/X11",
  homepage = "http://keplerproject.github.io/coxpcall"
}

dependencies = { }

source = {
  url = "git://github.com/keplerproject/coxpcall.git"
}

build = {
   type = "bultin",
   modules = { coxpcall = "src/coxpcall.lua" }
}
