
local mk = require "mk"
local util = require "mk.util"
local blocks = require "mk.blocks"
local themes = require "mk.themes"

local _M = {}

_M._NAME = "orbit"
_M._VERSION = "3.0"
_M._COPYRIGHT = "Copyright (C) 2007-2009 Kepler Project, Copyright (C) 2010 Fabio Mascarenhas"
_M._DESCRIPTION = "MVC Web Development for the Kepler platform"

_M.methods = {}
_M.methods.__index = _M.methods

function _M.new(app)
  local app = mk.new(app)
  for k, v in pairs(_M.methods) do
    app[k] = v
  end
  app.reparse = "MK_FORWARD"
  app.blocks = { protos = util.merge(blocks, app.blocks), instances = {} }
  app:load_config()
  app:load_themes()
  app:connect_mapper()
  app.models = { types = {} }
  app.plugins = {}
  app.routes = {}
  app.forms = {}
  app.js = {}
  app.css = {}
  app:load_plugins()
  app:connect_blocks()
  app:connect_routes()
  return app
end

function _M.methods:load_config()
  self.config = self.config or util.loadin(self.app_path .. "/config.lua")
  if not self.config then
    error("cannot find config.lua in " .. self.app_path)
  end
end

function _M.methods:load_themes()
  if self.config.theme then
    self.theme = themes.new{ name = self.config.theme, 
                             path = self.app_path .. "/themes" }
    if not self.theme then
      error("theme " .. self.config.theme .. " not found")
    end
  end
  if self.config.admin_theme then
    self.admin_theme = themes.new{ name = self.config.admin_theme, 
                                   path = self.app_path .. "/themes" }
    if not self.admin_theme then
      error("theme " .. self.config.admin_theme .. " not found")
    end
  end
  self.admin_theme = self.admin_theme or self.theme
  if self.theme then
    self.theme.blocks = self.blocks.instances
  end
  if self.admin_theme then
    self.admin_theme.blocks = self.blocks.instances
  end
end

function _M.methods:connect_mapper()
  self.mapper = self.mapper or { default = true, 
                                 logging = true,
                                 schema = { entities = {} } }
  if self.config.database then
    local luasql = require("luasql." .. self.config.database.driver)
    local env = luasql[self.config.database.driver]()
    self.mapper.conn = env:connect(unpack(self.config.database.connection))
    self.mapper.driver = self.config.database.driver
  end
end

function _M.methods:connect_blocks()
  for name, proto in pairs(self.config.blocks) do
    self.blocks.instances[name] = self.blocks.protos[proto[1]](self, proto.args, self:block_template(name, proto.engine))
  end
end

function _M.methods:connect_routes()
  for _, route in ipairs(self.routes) do
    self["dispatch_" .. route.method](self, route.name, route.pattern, route.handler and 
				      self:wrap(function (...) return route.handler(self, ...) end))
  end
end

function _M.methods:layout(req, res, inner_html)
  local layout_template = self.theme:load("layout.html", self.engine)
  if layout_template then
    return layout_template:render(req, res, { inner = inner_html })
  else
    return inner_html
  end
end

function _M.methods:admin_layout(req, res, inner_html)
  local layout_template = self.admin_theme:load("layout.html", self.engine)
  if layout_template then
    return layout_template:render(req, res, { inner = inner_html })
  else
    return inner_html
  end
end

function _M.methods:load_plugins()
  for _, file in ipairs(self.config.plugins or {}) do
    local plugin = dofile(self.app_path .. "/plugins/" .. file)
    self.plugins[plugin.name] = plugin.new(self)
  end
end

function _M.methods:block_template(block, engine)
  local tmpl, err = self.theme:load("blocks/" .. block .. ".html", engine or self.engine)
  return tmpl
end

function _M.methods:htmlify(...)
  local patterns = { ... }
  for _, patt in ipairs(patterns) do
    if type(patt) == "function" then
      util.htmlify(patt)
    else
      for name, func in pairs(self) do
        if string.match(name, "^" .. patt .. "$") and type(func) == "function" then
          htmlify_func(func)
        end
      end
    end
  end
end

function _M.methods:model(...)
  if self.mapper.default then
    local model = require "orbit.model"
    local mapper = model.new()
    mapper.conn, mapper.driver, mapper.logging, mapper.schema = 
      self.mapper.conn, model.drivers[self.mapper.driver or "sqlite3"], 
      self.mapper.logging, self.mapper.schema
    self.mapper = mapper
  end
  return self.mapper:new(...)
end

function _M.methods:page(req, res, name, env)
  local pages = require "orbit.pages"
  local filename
  if name:sub(1, 1) == "/" then
    filename = req.doc_root .. name
  else
    filename = req.app_path .. "/" .. name
  end
  local template = pages.load(filename)
  if template then
    return pages.fill(req, res, template, env)
  end
end

function _M.methods:page_inline(req, res, contents, env)
  local pages = require "orbit.pages"
  local template = pages.load(nil, contents)
  if template then
    return pages.fill(req, res, template, env)
  end
end

return _M
