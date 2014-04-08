-----------------------------------------------------------------------------
-- wsapi.common - common functionality for adapters and launchers
--
-- Author: Fabio Mascarenhas
-- Copyright (c) 2007 Kepler Project
--
-----------------------------------------------------------------------------

local os = require "os"
local string = require "string"
local io = require "io"
local table = require "table"
local debug = require "debug"

local wsapi = require "wsapi"
local lfs = require "lfs"

local tostring, tonumber, pairs, ipairs, error, type, pcall, xpcall, setmetatable, dofile, rawget, rawset, assert, loadfile =
  tostring, tonumber, pairs, ipairs, error, type, pcall, xpcall, setmetatable, dofile, rawget, rawset, assert, loadfile

local _, ringer = pcall(require, "wsapi.ringer")

local pcall = pcall
local xpcall = xpcall

if _VERSION < "Lua 5.2" then
	local coxpcall = require "coxpcall"
	pcall = coxpcall.pcall
	xpcall = coxpcall.xpcall
end

local package = package

local _M = {}

-- HTTP status codes
_M.status_codes = {
   [100] = "Continue",
   [101] = "Switching Protocols",
   [200] = "OK",
   [201] = "Created",
   [202] = "Accepted",
   [203] = "Non-Authoritative Information",
   [204] = "No Content",
   [205] = "Reset Content",
   [206] = "Partial Content",
   [300] = "Multiple Choices",
   [301] = "Moved Permanently",
   [302] = "Found",
   [303] = "See Other",
   [304] = "Not Modified",
   [305] = "Use Proxy",
   [307] = "Temporary Redirect",
   [400] = "Bad Request",
   [401] = "Unauthorized",
   [402] = "Payment Required",
   [403] = "Forbidden",
   [404] = "Not Found",
   [405] = "Method Not Allowed",
   [406] = "Not Acceptable",
   [407] = "Proxy Authentication Required",
   [408] = "Request Time-out",
   [409] = "Conflict",
   [410] = "Gone",
   [411] = "Length Required",
   [412] = "Precondition Failed",
   [413] = "Request Entity Too Large",
   [414] = "Request-URI Too Large",
   [415] = "Unsupported Media Type",
   [416] = "Requested range not satisfiable",
   [417] = "Expectation Failed",
   [500] = "Internal Server Error",
   [501] = "Not Implemented",
   [502] = "Bad Gateway",
   [503] = "Service Unavailable",
   [504] = "Gateway Time-out",
   [505] = "HTTP Version not supported",
}

-- Makes an index metamethod for the environment, from
-- a function that returns the value of a server variable
-- a metamethod lets us do "on-demand" loading of the WSAPI
-- environment, and provides the invariant the the WSAPI
-- environment returns the empty string instead of nil for
-- variables that do not exist
function _M.sv_index(func)
  if type(func) == "table" then
    return function (env, n)
             local v = func[n]
             env[n] = v or ""
             return v or ""
           end
  else
    return function (env, n)
             local v = func(n)
             env[n] = v or ""
             return v or ""
           end
  end
end

-- Makes an wsapi_env.input object from a low-level input
-- object and the name of the method to read from this object
function _M.input_maker(obj, read_method)
   local input = {}
   local read = obj[read_method or "read"]

   function input:read(n)
     n = n or self.length or 0
     if n > 0 then return read(obj, n) end
   end
   return input
end

-- Windows only: sets stdin and stdout to binary mode so
-- sending and receiving binary data works with CGI
function _M.setmode()
   pcall(lfs.setmode, io.stdin, "binary")
   pcall(lfs.setmode, io.stdout, "binary")
end

-- Returns the actual WSAPI handler (a function) for the
-- WSAPI application, whether it is a table, the name of a Lua
-- module, a Lua script, or the function itself
function _M.normalize_app(app_run, is_file)
   local t = type(app_run)
   if t == "function" then
      return app_run
   elseif t == "table" then
      return app_run.run
   elseif t == "string" then
      if is_file then
         return _M.normalize_app(dofile(app_run))
      else
         return _M.normalize_app(require(app_run))
      end
   else
      error("not a valid WSAPI application")
   end
end

