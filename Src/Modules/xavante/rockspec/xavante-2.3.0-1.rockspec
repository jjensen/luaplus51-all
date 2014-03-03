package = "Xavante"

version = "2.3.0-1"

description = {
  summary = "Lua Web Server Library",
  detailed = [[
    Xavante is a Lua HTTP 1.1 Web server that uses a modular architecture based on URI mapped handlers.
    This rock installs Xavante as a library that other applications can use.
  ]],
  license = "MIT/X11",
  homepage = "http://www.keplerproject.org/xavante"
}

dependencies = { 'luasocket >= 2.1', 'copas >= 1.2.0', 'luafilesystem >= 1.6.0' }

source = {
  url = "https://github.com/keplerproject/xavante/archive/v2.3.0.tar.gz",
  dir = "xavante-2.3.0",
}

local XAVANTE_LUAS = { "src/xavante/cgiluahandler.lua", 
	         "src/xavante/encoding.lua",
	         "src/xavante/filehandler.lua", 
	         "src/xavante/httpd.lua", 
	         "src/xavante/mime.lua", 
	         "src/xavante/patternhandler.lua", 
	         "src/xavante/redirecthandler.lua", 
	         "src/xavante/vhostshandler.lua", 
	         "src/xavante/indexhandler.lua", 
	         "src/xavante/urlhandler.lua", 
	         "src/xavante/ruleshandler.lua" }

build = {
   type = "builtin",
   modules = {
     sajax = "src/sajax/sajax.lua",
     xavante = "src/xavante/xavante.lua"
   }
}

for i = 1,#(XAVANTE_LUAS) do
   local src = XAVANTE_LUAS[i]
   local mod = "xavante." .. src:match("^src/xavante/([^%.]+)%.lua")
   build.modules[mod] = src 
end
