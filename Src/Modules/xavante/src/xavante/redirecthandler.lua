-------------------------------------------------------------------------------
-- Xavante redirect handler
--
-- Authors: Javier Guerra and Andre Carregal
-- Copyright (c) 2004-2007 Kepler Project
--
-- $Id: redirecthandler.lua,v 1.11 2008/07/25 18:40:31 mascarenhas Exp $
-------------------------------------------------------------------------------
local url = require "socket.url"
local httpd = require "xavante.httpd"

local _M = {}

-- params = { dest, action }
-- dest can be of three kinds:
--      absolute: begins with protocol string or '/', the entire path is replaced with dest
--      concat: begins with ':', dest is appended to the path
--      relative: dest is appended to the dirname of the path
-- if used with patternhandler, dest can use the captures with %1, %2 etc.
-- action can be "redirect" or "rewrite", default is "rewrite", except when
--      dest starts with a protocol string
local function redirect (req, res, dest, action, cap)
  dest = string.gsub(dest, "%%(%d)", function (capn) return cap[tonumber(capn)] or "" end)
  dest = string.gsub(dest, "%%%%", "%")

  local path = req.parsed_url.path
  local pfx = string.sub (dest, 1,1)

  if pfx == "/" then
    path = dest
  elseif pfx == ":" then
    path = path .. string.sub (dest, 2)
  elseif dest:find("^[%w]+://") then
    path = dest
    action = "redirect"
  else
    path = string.gsub (path, "/[^/]*$", "") .. "/" .. dest
  end

  local path, query = path:match("^([^?]+)%??(.*)$")
  if query and query:len() > 0 then
	  if req.parsed_url.query and req.parsed_url.query:len() > 0 then
		  query = '&'..query
	  else
		  query = '?'..query
	  end
  end
  req.parsed_url.path = path
  req.built_url = url.build (req.parsed_url) .. (query or "")
  req.cmd_url = string.gsub (req.built_url, "^[^:]+://[^/]+", "")

  if action == "redirect" then
    httpd.redirect(res, path .. (query or ""))
    return res
  elseif type(action) == "function" then
    return action(req, res, cap)
  else
    return "reparse"
  end
end

function _M.makeHandler (params)
  return function (req, res, cap)
           return redirect (req, res, params[1], params[2], cap)
         end
end

return _M