---------------------------------------------------------------------
-- XML-RPC over HTTP.
-- See Copyright Notice in license.html
-- $Id: http.lua,v 1.2 2004/09/27 16:39:37 tomas Exp $
---------------------------------------------------------------------

require"socket.http"
local xmlrpc = require"xmlrpc"

module("xmlrpc.http", package.seeall)

---------------------------------------------------------------------
-- Call a remote method.
-- @param url String with the location of the server.
-- @param method String with the name of the method to be called.
-- @return Table with the response (could be a `fault' or a `params'
--	XML-RPC element).
---------------------------------------------------------------------
function call (url, method, ...)
	local encode = xmlrpc.clEncode (method, unpack (arg))
    local t = {}
	local ret, code, headers, status = socket.http.request {
		url = url,
		source = ltn12.source.string(encode),
		headers = {
            ["content-length"] = string.len(encode),
			["User-agent"] = "LuaXMLRPC",
			["content-type"] = "text/xml",
		},
		method = "POST",
        sink = ltn12.sink.table(t),
	}
	if tonumber (code) == 200 then
		return xmlrpc.clDecode (table.concat(t))
	else
		error (err or code)
	end
end
