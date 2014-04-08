local util = require "wsapi.util"

local date = os.date
local format = string.format

local _M = {}

local methods = {}
methods.__index = methods

_M.methods = methods

local unpack = table.unpack or unpack

function methods:write(...)
  for _, s in ipairs{ ... } do
    if type(s) == "table" then
      self:write(unpack(s))
    elseif s then
      local s = tostring(s)
      self.body[#self.body+1] = s
      self.length = self.length + #s
    end
  end
end

function methods:forward(url)
  self.env.PATH_INFO = url or self.env.PATH_INFO
  return "MK_FORWARD"
end

function methods:finish()
  self.headers["Content-Length"] = self.length
  return self.status, self.headers, coroutine.wrap(function ()
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

local function optional_flag(what, isset)
  if isset then
    return format("; %s", what)
  end
  return ""
end

local function make_cookie(name, value)
  local options = {}
  local t
  if type(value) == "table" then
    options = value
    value = value.value
  end
  local cookie = name .. "=" .. util.url_encode(value)
  if options.expires then
    t = date("!%A, %d-%b-%Y %H:%M:%S GMT", options.expires)
    cookie = cookie .. optional("expires", t)
  end
  if options.max_age then
    t = date("!%A, %d-%b-%Y %H:%M:%S GMT", options.max_age)
    cookie = cookie .. optional("Max-Age", t)
  end
  cookie = cookie .. optional("path", options.path)
  cookie = cookie .. optional("domain", options.domain)
  cookie = cookie .. optional_flag("secure", options.secure)
  cookie = cookie .. optional_flag("HttpOnly", options.httponly)
  return cookie
end

function methods:set_cookie(name, value)
  local cookie = self.headers["Set-Cookie"]
  if type(cookie) == "table" then
    table.insert(self.headers["Set-Cookie"], make_cookie(name, value))
  elseif type(cookie) == "string" then
    self.headers["Set-Cookie"] = { cookie, make_cookie(name, value) }
  else
    self.headers["Set-Cookie"] = make_cookie(name, value)
  end
end

function methods:delete_cookie(name, path, domain)
  self:set_cookie(name, { value =  "xxx", expires = 1, path = path, domain = domain })
end

function methods:redirect(url)
  self.status = 302
  self.headers["Location"] = url
  self.body = {}
  return self:finish()
end

function methods:content_type(type)
  self.headers["Content-Type"] = type
end

function _M.new(status, headers)
  status = status or 200
  headers = headers or {}
  if not headers["Content-Type"] then
    headers["Content-Type"] = "text/html"
  end
  return setmetatable({ status = status, headers = headers, body = {}, length = 0 }, methods)
end

return _M
