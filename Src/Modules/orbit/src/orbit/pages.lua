
local orbit = require "orbit"
local model = require "orbit.model"
local cosmo = require "cosmo"

local io, string = io, string
local setmetatable, loadstring, setfenv = setmetatable, loadstring, setfenv
local type, error, tostring = type, error, tostring
local print, pcall, xpcall, traceback = print, pcall, xpcall, debug.traceback
local select, unpack = select, unpack

local _G = _G

module("orbit.pages", orbit.new)

local template_cache = {}

local BOM = string.char(239) .. string.char(187) .. string.char(191)

local function remove_shebang(s)
  return s:gsub("^#![^\n]+", "")
end

local function splitpath(filename)
  local path, file = string.match(filename, "^(.*)[/\\]([^/\\]*)$")
  return path, file
end

function load(filename, contents)
  filename = filename or contents
  local template = template_cache[filename]
  if not template then
     if not contents then
       local file = io.open(filename)
       if not file then
	 return nil
       end
       contents = file:read("*a")
       file:close()
       if contents:sub(1,3) == BOM then contents = contens:sub(4) end
     end
     template = cosmo.compile(remove_shebang(contents))
     template_cache[filename] = template
  end
  return template
end

local function env_index(env, key)
  local val = _G[key]
  if not val and type(key) == "string" then
    local template = 
      load(env.web.real_path .. "/" .. key .. ".op")
    if not template then return nil end
    return function (arg)
	     arg = arg or {}
	     if arg[1] then arg.it = arg[1] end
	     local subt_env = setmetatable(arg, { __index = env })
	     return template(subt_env)
	   end
  end
  return val
end

local function abort(res)
  error{ abort, res or "abort" }
end

local function make_env(web, initial)
  local env = setmetatable(initial or {}, { __index = env_index })
  env._G = env
  env.app = _G
  env.web = web
  env.finish = abort
  function env.lua(arg)
    local f, err = loadstring(arg[1])
    if not f then error(err .. " in \n" .. arg[1]) end
    setfenv(f, env)
    local ok, res = pcall(f)
    if not ok and (type(res)~= "table" or res[1] ~= abort) then 
      error(res .. " in \n" .. arg[1]) 
    elseif ok then
      return res or ""
    else
      abort(res[2])
    end
  end
  env["if"] = function (arg)
		if type(arg[1]) == "function" then arg[1] = arg[1](select(2, unpack(arg))) end
		if arg[1] then
		  cosmo.yield{ it = arg[1], _template = 1 }
		else
		  cosmo.yield{ _template = 2 }
		end
	      end
  function env.redirect(target)
    if type(target) == "table" then target = target[1] end
    web:redirect(target)
    abort()
  end
  function env.fill(arg)
    cosmo.yield(arg[1])
  end
  function env.link(arg)
    local url = arg[1]
    arg[1] = nil
    return web:link(url, arg)
  end
  function env.static_link(arg)
    return web:static_link(arg[1])
  end
  function env.include(name, subt_env)
    local filename
    if type(name) == "table" then 
      name = name[1] 
      subt_env = name[2]
    end
    if name:sub(1, 1) == "/" then
      filename = web.doc_root .. name
    else
      filename = web.real_path .. "/" .. name
    end
    local template = load(filename)
    if not template then return "" end
    if subt_env then
      if type(subt_env) ~= "table" then subt_env = { it = subt_env } end
      subt_env = setmetatable(subt_env, { __index = env })
    else
      subt_env = env
    end
    return template(subt_env)
  end
  function env.forward(...)
    abort(env.include(...))
  end
  env.mapper = model.new()
  function env.model(name, dao)
    if type(name) == "table" then
      name, dao = name[1], name[2]
    end
    return env.mapper:new(name, dao)
  end
  env.recycle = model.recycle
  return env
end

function fill(web, template, env)
  if template then
    local ok, res = xpcall(function () return template(make_env(web, env)) end,
			   function (msg) 
			     if type(msg) == "table" and msg[1] == abort then 
			       return msg
			     else 
			       return traceback(msg) 
			     end
			   end)
    if not ok and (type(res) ~= "table" or res[1] ~= abort) then
      error(res)
    elseif ok then
      return res
    else
      return res[2]
    end
  end
end

function handle_get(web)
  local filename = web.path_translated
  web.real_path = splitpath(filename)
  local res = fill(web, load(filename))
  if res then
    return res
  else
     web.status = 404
     return [[<html>
	      <head><title>Not Found</title></head>
	      <body><p>Not found!</p></body></html>]]
  end
end

handle_post = handle_get

return _M
