package = "Rings"
version = "1.3.0-1"

source = {
   url = "http://www.keplerproject.org/rings/rings-1.3.0.tar.gz",
}

description = {
   summary = "Create new Lua states from within Lua",
   detailed = [[
      Rings is a library which provides a way to create new Lua states
      from within Lua. It also offers a simple way to communicate
      between the creator (master) and the created (slave) states.
   ]],
   license = "MIT/X11",
   homepage = "http://www.keplerproject.org/rings/"
}
dependencies = {
   "lua >= 5.1"
}
build = {
   type = "builtin",
   modules = {
    rings = "src/rings.c",
    stable = "src/stable.lua"
   }
}