-- Sends the respose body through the "out" pipe, using
-- the provided write method. Gets the body from the
-- response iterator
function _M.send_content(out, res_iter, write_method)
   local write = out[write_method or "write"]
   local flush = out.flush
   local ok, res = xpcall(res_iter, debug.traceback)
   while ok and res do
      write(out, res)
      if flush then flush(out) end
      ok, res = xpcall(res_iter, debug.traceback)
   end
   if not ok then
      write(out,
            "======== WSAPI ERROR DURING RESPONSE PROCESSING: \n<pre>" ..
              tostring(res) .. "\n</pre>")
   end
end

-- Sends the complete response through the "out" pipe,
-- using the provided write method
function _M.send_output(out, status, headers, res_iter, write_method, res_line)
   local write = out[write_method or "write"]
   if type(status) == "number" or status:match("^%d+$") then
     status = status .. " " .. _M.status_codes[tonumber(status)]
   end
   if res_line then
     write(out, "HTTP/1.1 " .. (status or "500 Internal Server Error") .. "\r\n")
   else
     write(out, "Status: " .. (status or "500 Internal Server Error") .. "\r\n")
   end
   for h, v in pairs(headers or {}) do
      if type(v) ~= "table" then
         write(out, h .. ": " .. tostring(v) .. "\r\n")
      else
         for _, v in ipairs(v) do
            write(out, h .. ": " .. tostring(v) .. "\r\n")
         end
      end
   end
   write(out, "\r\n")
   _M.send_content(out, res_iter, write_method)
end

-- Formats the standard error message for WSAPI applications
function _M.error_html(msg)
   return string.format([[
        <html>
        <head><title>WSAPI Error in Application</title></head>
        <body>
        <p>There was an error in the specified application.
        The full error message follows:</p>
<pre>
%s
</pre>
        </body>
        </html>
      ]], tostring(msg))
end

-- Body for a 500 response
function _M.status_500_html(msg)
   return _M.error_html(msg)
end

-- Body for a 404 response
function _M.status_404_html(msg)
   return string.format([[
        <html>
        <head><title>Resource not found</title></head>
        <body>
        <p>%s</p>
        </body>
        </html>
      ]], tostring(msg))
end

function _M.status_200_html(msg)
   return string.format([[
        <html>
        <head><title>Resource not found</title></head>
        <body>
        <p>%s</p>
        </body>
        </html>
      ]], tostring(msg))
end

local function make_iterator(msg)
  local sent = false
  return function ()
           if sent then return nil
           else
             sent = true
             return msg
           end
         end
end

-- Sends an error response through the "out" pipe, replicated
-- to the "err" pipe (for logging, for example)
-- msg is the error message
function _M.send_error(out, err, msg, out_method, err_method, http_response)
   local write = out[out_method or "write"]
   local write_err = err[err_method or "write"]
   write_err(err, "WSAPI error in application: " .. tostring(msg) .. "\n")
   local msg = _M.error_html(msg)
   local status, headers, res_iter = "500 Internal Server Error", {
        ["Content-Type"] = "text/html",
        ["Content-Length"] = #msg
      }, make_iterator(msg)
   _M.send_output(out, status, headers, res_iter, out_method, http_response)
   return status, headers
end

-- Sends a 404 response to the "out" pipe, "msg" is the error
-- message
function _M.send_404(out, msg, out_method, http_response)
   local write = out[out_method or "write"]
   local msg = _M.status_404_html(msg)
   local status, headers, res_iter = "404 Not Found", {
        ["Content-Type"] = "text/html",
        ["Content-Length"] = #msg
      }, make_iterator(msg)
   _M.send_output(out, status, headers, res_iter, out_method, http_response)
   return status, headers
end

-- Runs the application in the provided WSAPI environment, catching errors and
-- returning the appropriate error repsonses
function _M.run_app(app, env)
   return xpcall(function () return (_M.normalize_app(app))(env) end,
                 function (msg)
                    if type(msg) == "table" then
                       env.STATUS = msg[1]
                       return _M["status_" .. msg[1] .. "_html"](msg[2])
                    else
                       return debug.traceback(msg, 2)
                    end
                 end)
end

-- Builds an WSAPI environment from the configuration table "t"
function _M.wsapi_env(t)
   local env = {}
   setmetatable(env, { __index = _M.sv_index(t.env) })
   env.input = _M.input_maker(t.input, t.read_method)
   env.error = t.error
   env.input.length = tonumber(env.CONTENT_LENGTH) or 0
   if env.PATH_INFO == "" then env.PATH_INFO = "/" end
   return env
