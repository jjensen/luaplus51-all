-------------------------------------------------------------------------------
-- Xavante main module
--
-- Handles HTTP 1.1 requests and responses with Copas.
-- Uses CGILua as native template engine.
--
-- See xavante/config.lua for configuration details.
--
-- Authors: Javier Guerra and Andre Carregal
-- Copyright (c) 2004 Kepler Project
--
-- $Id: xavante.lua,v 1.13 2009/03/06 23:44:23 carregal Exp $
-------------------------------------------------------------------------------

local _M = {}

local copas = require "copas"
local httpd = require "xavante.httpd"
local string = require "string"
local phandler = require "xavante.patternhandler"
local vhosts = require "xavante.vhostshandler"

-- Meta information is public even begining with an "_"
_M._COPYRIGHT   = "Copyright (C) 2004-2010 Kepler Project"
_M._DESCRIPTION = "A Copas based Lua Web server with WSAPI support"
_M._VERSION     = "Xavante 2.3.0"

local _startmessage = function (ports)
  print(string.format("Xavante started on port(s) %s", table.concat(ports, ", ")))
end

local function _buildRules(rules)
    local rules_table = {}
    for rule_n, rule in ipairs(rules) do
        local handler
        if type (rule.with) == "function" then
            if rule.params then
              handler = rule.with(rule.params)
            else
              handler = rule.with
            end
        elseif type (rule.with) == "table" then
            handler = rule.with.makeHandler(rule.params)
        else
            error("Error on config.lua. The rule has an invalid 'with' field.")
        end
        local match = rule.match
        if type(match) == "string" then
            match = {rule.match}
        end
        rules_table[rule_n] = { pattern = {}, handler = handler }
        for pat_n, pat in ipairs(match) do
        rules_table[rule_n].pattern[pat_n] = pat
        end
    end
    return rules_table
end

-------------------------------------------------------------------------------
-- Sets startup message
-------------------------------------------------------------------------------
function _M.start_message(msg)
        _startmessage = msg
end

-------------------------------------------------------------------------------
-- Register the server configuration
-------------------------------------------------------------------------------
function _M.HTTP(config)
    -- normalizes the configuration
    config.server = config.server or {host = "*", port = 80}

    local vhosts_table = {}

    if config.defaultHost then
        vhosts_table[""] = phandler(_buildRules(config.defaultHost.rules))
    end

    if type(config.virtualhosts) == "table" then
        for hostname, host in pairs(config.virtualhosts) do
            vhosts_table[hostname] = phandler(_buildRules(host.rules))
        end
    end

    httpd.register(config.server.host, config.server.port, _M._VERSION, config.server.ssl, vhosts(vhosts_table))
end

-------------------------------------------------------------------------------
-- Starts the server
-------------------------------------------------------------------------------
function _M.start(isFinished, timeout)
    _startmessage(httpd.get_ports())
    while true do
      if isFinished and isFinished() then break end
      copas.step(timeout)
    end
end

-------------------------------------------------------------------------------
-- Methods to define and return Xavante directory structure
-------------------------------------------------------------------------------

function _M.webdir()
  return _M._webdir
end

function _M.setwebdir(dir)
  _M._webdir = dir
end

return _M
