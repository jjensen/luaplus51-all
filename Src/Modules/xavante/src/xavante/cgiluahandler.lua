-----------------------------------------------------------------------------
-- Xavante CGILua handler
--
-- Authors: Javier Guerra and Andre Carregal
-- Copyright (c) 2004-2007 Kepler Project
--
-- $Id: cgiluahandler.lua,v 1.46 2009/05/20 22:28:42 mascarenhas Exp $
-----------------------------------------------------------------------------

local xavante = require "wsapi.xavante"
local common = require "wsapi.common"

local _M = {}

local bootstrap = [[
  function print(...)
    remotedostring("print(...)", ...)
  end

  io.stdout = {
     write = function (...)
               remotedostring("io.write(...)", ...)
             end
   }

  io.stderr = {
    write = function (...)
              remotedostring("io.stderr(...)", ...)
            end
  }
]]

-------------------------------------------------------------------------------
-- Returns the CGILua handler
-------------------------------------------------------------------------------
function _M.makeHandler (diskpath, params)
   params = setmetatable(params or {}, { __index = { modname = "wsapi.sapi",
      bootstrap = bootstrap } })
   local sapi_loader = common.make_isolated_launcher(params)
   return xavante.makeHandler(sapi_loader, nil, diskpath)
end

return _M
