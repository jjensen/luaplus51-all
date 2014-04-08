isolated = false
local app = dofile(docroot .. "/hello.lua")
local wsx = require "wsapi.xavante"

rules = {
  {
    match = { "^/app$", "^/app/" },
    with = wsx.makeHandler(app, "/app", docroot, docroot)
  },
}
