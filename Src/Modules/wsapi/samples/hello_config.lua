isolated = false
local app = dofile(docroot .. "/hello.lua")

rules = {
  {
    match = { "^/app$", "^/app/" },
    with = wsapi.xavante.makeHandler(app, "/app", docroot, docroot)
  },
}
