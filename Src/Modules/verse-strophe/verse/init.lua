--[[                 Verse XMPP Library                            ]]--
--[[
  - (C) Copyright 2008 Matthew Wild <me@matthewwild.co.uk>
  -
  - This program is free software; you can redistribute it and/or
  - modify it under the terms of the GNU General Public License
  - as published by the Free Software Foundation; either version 2
  - of the License, or (at your option) any later version.
  - 
  - This program is distributed in the hope that it will be useful,
  - but WITHOUT ANY WARRANTY; without even the implied warranty of
  - MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  - GNU General Public License for more details.
  - 
  - You should have received a copy of the GNU General Public License
  - along with this program (in a file named COPYING). If not, write to 
  - the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, 
  - Boston, MA  02110-1301, USA.
  - 
  - For information about purchasing an exemption for use with
  - proprietary software under terms other than the above, please
  - contact <contact@heavy-horse.co.uk>.
  -
  ------------------------------------------------------------------------------]]


require "strophe"
local tostring = tostring
local type = type
local setmetatable = setmetatable
local getmetatable = getmetatable
local format = string.format
local pairs = pairs
local strophe = strophe;
local newproxy = newproxy;
local table_insert = table.insert;

module("verse", package.seeall)

------------ Stanza Management ------------

local stanza_mt = {};
local log_level = strophe.XMPP_LEVEL_WARN;
local log_handler = nil; -- Default handler
strophe.xmpp_initialize()
local ctx;

do
	local print = print;
	ctx = strophe.xmpp_ctx_new(nil, strophe.xmpp_log_handler_new(function(level, area, msg) return (log_handler and loghandler())or (level >= (log_level or 0) and print("LOG:", level, area, msg)); end));
end
print = function (...) strophe.xmpp_info(ctx, "verse", table.concat({...}, "   ")); end
function set_log_level(level)
	if not level then log_level = strophe.XMPP_LEVEL_WARN; return; end
	log_level = strophe["XMPP_LEVEL_"..level:upper()] or strophe.XMPP_LEVEL_WARN;
end

function set_log_handler(handler)
	log_handler = handler;
end

unset_log_handler = set_log_handler;

function stanza_mt:copy()
	return new_stanza_from_obj(strophe.xmpp_stanza_copy(self.obj));
end

function stanza_mt:iq(attrs)
	return self + stanza("iq", attrs)
end
function stanza_mt:message(attrs)
	return self + stanza("message", attrs)
end
function stanza_mt:presence(attrs)
	return self + stanza("presence", attrs)
end
function stanza_mt:query(xmlns)
	return self:tag("query", { xmlns = xmlns });
