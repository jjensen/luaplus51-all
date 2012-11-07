-- Copyright (c) 2011-2012 by Robert G. Jakabosky <bobby@neoawareness.com>
--
-- Permission is hereby granted, free of charge, to any person obtaining a copy
-- of this software and associated documentation files (the "Software"), to deal
-- in the Software without restriction, including without limitation the rights
-- to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
-- copies of the Software, and to permit persons to whom the Software is
-- furnished to do so, subject to the following conditions:
--
-- The above copyright notice and this permission notice shall be included in
-- all copies or substantial portions of the Software.
--
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
-- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
-- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
-- AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
-- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
-- OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
-- THE SOFTWARE.

local ltraverse = require"luatraverse"
local sformat = string.format
local have_getsize = false
local getsize = function()
	return 0
end
if not jit then
	getsize = require"getsize"
	have_getsize = true
end

local _M = {}

function _M.dump_stats(file)
	local type_cnts = {}
	local function type_inc(t)
		type_cnts[t] = (type_cnts[t] or 0) + 1
	end
	local type_mem = {}
	local function type_mem_inc(v)
		if not have_getsize then return end
		local t = type(v)
		local s = getsize(v)
		type_mem[t] = (type_mem[t] or 0) + s
	end
	-- build metatable->type map for userdata type detection.
	local ud_types = {}
	local reg = debug.getregistry()
	for k,v in pairs(reg) do
		if type(k) == 'string' and type(v) == 'table' then
			ud_types[v] = k
		end
	end
	local function ud_type(ud)
		return ud_types[debug.getmetatable(ud)] or "<unknown>"
	end
	local str_data = 0
	local funcs = {
	["edge"] = function(from, to, how, name)
		type_inc"edges"
	end,
	["table"] = function(v)
		type_inc"table"
		type_mem_inc(v)
	end,
	["string"] = function(v)
		type_inc"string"
		str_data = str_data + #v
		type_mem_inc(v)
	end,
	["userdata"] = function(v)
		type_inc"userdata"
		type_inc(ud_type(v))
		type_mem_inc(v)
	end,
	["cdata"] = function(v)
		type_inc"cdata"
	end,
	["func"] = function(v)
		type_inc"function"
		type_mem_inc(v)
	end,
	["thread"] = function(v)
		type_inc"thread"
		type_mem_inc(v)
	end,
	}
	local ignores = {}
	for k,v in pairs(funcs) do
		ignores[#ignores + 1] = k
		ignores[#ignores + 1] = v
	end
	ignores[#ignores + 1] = type_cnts
	ignores[#ignores + 1] = funcs
	ignores[#ignores + 1] = ignores

	ltraverse.traverse(funcs, ignores)

	local fd = file
	if type(file) == 'string' then
		fd = io.open(filename, "w")
	end
	fd:write(sformat("memory = %i bytes\n", collectgarbage"count" * 1024))
	fd:write(sformat("str_data = %i\n", str_data))
	fd:write(sformat("object type counts:\n"))
	for t,cnt in pairs(type_cnts) do
		fd:write(sformat("  %9s = %9i\n", t, cnt))
	end
	fd:write("\n")
	if have_getsize then
		fd:write(sformat("per type memory usage:\n"))
		local total = 0
		for t,mem in pairs(type_mem) do
			total = total + mem
			fd:write(sformat("  %9s = %9i bytes\n", t, mem))
		end
		fd:write(sformat("total: %9i bytes\n", total))
		fd:write("\n")
	end
	--[[
	fd:write("LUA_REGISTRY dump:\n")
	for k,v in pairs(reg) do
		fd:write(tostring(k),'=', tostring(v),'\n')
	end
	--]]
	if type(file) == 'string' then
		fd:close()
	end
end

return _M
