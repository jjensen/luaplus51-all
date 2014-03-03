-------------------------------------------------------------------------------
-- Xavante configuration file.
--
--
-- Authors: Javier Guerra and Andre Carregal
-- Copyright (c) 2004-2007 Kepler Project
---
-- $Id: config.lua,v 1.1 2007/10/31 17:50:13 carregal Exp $
------------------------------------------------------------------------------

local xavante = require "xavante.httpd"

local hvhost = require "xavante.vhostshandler"
local hurl = require "xavante.urlhandler"
local hindex = require "xavante.indexhandler"
local hfile = require "xavante.filehandler"
local hcgilua = require "xavante.cgiluahandler"


xavante.handle_request = hvhost {
	[""] = hurl {
		["/"] = hindex ("/cgi/index.lp"),
		["/cgi/"] = hcgilua.makeHandler (XAVANTE_WEB),
		["/img/"] = hfile (XAVANTE_WEB.."/img"),
	}
}

xavante.register ("*", 8080, "Xavante 1.3")