end
function stanza_mt:tag(name, attrs)
	local s = stanza(name, attrs);
	(self.last_add[#self.last_add] or self):add_child(s);
	table_insert(self.last_add, s);
	return self;
end

function stanza_mt:child(s)
	(self.last_add[#self.last_add] or self):add_child(s);
	table_insert(self.last_add, s);
	return self;
end

function stanza_mt:text(text)
	local s = stanza();
	s:set_text(text);
	(self.last_add[#self.last_add] or self):add_child(s);
	return self; 
end

function stanza_mt:up()
	table.remove(self.last_add);
	return self;
end

function stanza_mt:add_child(child)
	if type(child) == "table" then
		--getmetatable(child.__gc).__gc = nil;
		--child.__gc = nil;
		strophe.xmpp_stanza_add_child(self.obj, child.obj);
		--strophe.xmpp_stanza_release(child.obj);
	else
		strophe.xmpp_stanza_add_child(self.obj, child);
	end
end

function stanza_mt:send(conn) if conn then strophe.xmpp_send(conn, self.obj); return self; else error("Connection expected as argument 1. Got nil."); end end

function stanza_mt:get_child_by_name(name)
	return new_stanza_from_obj(strophe.xmpp_stanza_get_child_by_name(self.obj, name));
end

function stanza_mt.__index(t, k)
	if stanza_mt[k] then
		return function (stanza, ...) return stanza_mt[k](stanza, ...); end
	elseif rawget(t, "obj") and strophe["xmpp_stanza_"..k] then
		return function (stanza, ...) return strophe["xmpp_stanza_"..k](stanza.obj, ...); end
	end
end

function stanza_mt:childtags()
	local i = 0;
	return function (a)
			i = i + 1
			local v = self.tags[i]
			if v then return v; end
		end, self.tags[1], i;
	                                    
end

do
	local xml_entities = { ["'"] = "&apos;", ["\""] = "&quot;", ["<"] = "&lt;", [">"] = "&gt;", ["&"] = "&amp;" };
	function xml_escape(s) return s_gsub(s, "['&<>\"]", xml_entities); end
end

local xml_escape = xml_escape;

function stanza_mt.__tostring(t)
	local children_text = "";
	for n, child in ipairs(t) do
		if type(child) == "string" then	
			children_text = children_text .. xml_escape(child);
		else
			children_text = children_text .. tostring(child);
		end
	end

	local attr_string = "";
	if t.attr then
		for k, v in pairs(t.attr) do if type(k) == "string" then attr_string = attr_string .. format(" %s='%s'", k, tostring(v)); end end
	end

	if t.name then
		return format("<%s%s>%s</%s>", t.name, attr_string, children_text, t.name);
	else
		return children_text;
	end
end

function stanza_mt.__add(s1, s2)
	local s = s1:copy();
	s:add_child(s2.obj)
	return s;
end

do
	local refcounts = {};
	local stanzas = setmetatable({}, { __mode = "v" });
	function new_stanza_from_obj(obj)
		if not obj then return nil; end
		local stanza = { obj = obj };
		stanza.__gc = newproxy(true);
		getmetatable(stanza.__gc).__gc = 	function (s)
								--if refcounts[obj] and refcounts[obj] > 0 then
									print("Lua decided to release [["..tostring(stanza:get_data()).."]][["..tostring(stanza).."]][["..tostring(stanza.obj).."]]...");
									strophe.xmpp_stanza_release(obj);
									refcounts[obj] = refcounts[obj] - 1;
								--end
							end;
		getmetatable(stanza.__gc).obj = obj;
		if obj then refcounts[obj] = (refcounts[obj] and refcounts[obj] + 1) or 1; end
		stanza.last_add = {};
		stanzas[obj] = stanza;
		return setmetatable(stanza, stanza_mt);
	end
end

function stanza(name, attrs)
	local stanza = {};
	stanza = new_stanza_from_obj(strophe.xmpp_stanza_new(ctx));
	if type(name) == "string" then
		stanza:set_name(name);
	end
	if type(attrs) == "table" then
		for k, v in pairs(attrs) do
			stanza:set_attribute(k, v);
		end
	end
	return stanza;
end

function message(attr, body)
	if not body then
		return stanza("message", attr);
	else
		return stanza("message", attr):tag("body"):text(body);
	end
end
function iq(attr)
	if attr and not attr.id then attr.id = new_id(); end
	return stanza("iq", attr or { id = new_id() });
end
function presence(attr)
	return stanza("presence", attr);
end
function text(text)
	local s = stanza();
	s:set_text(text);
	return s;
end

function stanza_mt:children()
	local iter = 	function (root_stanza, curr_stanza)
				if not curr_stanza then return new_stanza_from_obj(root_stanza:get_children()); end
				return new_stanza_from_obj(curr_stanza:get_next());
			end;
	return iter, self;
end

function stanza_mt:run_xpath(xpath)
	local ret = strophe.xmpp_stanza_run_xpath(self.obj, xpath, nil);
	if not ret then return; end
	return new_stanza_from_obj(ret);
end

do
	local id = 0;
	function new_id()
		id = id + 1;
		return "s"..id;
	end
end

------------- Protocol Management --------------

protocols = {};

setmetatable(_G, { __index = protocols });

function add_protocol(conn, name, protocol)
	for event, info in pairs(protocol.events) do
		if type(info) == "table" then
			local criteria = info[1];
			local params = {};
			
			for k,v in pairs(info) do
				if type(v) == "string" then params[k] = v; end
			end

			if (not criteria[1]) then
				local f = 	function (conn, stanza, data)
							print("Handler for ", name.."/"..event.." called");
							local t = {};
							t.from = strophe.xmpp_stanza_get_attribute(stanza, "from");
							t.type = strophe.xmpp_stanza_get_attribute(stanza, "type");
							t.to = strophe.xmpp_stanza_get_attribute(stanza, "to");
							for param, xpath in pairs(params) do
								local results = strophe.xmpp_stanza_run_xpath(stanza, xpath, nil)
								if results then
									local result = strophe.xmpp_stanza_get_children(results);
									if strophe.xmpp_stanza_get_type(result) == strophe.XMPP_STANZA_TEXT then
										t[param] = strophe.xmpp_stanza_get_text(result);
									else
										t[param] = new_stanza_from_obj(result);
										strophe.xmpp_stanza_clone(result);
									end
									strophe.xmpp_stanza_release(results);
								end
							end
							return not verse.fire_event(name, event, { conn = conn, stanza = new_stanza_from_obj(stanza), params = params }, t);
						end
				strophe.xmpp_handler_add(conn, f, criteria.xmlns, criteria.name, criteria.type);
			else
				-- We are just a post-processor
				local f = 	function (e, p)
							for k, v in pairs(criteria) do if type(k) == "string" and tostring(p[k]) ~= tostring(criteria[k]) then print("###", tostring(p[k]).." ~= "..tostring(criteria[k])); return; end end
							local t = {};
							for k, v in pairs(p) do t[k] = v; end
							for param, xpath in pairs(params) do
								local results = strophe.xmpp_stanza_run_xpath(e.stanza, xpath, nil)
								if results then
									local result = strophe.xmpp_stanza_get_children(results);
									if strophe.xmpp_stanza_get_type(result) == strophe.XMPP_STANZA_TEXT then
										t[param] = strophe.xmpp_stanza_get_text(result);
									else
										t[param] = new_stanza_from_obj(result);
										strophe.xmpp_stanza_clone(result);
									end
									strophe.xmpp_stanza_release(results);
								end
								
							end
							return not verse.fire_event(name, event, e, t);
						end				
				hook_event(criteria[1], criteria[2], f);
			end
		end
	end
end


------------ Events/hooks Management -------------

do
	local hooks = {};
	function fire_event(proto, event, data1, data2)
		print("**** Firing event:", proto, event);
		if hooks[proto] and hooks[proto][event] then
			for _, handler in ipairs(hooks[proto][event]) do
				local success, message = pcall(handler, data1, data2);
				if not success then print("ERROR: ", tostring(proto), tostring(event), tostring(message)); end
			end
		end
	end

	function hook_event(proto, event, handler)
		hooks[proto] = hooks[proto] or {};
		hooks[proto][event] = hooks[proto][event] or {};
		table_insert(hooks[proto][event], handler);
		return handler;
	end
end

-------- Connection management ------------

function connect(jid, pass, port, handler, server)
	if not jid and pass and handler then error("Please supply a jid, password and handler function", 1); end
	if not ctx then error("There was an error initialising the XMPP library"); end
	local conn = strophe.xmpp_conn_new(ctx)
	if not conn then error("There was an error initialising the connection"); end
	strophe.xmpp_conn_set_jid(conn, jid)
	strophe.xmpp_conn_set_pass(conn, pass)
	strophe.xmpp_connect_client(conn, server or jid:match("^[^@]+@([^/]+)"), port or 5222, strophe.xmpp_conn_handler_new(handler or function () end));
	for name, proto in pairs(protocols) do
		print("Adding protocol extension:", tostring(name));
		add_protocol(conn, name, proto);
	end 
	return conn;
end


function run(time)
	if time then
		strophe.xmpp_run_once(ctx, time);
	else
		strophe.xmpp_run(ctx);
	end		
end
