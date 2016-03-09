
local lpeg = require "lpeg"
local re = require "re"

if _VERSION ~= "Lua 5.1" then
  _ENV = setmetatable({}, { __index = _G })
else
  module(..., package.seeall)
  _ENV = _M
end

local function parse_selector(selector, env)
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

local function parse_exp(exp)
  return exp
end

local start = "[" * lpeg.P"="^1 * "["

local start_ls = "[" * lpeg.P"="^0 * "["

local longstring1 = lpeg.P{
  "longstring",
  longstring = lpeg.P"[[" * (lpeg.V"longstring" + (lpeg.P(1) - lpeg.P"]]"))^0 * lpeg.P"]]"
}

local longstring2 = lpeg.Cmt(start, function (s, i, start)
  local p = string.gsub(start, "%[", "]")
  local _, e = string.find(s, p, i)
  return (e and e + 1)
end)

local longstring = #("[" * lpeg.S"[=") * (longstring1 + longstring2)

local function parse_longstring(s)
  local start = s:match("^(%[=*%[)")
  if start then
    return string.format("%q", s:sub(#start + 1, #s - #start))
  else
    return s
  end
end

local alpha =  lpeg.R('__','az','AZ','\127\255')

local n = lpeg.R'09'

local alphanum = alpha + n

local name = alpha * (alphanum)^0

local number = (lpeg.P'.' + n)^1 * (lpeg.S'eE' * lpeg.S'+-'^-1)^-1 * (alphanum)^0
number = #(n + (lpeg.P'.' * n)) * number

local shortstring = (lpeg.P'"' * ( (lpeg.P'\\' * 1) + (1 - (lpeg.S'"\n\r\f')) )^0 * lpeg.P'"') +
  (lpeg.P"'" * ( (lpeg.P'\\' * 1) + (1 - (lpeg.S"'\n\r\f")) )^0 * lpeg.P"'")

local space = (lpeg.S'\n \t\r\f')^0

local function syntax(lbra, rbra)
  return [[
      template <- (<item>* -> {} !.) -> compiletemplate
      item <- <text> / <templateappl> / (. => error)
      text <- ({~ (!<selector> ('$$' -> '$' / .))+ ~}) -> compiletext
      selector <- ('$]] .. lbra .. [[' %s {~ <exp> ~} %s ']] .. rbra .. [[') -> parseexp /
         ('$' %alphanum+ ('|' %alphanum+)*) -> parseselector
      templateappl <- ({~ <selector> ~} {~ <args>? ~} !'{'
         ({%longstring} -> compilesubtemplate)? (%s ','? %s ({%longstring} -> compilesubtemplate))* -> {} !(','? %s %start))
         -> compileapplication
      args <- '{' %s '}' / '{' %s <arg> %s (',' %s <arg> %s)* ','? %s '}'
      arg <- <attr> / <exp>
      attr <- <symbol> %s '=' !'=' %s <exp> / '[' !'[' !'=' %s <exp> %s ']' %s '=' %s <exp>
      symbol <- %alpha %alphanum*
      explist <- <exp> (%s ',' %s <exp>)* (%s ',')?
      exp <- <simpleexp> (%s <binop> %s <simpleexp>)*
      simpleexp <- <args> / %string / %longstring -> parsels / %number / 'true' / 'false' /
         'nil' / <unop> %s <exp> / <prefixexp> / (. => error)
      unop <- '-' / 'not' / '#'
      binop <- '+' / '-' / '*' / '/' / '^' / '%' / '..' / '<=' / '<' / '>=' / '>' / '==' / '~=' /
          'and' / 'or'
      prefixexp <- ( <selector> / {%name} -> addenv / '(' %s <exp> %s ')' )
          ( %s <args> / '.' %name / ':' %name %s ('(' %s ')' / '(' %s <explist> %s ')') /
          '[' %s <exp> %s ']' / '(' %s ')' / '(' %s <explist> %s ')' /
          %string / %longstring -> parsels %s )*
  ]]
end

local function pos_to_line(str, pos)
  local s = str:sub(1, pos)
  local line, start = 1, 0
  local newline = string.find(s, "\n")
  while newline do
    line = line + 1
    start = newline
    newline = string.find(s, "\n", newline + 1)
  end
  return line, pos - start
end

local function ast_text(text)
  return { tag = "text", text = text }
end

local function ast_template_application(selector, args, ast_first_subtemplate, ast_subtemplates)
  if not ast_subtemplates then
    ast_first_subtemplate = nil
  end
  local subtemplates = { ast_first_subtemplate, (table.unpack or unpack)(ast_subtemplates or {}) }
  return { tag = "appl", selector = selector, args = args, subtemplates = subtemplates }
end

local function ast_template(parts)
  return { tag = "template", parts = parts }
end

local function ast_subtemplate(text)
  local start = text:match("^(%[=*%[)")
  if start then text = text:sub(#start + 1, #text - #start) end
  return ast:match(text)
end

local syntax_defs = {
  start = start_ls,
  alpha = alpha,
  alphanum = alphanum,
  name = name,
  number = number,
  string = shortstring,
  longstring = longstring,
  s = space,
  parseselector = parse_selector,
  parseexp = parse_exp,
  parsels = parse_longstring,
  addenv = function (s) return "env['" .. s .. "']" end,
  error = function (tmpl, pos)
                local line, pos = pos_to_line(tmpl, pos)
                error("syntax error in template at line " .. line .. " position " .. pos)
              end,
  compiletemplate = ast_template,
  compiletext = ast_text,
  compileapplication = ast_template_application,
  compilesubtemplate = ast_subtemplate
}

function new(lbra, rbra)
  lbra = lbra or "("
  rbra = rbra or ")"
  return re.compile(syntax(lbra, rbra), syntax_defs)
end

default = new()

return _ENV
