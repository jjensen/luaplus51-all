
local lpeg = require "lpeg"
local re = require "re"

module(..., package.seeall)

function parse_selector(selector, env)
  env = env or "env"
  selector = string.sub(selector, 2, #selector)
  local parts = {}
  for w in string.gmatch(selector, "[^|]+") do
    local n = tonumber(w)
    if n then
      table.insert(parts, "[" .. n .. "]")
    else
      table.insert(parts, "['" .. w .. "']")
    end
  end
  return env .. table.concat(parts)
end

local start = "[" * lpeg.P"="^0 * "["

local longstring = lpeg.P(function (s, i)
  local l = lpeg.match(start, s, i)
  if not l then return nil end
  local p = lpeg.P("]" .. string.rep("=", l - i - 2) .. "]")
  p = (1 - p)^0 * p
  return lpeg.match(p, s, l)
end)

longstring = #("[" * lpeg.S"[=") * longstring

local alpha =  lpeg.R('__','az','AZ','\127\255') 

local n = lpeg.R'09'

local alphanum = alpha + n

local number = (lpeg.P'.' + n)^1 * (lpeg.S'eE' * lpeg.S'+-'^-1)^-1 * (alphanum)^0
number = #(n + (lpeg.P'.' * n)) * number

local shortstring = (lpeg.P'"' * ( (lpeg.P'\\' * 1) + (1 - (lpeg.S'"\n\r\f')) )^0 * lpeg.P'"') +
  (lpeg.P"'" * ( (lpeg.P'\\' * 1) + (1 - (lpeg.S"'\n\r\f")) )^0 * lpeg.P"'")

local space = (lpeg.S'\n \t\r\f')^0
 
local syntax = [[
  template <- (%state <item>* -> {} !.) -> compiletemplate
  item <- <text> / <templateappl>
  text <- (%state {~ (!<selector> ('$$' -> '$' / .))+ ~}) -> compiletext
  selector <- '$' %alphanum+ ('|' %alphanum+)*
  templateappl <- (%state {<selector>} {~ <args>? ~} {%longstring?} (%s ',' %s {%longstring})* -> {}) 
      -> compileapplication
  args <- '{' %s '}' / '{' %s <arg> %s (',' %s <arg> %s)* ','? %s '}'
  arg <- <attr> / <literal>
  attr <- <symbol> %s '=' %s <literal> / '[' %s <literal> %s ']' %s '=' %s <literal>
  symbol <- %alpha %alphanum*
  literal <- <args> / %string / %longstring / %number / 'true' / 'false' / 
     'nil' / {<selector>} -> parseselector
]]

local syntax_defs = {
  alpha = alpha,
  alphanum = alphanum,
  number = number,
  string = shortstring,
  longstring = longstring,
  s = space,
  parseselector = parse_selector,
  state = lpeg.Carg(1)
}

function cosmo_compiler(compiler_funcs)
   syntax_defs.compiletemplate = compiler_funcs.template
   syntax_defs.compiletext = compiler_funcs.text
   syntax_defs.compileapplication = compiler_funcs.template_application
   return re.compile(syntax, syntax_defs)
end
