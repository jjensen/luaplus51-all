-----------------------------------------------------------------------------
-- Xavante CGILua handler
--
-- Authors: Javier Guerra and Andre Carregal
-- Copyright (c) 2004-2007 Kepler Project
--
-- $Id: cgiluahandler.lua,v 1.46 2009/05/20 22:28:42 mascarenhas Exp $
-----------------------------------------------------------------------------

require "wsapi.xavante"
require "wsapi.common"
require "kepler_init"

module ("xavante.cgiluahandler", package.seeall)

local function sapi_loader(wsapi_env, reload)
  wsapi.common.normalize_paths(wsapi_env)
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
  for _, global in ipairs(RINGS_CGILUA_GLOBALS) do
    bootstrap = bootstrap .. 
      "_, _G[\"" .. global .. "\"] = remotedostring(\"return _G['" ..
      global .. "']\")\n"
  end
  local app = wsapi.common.load_isolated_launcher(wsapi_env.PATH_TRANSLATED, "wsapi.sapi", bootstrap, reload)
  return app(wsapi_env)
end 

-------------------------------------------------------------------------------
-- Returns the CGILua handler
-------------------------------------------------------------------------------
function makeHandler (diskpath, reload)
   return wsapi.xavante.makeHandler(function (wsapi_env) return sapi_loader(wsapi_env, reload) end, nil, diskpath, diskpath)
end