end

-- Runs an application with data from the configuration table "t",
-- sending the WSAPI error/not found responses in case of errors
function _M.run(app, t)
   local env = _M.wsapi_env(t)
   local ok, status, headers, res_iter =
      _M.run_app(app, env)
   if ok then
     if not headers["Content-Length"] then
       if t.http_response then
         headers["Transfer-Encoding"] = "chunked"
         local unchunked = res_iter
         res_iter = function ()
                      local msg = unchunked()
                      if msg then
                        return string.format("%x\r\n%s\r\n", #msg, msg)
                      end
                    end
       end
     end
     _M.send_output(t.output, status, headers, res_iter, t.write_method, t.http_response)
   else
     if env.STATUS == 404 then
       return _M.send_404(t.output, status, t.write_method, t.http_response)
     else
       return _M.send_error(t.output, t.error, status, t.write_method, t.err_method, t.http_response)
     end
   end
   return status, headers
end

function _M.splitpath(filename)
  local path, file = string.match(filename, "^(.*)[/\\]([^/\\]*)$")
  return path, file
end

function _M.splitext(filename)
  local modname, ext = string.match(filename, "^(.+)%.([^%.]+)$")
  if not modname then modname, ext = filename, "" end
  return modname, ext
end

-- Gets the data for file or directory "filename" if it exists:
-- path, actual file name, file name without extension, extension,
-- and modification time. If "filename" is a directory it assumes
-- that the actual file is a .lua file in this directory with
-- the same name as the directory (for example, "/foo/bar/bar.lua")
function _M.find_file(filename)
   local mode = assert(lfs.attributes(filename, "mode"))
   local path, file, modname, ext
   if mode == "directory" then
      path, modname = _M.splitpath(filename)
      path = path .. "/" .. modname
      file = modname .. ".lua"
      ext = "lua"
   elseif mode == "file" then
      path, file = _M.splitpath(filename)
      modname, ext = _M.splitext(file)
   else
      return nil
   end
   local mtime = assert(lfs.attributes(path .. "/" .. file, "modification"))
   return path, file, modname, ext, mtime
end

-- IIS appends the PATH_INFO to PATH_TRANSLATED, this function
-- corrects for that
function _M.adjust_iis_path(wsapi_env, filename)
   local script_name, ext =
      wsapi_env.SCRIPT_NAME:match("([^/%.]+)%.([^%.]+)$")
   if script_name then
      local path =
         filename:match("^(.+)" .. script_name .. "%." .. ext .. "[/\\]")
      if path then
         return path .. script_name .. "." .. ext
      else
         return filename
      end
   else
      return filename
   end
end

-- IIS appends the PATH_INFO to the DOCUMENT_ROOT, this corrects
-- for that and for virtual directories
local function not_compatible(wsapi_env, filename)
  local script_name = wsapi_env.SCRIPT_NAME
  if not filename:gsub("\\","/"):find(script_name, 1, true) then
    -- more IIS madness, down into the rabbit hole...
    local path_info = wsapi_env.PATH_INFO:gsub("/", "\\")
    wsapi_env.DOCUMENT_ROOT = filename:sub(1, #filename-#path_info)
    return true
  end
end

-- Find the actual script file in case of non-wrapped launchers
-- (http://server/cgi-bin/wsapi.cgi/bar/baz.lua/foo) and for IIS,
-- as IIS provides a wrong PATH_TRANSLATED variable
-- Corrects PATH_INFO and SCRIPT_NAME, so SCRIPT_NAME will be
-- /cgi-bin/wsapi.cgi/bar/baz.lua and PATH_INFO will be /foo
-- for the previous example
function _M.adjust_non_wrapped(wsapi_env, filename, launcher)
  if filename == "" or not_compatible(wsapi_env, filename) or
    (launcher and filename:match(launcher:gsub("%.", ".") .. "$")) then
    local path_info = wsapi_env.PATH_INFO
    local docroot = wsapi_env.DOCUMENT_ROOT
    if docroot:sub(#docroot) ~= "/" and docroot:sub(#docroot) ~= "\\" then
      docroot = docroot .. "/"
    end
    local s, e = path_info:find("[^/%.]+%.[^/%.]+", 1)
    while s do
      local filepath = path_info:sub(2, e)
        local filename
        if docroot:find("\\", 1, true) then
        filename = docroot .. filepath:gsub("/","\\")
      else
        filename = docroot .. filepath
      end
      local mode = lfs.attributes(filename, "mode")
      if not mode then
        error({ 404, "Resource " .. wsapi_env.SCRIPT_NAME .. "/" .. filepath
                 .. " not found!" }, 0)
      elseif lfs.attributes(filename, "mode") == "file" then
        wsapi_env.PATH_INFO = path_info:sub(e + 1)
        if wsapi_env.PATH_INFO == "" then wsapi_env.PATH_INFO = "/" end
        wsapi_env.SCRIPT_NAME = wsapi_env.SCRIPT_NAME .. "/" .. filepath
        return filename
      end
      s, e = path_info:find("[^/%.]+%.[^/%.]+", e + 1)
    end
    error("could not find a filename to load, check your configuration or URL")
  else return filename end
end

-- Tries to guess the correct path for the WSAPI application script,
-- correcting for misbehaving web servers (IIS), non-wrapped launchers
-- and (http://server/cgi-bin/wsapi.cgi/bar/baz.lua/foo)
function _M.normalize_paths(wsapi_env, filename, launcher, vars)
   vars = vars or { "SCRIPT_FILENAME", "PATH_TRANSLATED" }
   if not filename or filename == "" then
     for _, var in ipairs(vars) do
        filename = wsapi_env[var]
        if filename ~= "" then break end
     end
     filename = _M.adjust_non_wrapped(wsapi_env, filename, launcher)
     filename = _M.adjust_iis_path(wsapi_env, filename)
     wsapi_env.PATH_TRANSLATED = filename
     wsapi_env.SCRIPT_FILENAME = filename
   else
     wsapi_env.PATH_TRANSLATED = filename
     wsapi_env.SCRIPT_FILENAME = filename
   end
   local s, e = wsapi_env.PATH_INFO:find(wsapi_env.SCRIPT_NAME, 1, true)
   if s == 1 then
     wsapi_env.PATH_INFO = wsapi_env.PATH_INFO:sub(e+1)
     if wsapi_env.PATH_INFO == "" then wsapi_env.PATH_INFO = "/" end
   end
end

-- Tries to find the correct script to launch for the WSAPI application
function _M.find_module(wsapi_env, filename, launcher, vars)
   _M.normalize_paths(wsapi_env, filename or "", launcher, vars)
   return _M.find_file(wsapi_env.PATH_TRANSLATED)
end

-- Version of require skips searching package.path
function _M.require_file(filename, modname)
  package.loaded[modname] = true
  local res = loadfile(filename)(modname)
  if res then
    package.loaded[modname] = res
  end
  return package.loaded[modname]
end

-- Loads the script for a WSAPI application (require'ing in case of
-- a .lua script and dofile'ing it in case of other extensions),
-- returning the WSAPI handler function for this application
-- also moves the current directory to the application's path
function _M.load_wsapi(path, file, modname, ext)
  lfs.chdir(path)
  local app
  if ext == "lua" then
    app = _M.require_file(file, modname)
  else
    app = dofile(file)
  end
  return _M.normalize_app(app)
end

-- Local state and helper functions for the loader if isolated applications,
-- used in the FastCGI and Xavante WSAPI launchers
do
  local app_states = {}
  local last_collection = os.time()
  setmetatable(app_states, { __index = function (tab, app)
                                          tab[app] = { states = {} }
                                          return tab[app]
                                       end })

  -- Bootstraps a Lua state (using rings) with the provided WSAPI application
  local function bootstrap_app(path, file, modname, ext)
     local bootstrap = [=[
           _, package.path = remotedostring("return package.path")
           _, package.cpath = remotedostring("return package.cpath")
           pcall(require, "luarocks.require")
           wsapi = {}
           wsapi.app_path = [[]=] .. path .. [=[]]
     ]=]
     if ext == "lua" then
        return ringer.new(modname, bootstrap)
     else
        return ringer.new(file, bootstrap, true)
     end
  end

  -- "Garbage-collect" stale Lua states
  local function collect_states(period, ttl)
     if period and (last_collection + period < os.time()) then
        for app, app_state in pairs(app_states) do
           local new_states = {}
           for _, state in ipairs(app_state.states) do
              if ttl and (rawget(state.data, "created_at") + ttl > os.time()) then
                 table.insert(new_states, state)
              else
                 if not rawget(state.data, "status") then
                   state.app("close")
                 else
                   rawset(state.data, "cleanup", true)
                 end
              end
           end
           app_state.states = new_states
        end
        last_collection = os.time()
     end
  end

  -- Helper for the isolated launchers: find the application path and script,
  -- loads it in an isolated Lua state (reusing an existing state if one is free)
  -- and runs the application in the provided WSAPI environment
  local function wsapi_loader_isolated_helper(wsapi_env, params)
     local path, file, modname, ext, mtime =
        _M.find_module(wsapi_env, params.filename, params.launcher, params.vars)
     if params.reload then mtime = nil end
     if not path then
        error({ 404, "Resource " .. wsapi_env.SCRIPT_NAME .. " not found"})
     end
     local app = _M.load_wsapi_isolated(path, file, modname, ext, mtime)
     wsapi_env.APP_PATH = path
     return app(wsapi_env)
  end

  -- Loads a WSAPI application isolated in its own Lua state and returns
  -- the application handler (reusing an existing state if one is free)
  function _M.load_wsapi_isolated(path, file, modname, ext, mtime)
    local filename = path .. "/" .. file
    lfs.chdir(path)
    local app, data
    local app_state = app_states[filename]
    if mtime and app_state.mtime == mtime then
      for i, state in ipairs(app_state.states) do
         if not rawget(state.data, "status") then
            return state.app
         end
      end
      app, data = bootstrap_app(path, file, modname, ext)
      table.insert(app_state.states, { app = app, data = data })
    else
      for _, state in ipairs(app_state.states) do
        if not rawget(state.data, "status") then
          state.app("close")
        else
          rawset(state.data, "cleanup", true)
        end
      end
      app, data = bootstrap_app(path, file, modname, ext)
      if mtime then
        app_states[filename] = { states = { { app = app, data = data } },
                                 mtime = mtime }
      end
    end
    return app
  end

  -- Makes an WSAPI application that launches isolated WSAPI applications
  -- scripts with the provided parameters - see wsapi.fcgi for the
  -- parameters and their descriptions
  function _M.make_isolated_loader(params)
     params = params or {}
     return function (wsapi_env)
               collect_states(params.period, params.ttl)
               return wsapi_loader_isolated_helper(wsapi_env, params)
            end
  end

  function _M.wsapi_loader_isolated(wsapi_env)
     return wsapi_loader_isolated_helper(wsapi_env, {})
  end

  function _M.wsapi_loader_isolated_reload(wsapi_env)
     return wsapi_loader_isolated_helper(wsapi_env, { reload = true })
  end

end

-- Local state and helper functions for the loader if isolated dedicated
-- launchers, used in the CGILua and Orbit pages launchers
do
  local app_states = {}
  local last_collection = os.time()
  setmetatable(app_states, { __index = function (tab, app)
                                          tab[app] = { states = {} }
                                          return tab[app]
                                       end })

  -- "Garbage-collect" stale Lua states
  local function collect_states(period, ttl)
     if period and (last_collection + period < os.time()) then
        for app, app_state in pairs(app_states) do
           local new_states = {}
           for _, state in ipairs(app_state.states) do
              if ttl and (rawget(state.data, "created_at") + ttl > os.time()) then
                table.insert(new_states, state)
              else
                if not rawget(state.data, "status") then
                  state.app("close")
                else
                  rawset(state.data, "cleanup", true)
                end
              end
           end
           app_state.states = new_states
        end
        last_collection = os.time()
     end
  end

  -- Bootstraps a Lua state (using rings) with the provided launcher
  local function bootstrap_app(path, app_modname, extra)
     local bootstrap = [=[
           _, package.path = remotedostring("return package.path")
           _, package.cpath = remotedostring("return package.cpath")
           pcall(require, "luarocks.require")
           wsapi = {}
           wsapi.app_path = [[]=] .. path .. [=[]]
     ]=] .. (extra or "")
     return ringer.new(app_modname, bootstrap)
  end

  -- Loads a WSAPI application isolated in its own Lua state and returns
  -- the application handler (reusing an existing state if one is free)
  function _M.load_isolated_launcher(filename, app_modname, bootstrap, reload)
    local app, data
    local app_state = app_states[filename]
    local path, _ = _M.splitpath(filename)
    local mtime = lfs.attributes(filename, "modification")
    if not reload and app_state.mtime == mtime then
       for _, state in ipairs(app_state.states) do
          if not rawget(state.data, "status") then
             return state.app
          end
       end
       app, data = bootstrap_app(path, app_modname, bootstrap)
       table.insert(app_state.states, { app = app, data = data })
    else
       for _, state in ipairs(app_state.states) do
         if not rawget(state.data, "status") then
           state.app("close")
         else
           rawset(state.data, "cleanup", true)
         end
       end
       app, data = bootstrap_app(path, app_modname, bootstrap)
       app_states[filename] = { states = { { app = app, data = data } },
                                mtime = mtime }
    end
    return app
  end

  -- Makes an WSAPI application that launches an isolated WSAPI launcher
  -- with the provided parameters - see op.fcgi in the Orbit sources for the
  -- parameters and their descriptions
  function _M.make_isolated_launcher(params)
     params = params or {}
     return function (wsapi_env)
               collect_states(params.period, params.ttl)
               _M.normalize_paths(wsapi_env, params.filename, params.launcher, params.vars)
               local app = _M.load_isolated_launcher(wsapi_env.PATH_TRANSLATED, params.modname, params.bootstrap, params.reload)
               return app(wsapi_env)
            end
  end
end

-- Local state and helper functions for the loader of persistent applications,
-- used in the FastCGI and Xavante WSAPI launchers
do
  local apps = {}
  local last_collection = os.time()
  setmetatable(apps, { __index = function (tab, app)
                                   tab[app] = { created_at = os.time() }
                                   return tab[app]
                                 end })

  -- Bootstraps a Lua state (using rings) with the provided WSAPI application
  local function bootstrap_app(path, file, modname, ext)
     return _M.load_wsapi(path, file, modname, ext)
  end

  -- "Garbage-collect" stale Lua states
  local function collect_states(period, ttl)
     if period and (last_collection + period < os.time()) then
        for app_name, app_data in pairs(apps) do
           local new_data = { created_at = os.time() }
           if ttl and app_data.created_at + ttl > os.time() then
              new_data.app = app_data.app
           end
           apps[app_name] = new_data
        end
        last_collection = os.time()
     end
  end

  -- Loads a persistent WSAPI application Lua state and returns
  -- the application handler (reusing an existing state if one is free)
  local function load_wsapi_persistent(path, file, modname, ext, mtime)
    local filename = path .. "/" .. file
    lfs.chdir(path)
    local app
    local app_data = apps[filename]
    if mtime and app_data.mtime == mtime then
      return app_data.app
    else
      app = bootstrap_app(path, file, modname, ext)
      if mtime then
        apps[filename].app = app
        apps[filename].mtime = mtime
      end
      return app
    end
  end

  -- Helper for the persistent launchers: find the application path and script,
  -- loads and runs the application in the provided WSAPI environment
  local function wsapi_loader_persistent_helper(wsapi_env, params)
     local path, file, modname, ext, mtime =
        _M.find_module(wsapi_env, params.filename, params.launcher, params.vars)
     if params.reload then mtime = nil end
     if not path then
        error({ 404, "Resource " .. wsapi_env.SCRIPT_NAME .. " not found"})
     end
     local app = load_wsapi_persistent(path, file, modname, ext, mtime)
     wsapi_env.APP_PATH = path
     return app(wsapi_env)
  end

  -- Makes an WSAPI application that launches persistent WSAPI applications
  -- scripts with the provided parameters - see wsapi.fcgi for the
  -- parameters and their descriptions
  function _M.make_persistent_loader(params)
     params = params or {}
     return function (wsapi_env)
               collect_states(params.period, params.ttl)
               return wsapi_loader_persistent_helper(wsapi_env, params)
            end
  end
end

function _M.make_loader(params)
   params = params or { isolated = true }
   if params.isolated then
      return _M.make_isolated_loader(params)
   else
      return _M.make_persistent_loader(params)
   end
end

return _M
