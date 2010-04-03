local util = require "wsapi.util"

local date = os.date
local format = string.format

module("wsapi.response", package.seeall)

local function write(self, s)
  if type(s) == "string" then
    table.insert(self.body, s)
  else
    s = table.concat(s)
    table.insert(self.body, s)
  end
  self.length = self.length + #s
end

local function finish(self)
  self.headers["Content-Length"] = self.length
  return self.status, self.headers,
    coroutine.wrap(function ()
		     for _, s in ipairs(self.body) do
		       coroutine.yield(s)
		     end
		   end)
end

local function optional (what, name)
  if name ~= nil and name ~= "" then
    return format("; %s=%s", what, name)
  else
    return ""
  end
end

local function make_cookie(name, value)
  local options = {}
  if type(value) == "table" then
    options = value
    value = value.value
  end
  local cookie = name .. "=" .. util.url_encode(value)
  if options.expires then
    local t = date("!%A, %d-%b-%Y %H:%M:%S GMT", options.expires)
    cookie = cookie .. optional("expires", t)
  end
  cookie = cookie .. optional("path", options.path)
  cookie = cookie .. optional("domain", options.domain)
  cookie = cookie .. optional("secure", options.secure)
  return cookie
end

local function set_cookie(self, name, value)
  local cookie = self.headers["Set-Cookie"]
  if type(cookie) == "table" then
    table.insert(self.headers["Set-Cookie"], make_cookie(name, value))
  elseif type(cookie) == "string" then
    self.headers["Set-Cookie"] = { cookie, make_cookie(name, value) }
  else
    self.headers["Set-Cookie"] = make_cookie(name, value)
  end
end

local function delete_cookie(self, name, path)
  self:set_cookie(name, { value =  "xxx", expires = 1, path = path })
end

function new(status, headers, body)
  status = status or 200
  headers = headers or {}
  if not headers["Content-Type"] then
    headers["Content-Type"] = "text/html"
  end
  body = body or function () return nil end
  
  local resp = { status = status, headers = headers, body = {}, cookies = {}, length = 0 }
  local s = body()
  while s do
    write(resp, s)
    s = body()
  end

  resp.write = write
  resp.finish = finish
  resp.set_cookie = set_cookie
  resp.delete_cookie = delete_cookie

  setmetatable(resp, { __index = headers, __newindex = headers })
  return resp
end

