
local grammar = require "cosmo.grammar"

module(..., package.seeall)

local function get_selector(env, selector)
  selector = string.sub(selector, 2, #selector)
  local parts = {}
  for w in string.gmatch(selector, "[^|]+") do
    local n = tonumber(w)
    if n then
       env = env[n]
    else
       env = env[w]
    end
  end
  return env
end

local insert = table.insert
local concat = table.concat

local function fill_text(state, text)
  insert(state.out, text)
end

local function fill_template_application(state, selector, args, first_subtemplate, 
					 subtemplates)
   local fill = state.fill
   local env, out = state.env, state.out
   subtemplates = subtemplates or {}
   if first_subtemplate ~= "" then table.insert(subtemplates, 1, first_subtemplate) end
   selector = get_selector(env, selector) or selector
   if #subtemplates == 0 then
      if args and args ~= "" then
	 if type(selector) == 'function' then
	    selector = selector(loadstring("local env = (...); return " .. args)(env), false)
	 end
	 insert(out, tostring(selector))
      else
	 if type(selector) == 'function' then
	    insert(out, tostring(selector()))
	 else
	    insert(out, tostring(selector))
	 end
      end
   else
      if args and args ~= "" then
	 args = loadstring("local env = (...); return " .. args)(env)
	 for e in coroutine.wrap(selector), args, true do
	    if type(e) ~= "table" then
	       e = { it = tostring(e) }
	    end
	    if not getmetatable(e) then setmetatable(e, { __index = env }) end
	    insert(out, fill(subtemplates[rawget(e, '_template') or 1], e, fill))
	 end
      else
	 if type(selector) == 'table' then
	    for _, e in ipairs(selector) do
	       if type(e) ~= "table" then
		  e = { it = tostring(e) }
	       end
	       if not getmetatable(e) then setmetatable(e, { __index = env }) end
	       insert(out, fill(subtemplates[rawget(e, '_template') or 1], e, fill))
	    end
	 else
	    for e in coroutine.wrap(selector), nil, true do
	       if type(e) ~= "table" then
		  e = { it = tostring(e) }
	       end
	       if not getmetatable(e) then setmetatable(e, { __index = env }) end
	       insert(out, fill(subtemplates[rawget(e, '_template') or 1], e, fill))
	    end
	 end
      end
   end
end

local function fill_template(state, compiled_parts)
   return concat(state.out)
end

local interpreter = grammar.cosmo_compiler{ text = fill_text,
   template_application = fill_template_application, 
   template = fill_template }

function fill(template, env, subtemplate_fill)
   subtemplate_fill = subtemplate_fill or fill
   local start = template:match("^(%[=*%[)")
   if start then template = template:sub(#start + 1, #template - #start) end
   local out = {}
   if type(env) == "string" then env = { it = env } end
   return interpreter:match(template, 1, { env = env, out = out, fill = subtemplate_fill })
end
