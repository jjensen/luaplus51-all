local require = require

local grammar = require "cosmo.grammar"
local interpreter = require "cosmo.fill"
local loadstring = loadstring

if _VERSION ~= "Lua 5.1" then
  _ENV = setmetatable({}, { __index = _G })
else
  module(..., package.seeall)
  _ENV = _M
end

yield = coroutine.yield

local preamble = [[
    local is_callable, insert, concat, setmetatable, getmetatable, type, wrap, tostring, check_selector = ...
    local function unparse_name(parsed_selector)
      local name = parsed_selector:match("^env%%['([%%w_]+)'%%]$")
      if name then name = "$" .. name end
      return name or parsed_selector
    end
    local function prepare_env(env, parent)
      local __index = function (t, k)
                        local v = env[k]
                        if not v then
                          v = parent[k]
                        end
                        return v
                      end
      local __newindex = function (t, k, v)
                           env[k] = v
                         end
      return setmetatable({ self = env }, { __index = __index, __newindex = __newindex })
    end
    local id = function () end
    local template_func = %s
    return function (env, opts)
             opts = opts or {}
             local out = opts.out or {}
             template_func(out, env, opts)
             return concat(out, opts.delim)
           end
]]

local compiled_template = [[
    function (out, env, opts)
      if type(env) == "string" then env = { it = env } end
      $parts[=[
          insert(out, $quoted_text)
      ]=],
      [=[
          local selector_name = unparse_name($selector)
          local selector = $parsed_selector
          $if_subtemplate[==[
              local subtemplates = {}
              $subtemplates[===[
                  subtemplates[$i] = $subtemplate
              ]===]
              local default = id
              if opts.fallback then
                default = subtemplates[1]
              end
              $if_args[===[
                  check_selector(selector_name, selector)
                  for e, literal in wrap(selector), $args, true do
                    if literal then
                      insert(out, tostring(e))
                    else
                      if type(e) ~= "table" then
                        e = prepare_env({ it = tostring(e) }, env)
                      else
                        e = prepare_env(e, env)
                      end
                      (subtemplates[e.self._template or 1] or default)(out, e, opts)
                    end
                  end
              ]===],
              [===[
                  if type(selector) == 'table' then
                    for _, e in ipairs(selector) do
                      if type(e) ~= "table" then
                        e = prepare_env({ it = tostring(e) }, env)
                      else
                        e = prepare_env(e, env)
                      end
                      (subtemplates[e.self._template or 1] or default)(out, e, opts)
                    end
                  else
                    check_selector(selector_name, selector)
                    for e, literal in wrap(selector), nil, true do
                      if literal then
                        insert(out, tostring(e))
                      else
                        if type(e) ~= "table" then
                          e = prepare_env({ it = tostring(e) }, env)
                        else
                          e = prepare_env(e, env)
                        end
                        (subtemplates[e.self._template or 1] or default)(out, e, opts)
                      end
                    end
                  end
              ]===]
          ]==],
          [==[
              $if_args[===[
                  check_selector(selector_name, selector)
                  selector = selector($args, false)
                  insert(out, tostring(selector))
              ]===],
              [===[
                  if is_callable(selector) then
                    insert(out, tostring(selector()))
                  else
                    if not selector and opts.passthrough then
                      selector = selector_name
                    end
                    insert(out, tostring(selector or ""))
                  end
              ]===]
          ]==]
      ]=]
    end
]]

local function is_callable(f)
  if type(f) == "function" then return true end
  local meta = getmetatable(f)
  if meta and meta.__call then return true end
  return false
end

local function check_selector(name, selector)
  if not is_callable(selector) then
    error("selector " .. name .. " is not callable but is " .. type(selector))
  end
end

local function compile_template(chunkname, template_code)
   local template_func, err = loadstring(string.format(preamble, template_code), chunkname)
   if not template_func then
     error("syntax error when compiling template: " .. err)
   else
     return template_func(is_callable, table.insert, table.concat, setmetatable, getmetatable, type,
                          coroutine.wrap, tostring, check_selector)
   end
end

local compiler = {}

