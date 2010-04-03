local http = require 'xmlrpc.http'
local base = _G
local table = require 'table'
module(...)

local __proxymetatable =
{
	__index = function(t, key)
		t._function[#t._function + 1] = key
		return t
	end,

	__call = function(t, ...)
		local func = table.concat(t._function, '.')
		t._function = {}
		return http.call(t._url, func, ...)
	end
}

base.setmetatable(base.getfenv(1), { __call = function(self, url)
	return base.setmetatable( { _url = url, _function = {} }, __proxymetatable )
end})
