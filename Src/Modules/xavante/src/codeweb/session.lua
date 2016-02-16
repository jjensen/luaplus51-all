-----------------------------------------------------------------------------
-- Xavante sessions handler
--
-- Author: Javier Guerra
-- Copyright (c) 2005 Kepler Project
-----------------------------------------------------------------------------

local cookies = require "xavante.cookies"

local session = {}

local _sessions = {}
local _max_age = 3600	-- one hour
local _def_name = "XVTE_SESS"
local MIN_SID = 11111111
local MAX_SID = 99999999

local function new_sid ()
	local sid = 0
	while sid <MIN_SID or sid >MAX_SID or _sessions[sid] do
		sid = math.random (MAX_SID)
	end
	return sid
end

function session.set_max_age (mx)
	_max_age = mx
end

function session.open (req, res, name)
	name = (name or _def_name) .. "_SID"
	local sid = cookies.get (req, name) or new_sid ()
	
	_sessions [sid] = _sessions [sid] or {data={}}
	local sess = _sessions [sid]
	sess.expires = os.time() + _max_age
	
	cookies.set (res, name, sid, {expires=sess.expires})
	
	return sess.data
end

function session.close (req, res, name)
	name = (name or _def_name) .. "_SID"
	local sid = cookies.get (req, name)
	
	if sid then
		_sessions [sid] = nil
	end
	
	cookies.delete (res, name)
end

return session