function compiler.template(template)
  assert(template.tag == "template")
  local parts = {}
  for _, part in ipairs(template.parts) do
    parts[#parts+1] = compiler[part.tag](part)
  end
  return interpreter.fill(compiled_template, { parts = parts })
end

function compiler.text(text)
  assert(text.tag == "text")
  return { _template = 1, quoted_text = string.format("%q", text.text) }
end

function compiler.appl(appl)
  assert(appl.tag == "appl")
  local selector, args, subtemplates = appl.selector, appl.args, appl.subtemplates
  local ta = { _template = 2, selector = string.format("%q", selector),
      parsed_selector = selector }
   local do_subtemplates = function ()
                             for i, subtemplate in ipairs(subtemplates) do
                               yield{ i = i, subtemplate = compiler.template(subtemplate) }
                             end
                           end
   if #subtemplates == 0 then
     if args and args ~= "" and args ~= "{}" then
       ta.if_subtemplate = { { _template = 2, if_args = { { _template = 1, args = args } } } }
     else
       ta.if_subtemplate = { { _template = 2, if_args = { { _template = 2 } } } }
     end
   else
     if args and args ~= "" and args ~= "{}" then
       ta.if_subtemplate = { { _template = 1, subtemplates = do_subtemplates,
                               if_args = { { _template = 1, args = args } } } }
     else
       ta.if_subtemplate = { { _template = 1, subtemplates = do_subtemplates,
                               if_args = { { _template = 2 } } } }
     end
   end
   return ta
end

local cache = {}
setmetatable(cache, { __index = function (tab, key)
                                   local new = {}
                                   tab[key] = new
                                   return new
                                end,
                      __mode = "v" })

function compile(template, chunkname, opts)
  opts = opts or {}
  template = template or ""
  chunkname = chunkname or template
  local compiled_template = cache[template][chunkname]
  grammar.ast = opts.parser or grammar.default
  if not compiled_template then
    compiled_template = compile_template(chunkname, compiler.template(grammar.ast:match(template)))
    cache[template][chunkname] = compiled_template
  end
  return compiled_template
end

local filled_templates = {}
setmetatable(filled_templates, { __mode = "k" })

function fill(template, env, opts)
   opts = opts or {}
   template = template or ""
   local start = template:match("^(%[=*%[)")
   if start then template = template:sub(#start + 1, #template - #start) end
   if filled_templates[template] then
      return compile(template, opts.chunkname, opts.parser)(env, opts)
   else
      filled_templates[template] = true
      return interpreter.fill(template, env, opts)
   end
end

local nop = function () end

function cond(bool, table)
   if bool then
      return function () yield(table) end
   else
      return nop
   end
end

f = compile

function c(bool)
   if bool then
      return function (table)
                return function () yield(table) end
             end
   else
      return function (table) return nop end
   end
end

function map(arg, has_block)
   if has_block then
      for _, item in ipairs(arg) do
         yield(item)
      end
   else
      return table.concat(arg)
   end
end

function inject(arg)
   yield(arg)
end

function cif(arg, has_block)
  if not has_block then error("this selector needs a block") end
  if arg[1] then
    arg._template = 1
  else
    arg._template = 2
  end
  yield(arg)
end

function concat(arg)
  local list, sep = arg[1], arg[2] or ", "
  local size = #list
  for i, e in ipairs(list) do
    if type(e) == "table" then
      if i ~= size then
        yield(e)
        yield(sep, true)
      else
        yield(e)
      end
    else
      if i ~= size then
        yield{ it = e }
        yield(sep, true)
      else
        yield{ it = e }
      end
    end
  end
end

function make_concat(list)
  return function (arg)
           local sep = (arg and arg[1]) or ", "
           local size = #list
           for i, e in ipairs(list) do
             if type(e) == "table" then
               if i ~= size then
                 yield(e)
                 yield(sep, true)
               else
                 yield(e)
               end
             else
               if i ~= size then
                 yield{ it = e }
                 yield(sep, true)
               else
                 yield{ it = e }
               end
             end
           end
         end
end

function cfor(args)
  local name, list, args = args[1], args[2], args[3]
  if type(list) == "table" then
    for i, item in ipairs(list) do
      yield({ [name] = item, i = i })
    end
  else
    for item, literal in coroutine.wrap(list), args, true do
      if literal then
        yield(item, true)
      else
        yield({ [name] = item })
      end
    end
  end
end

return _ENV
