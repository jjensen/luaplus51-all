----------------------------------------------------------------------------
-- Utility functions for encoding/decoding of URLs.
--
-- @release $Id: urlcode.lua,v 1.10 2008/01/21 16:11:32 carregal Exp $
----------------------------------------------------------------------------

local ipairs, next, pairs, tonumber, type = ipairs, next, pairs, tonumber, type
local string = require"string"
local gsub = string.gsub
local strbyte, strchar, strformat, strsub = string.byte, string.char, string.format, string.sub
local tinsert = require"table".insert

local M = {}

-- Converts an hexadecimal code in the form %XX to a character
local function hexcode2char (h)
	return strchar(tonumber(h,16))
end

----------------------------------------------------------------------------
-- Decode an URL-encoded string (see RFC 2396)
----------------------------------------------------------------------------
function M.unescape (str)
	str = gsub (str, "+", " ")
	str = gsub (str, "%%(%x%x)", hexcode2char)
	str = gsub (str, "\r\n", "\n")
	return str
end

-- Converts a character to an hexadecimal code in the form %XX
local function char2hexcode (c)
	return strformat ("%%%02X", strbyte(c))
end

----------------------------------------------------------------------------
-- URL-encode a string (see RFC 2396)
----------------------------------------------------------------------------
function M.escape (str)
	str = gsub (str, "\n", "\r\n")
	str = gsub (str, "([^0-9a-zA-Z ])", char2hexcode) -- locale independent
	str = gsub (str, " ", "+")
	return str
end

----------------------------------------------------------------------------
-- Insert a (name=value) pair into table [[args]]
-- @param args Table to receive the result.
-- @param name Key for the table.
-- @param value Value for the key.
-- Multi-valued names will be represented as tables with numerical indexes
--	(in the order they came).
----------------------------------------------------------------------------
function M.insertfield (args, name, value)
	if not args[name] then
		args[name] = value
	else
		local t = type (args[name])
		if t == "string" then
			args[name] = {
				args[name],
				value,
			}
		elseif t == "table" then
			tinsert (args[name], value)
		else
			error ("CGILua fatal error (invalid args table)!")
		end
	end
end

----------------------------------------------------------------------------
-- Parse url-encoded request data 
--   (the query part of the script URL or url-encoded post data)
--
--  Each decoded (name=value) pair is inserted into table [[args]]
-- @param query String to be parsed.
-- @param args Table where to store the pairs.
----------------------------------------------------------------------------
function M.parsequery (query, args)
	if type(query) == "string" then
		local insertfield, unescape = M.insertfield, M.unescape
		gsub (query, "([^&=]+)=([^&=]*)&?",
			function (key, val)
				M.insertfield (args, unescape(key), unescape(val))
			end)
	end
end

----------------------------------------------------------------------------
-- URL-encode the elements of a table creating a string to be used in a
--   URL for passing data/parameters to another script
-- @param args Table where to extract the pairs (name=value).
-- @return String with the resulting encoding.
----------------------------------------------------------------------------
function M.encodetable (args)
	if args == nil or next(args) == nil then	 -- no args or empty args?
		return ""
	end
	local escape = M.escape
	local strp = ""
	for key, vals in pairs(args) do
		if type(vals) ~= "table" then
			vals = {vals}
		end
		for i,val in ipairs(vals) do
			strp = strp.."&"..escape(key).."="..escape(val)
		end
	end
	-- remove first & 
	return strsub(strp,2)
end

return M
