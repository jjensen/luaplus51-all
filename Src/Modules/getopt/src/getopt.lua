--- Simplified getopt, based on Svenne Panne's Haskell GetOpt.<br>
-- Usage:
-- <ul>
-- <li><code>options = {Option {...}, ...}</br>
-- getopt.processArgs ()</code></li>
-- <li>Assumes <code>prog = {name[, banner] [, purpose] [, notes] [, usage]}</code></li>
-- <li>Options take a single dash, but may have a double dash.</li>
-- <li>Arguments may be given as <code>-opt=arg</code> or <code>-opt arg</code>.</li>
-- <li>If an option taking an argument is given multiple times, only the
-- last value is returned; missing arguments are returned as 1.</li>
-- </ul>
-- getOpt, usageInfo and usage can be called directly (see
-- below, and the example at the end). Set _DEBUG.std to a non-nil
-- value to run the example.
-- <ul>
-- <li>TODO: Sort out the packaging. getopt.Option is tedious to type, but
-- surely Option shouldn't be in the root namespace?</li>
-- <li>TODO: Wrap all messages; do all wrapping in processArgs, not
-- usageInfo; use sdoc-like library (see string.format todos).</li>
-- <li>TODO: Don't require name to be repeated in banner.</li>
-- <li>TODO: Store version separately (construct banner?).</li>
-- </ul>

--- Make a shallow copy of a table, including any metatable (for a
-- deep copy, use tree.clone).
-- @param t table
-- @param nometa if non-nil don't copy metatable
-- @return copy of table
local unpack = unpack or table.unpack

local function clone (t, nometa)
  local u = {}
  if not nometa then
    setmetatable (u, getmetatable (t))
  end
  for i, v in pairs (t) do
    u[i] = v
  end
  return u
end

--- Clone a table, renaming some keys.
-- @param map table <code>{old_key=new_key, ...}</code>
-- @param t table to copy
-- @return copy of table
local function clone_rename (map, t)
  local r = clone (t)
  for i, v in pairs (map) do
    r[v] = t[i]
    r[i] = nil
  end
  return r
end

--- Compose functions.
-- @param f1...fn functions to compose
-- @return composition of f1 ... fn
local function compose (...)
  local arg = {...}
  local fns, n = arg, #arg
  return function (...)
           local arg = {...}
           for i = n, 1, -1 do
             arg = {fns[i] (unpack (arg))}
           end
           return unpack (arg)
         end
end

local function _leaves (it, tr)
  local function visit (n)
    if type (n) == "table" then
      for _, v in it (n) do
        visit (v)
      end
    else
      coroutine.yield (n)
    end
  end
  return coroutine.wrap (visit), tr
end

--- Tree iterator which returns just numbered leaves, in order.
-- @param tr tree to iterate over
-- @return iterator function
-- @return the tree, as above
function _G.ileaves (tr)
  return _leaves (ipairs, tr)
end

--- Merge one table into another. <code>u</code> is merged into <code>t</code>.
-- @param t first table
-- @param u second table
-- @return first table
local function merge (t, u)
  for i, v in pairs (u) do
    t[i] = v
  end
  return t
end

--- Map a function over an iterator.
-- @param f function
-- @param i iterator
-- @return result table
local function map (f, i, ...)
  local t = {}
  for e in i (...) do
    local r = f (e)
    if r then
      table.insert (t, r)
    end
  end
  return t
end

--- Write values adding a newline after each.
-- @param h file handle (default: <code>io.output ()</code>
-- @param ... values to write (as for write)
local function writelines (h, ...)
  if io.type (h) ~= "file" then
    io.write (h, "\n")
    h = io.output ()
  end
  for v in ileaves ({...}) do
    h:write (v, "\n")
  end
end

--- An iterator over the elements of a list.
-- @param l list to iterate over
-- @return iterator function which returns successive elements of the list
-- @return the list <code>l</code> as above
-- @return <code>true</code>
local function list_elems (l)
  local n = 0
  return function (l)
           n = n + 1
           if n <= #l then
             return l[n]
           end
         end,
  l, true
end

--- Concatenate lists.
-- @param ... lists
-- @return <code>{l<sub>1</sub>[1], ...,
-- l<sub>1</sub>[#l<sub>1</sub>], ..., l<sub>n</sub>[1], ...,
-- l<sub>n</sub>[#l<sub>n</sub>]}</code>
local function list_concat (...)
  local r = {}
  for l in list_elems ({...}) do
    for v in list_elems (l) do
      table.insert (r, v)
    end
  end
  return r
end

--- Map a function over a list.
-- @param f function
-- @param l list
-- @return result list <code>{f (l[1]), ..., f (l[#l])}</code>
local function list_map (f, l)
  return map (f, list_elems, l)
end

--- Map a function over a list of lists.
-- @param f function
-- @param ls list of lists
-- @return result list <code>{f (unpack (ls[1]))), ..., f (unpack (ls[#ls]))}</code>
local function list_mapWith (f, l)
  return map (compose (f, unpack), list_elems, l)
end

--- Reverse a list.
-- @param l list
-- @return list <code>{l[#l], ..., l[1]}</code>
local function list_reverse (l)
  local m = {}
  for i = #l, 1, -1 do
    table.insert (m, l[i])
  end
  return m
end

--- Transpose a list of lists.
-- This function in Lua is equivalent to zip and unzip in more
-- strongly typed languages.
-- @param ls <code>{{l<sub>1,1</sub>, ..., l<sub>1,c</sub>}, ...,
-- {l<sub>r,1<sub>, ..., l<sub>r,c</sub>}}</code>
-- @return <code>{{l<sub>1,1</sub>, ..., l<sub>r,1</sub>}, ...,
-- {l<sub>1,c</sub>, ..., l<sub>r,c</sub>}}</code>
local function list_transpose (ls)
  local ms, len = {}, #ls
  for i = 1, math.max (unpack (list_map (function (l) return #l end, ls))) do
    ms[i] = {}
    for j = 1, len do
      ms[i][j] = ls[j][i]
    end
  end
  return ms
end

--- Wrap a string into a paragraph.
-- @param s string to wrap
-- @param w width to wrap to (default: 78)
-- @param ind indent (default: 0)
-- @param ind1 indent of first line (default: ind)
-- @return wrapped paragraph
local function string_wrap (s, w, ind, ind1)
  w = w or 78
  ind = ind or 0
  ind1 = ind1 or ind
  assert (ind1 < w and ind < w,
          "the indents must be less than the line width")
  s = string.rep (" ", ind1) .. s
  local lstart, len = 1, string.len (s)
  while len - lstart > w - ind do
    local i = lstart + w - ind
    while i > lstart and string.sub (s, i, i) ~= " " do
      i = i - 1
    end
    local j = i
    while j > lstart and string.sub (s, j, j) == " " do
      j = j - 1
    end
    s = string.sub (s, 1, j) .. "\n" .. string.rep (" ", ind) ..
      string.sub (s, i + 1, -1)
    local change = ind + 1 - (i - j)
    lstart = j + change
    len = len + change
  end
  return s
end

--- Root object
-- @class table
-- @name Object
-- @field _init constructor method or list of fields to be initialised by the
-- constructor
-- @field _clone object constructor which provides the behaviour for <code>_init</code>
-- documented above
local Object = {
  _init = {},

  _clone = function (self, ...)
    local object = clone (self)
    if type (self._init) == "table" then
      merge (object, clone_rename (self._init, ...))
    else
      object = self._init (object, ...)
    end
    return setmetatable (object, object)
  end,

  -- Sugar instance creation
  __call = function (...)
    -- First (...) gets first element of list
    return (...)._clone (...)
  end,
}
setmetatable (Object, Object)

---------------------------------------------------------------------------------------
local M = {}

--- Perform argument processing
-- @param argIn list of command-line args
-- @param options options table
-- @return table of remaining non-options
-- @return table of option key-value list pairs
-- @return table of error messages
function M.getOpt (argIn, options)
  local noProcess = nil
  local argOut, optOut, errors = {[0] = argIn[0]}, {}, {}
  -- get an argument for option opt
  local function getArg (o, opt, arg)
    if o.type == nil then
      if arg ~= nil then
        table.insert (errors, "option `" .. opt .. "' doesn't take an argument")
      end
    else
      if arg == nil and argIn[1] and
        string.sub (argIn[1], 1, 1) ~= "-" then
        arg = argIn[1]
        table.remove (argIn, 1)
      end
      if arg == nil and o.type == "Req" then
        table.insert (errors,  "option `" .. opt ..
                      "' requires an argument `" .. o.var .. "'")
        return nil
      end
    end
    return arg or true -- make sure arg has a value
  end

  local function parseOpt (opt, arg)
    local o = options.name[opt]
    if o ~= nil then
      if o.var then
        optOut[o.name[1]] = optOut[o.name[1]] or {}
        table.insert (optOut[o.name[1]], getArg (o, opt, arg))
      else
        optOut[o.name[1]] = getArg (o, opt, arg)
      end
    else
      table.insert (errors, "unrecognized option `-" .. opt .. "'")
    end
  end
  while argIn[1] do
    local v = argIn[1]
    table.remove (argIn, 1)
    local _, _, dash, opt = string.find (v, "^(%-%-?)([^=-][^=]*)")
    local _, _, arg = string.find (v, "=(.*)$")
    if v == "--" then
      noProcess = 1
    elseif dash == nil or noProcess then -- non-option
      table.insert (argOut, v)
    else -- option
      parseOpt (opt, arg)
    end
  end
  return argOut, optOut, errors
end


--- Options table type.
-- @class table
-- @name _G.Option
-- @field name list of names
-- @field desc description of this option
-- @field type type of argument (if any): <code>Req</code>(uired),
-- <code>Opt</code>(ional)
-- @field var descriptive name for the argument
M.Option = Object {_init = {"name", "desc", "type", "var"}}

--- Options table constructor: adds lookup tables for the option names
function M.makeOptions (t)
  --[[t = list_concat (t or {},
                   {M.Option {{"version", "V"},
                            "output version information and exit"},
                    M.Option {{"help", "h"},
                            "display this help and exit"}}
               )
--]]
  local name = {}
  for v in list_elems (t) do
    for j, s in pairs (v.name) do
      if name[s] then
        warn ("duplicate option '%s'", s)
      end
      name[s] = v
    end
  end
  t.name = name
  return t
end


--- Produce usage info for the given options
-- @param header header string
-- @param optDesc option descriptors
-- @param pageWidth width to format to [78]
-- @return formatted string
function M.usageInfo (header, optDesc, pageWidth)
  pageWidth = pageWidth or 78
  -- Format the usage info for a single option
  -- @param opt the Option table
  -- @return options
  -- @return description
  local function fmtOpt (opt)
    local function fmtName (o)
      return "-" .. o
    end
    local function fmtArg ()
      if opt.type == nil then
        return ""
      elseif opt.type == "Req" then
        return "=" .. opt.var
      else
        return "[=" .. opt.var .. "]"
      end
    end
    local textName = list_reverse (list_map (fmtName, opt.name))
    textName[#textName] = textName[#textName] .. fmtArg ()
    return {table.concat ({table.concat (textName, ", ")}, ", "),
      opt.desc}
  end
  local function sameLen (xs)
    local n = math.max (unpack (list_map (string.len, xs)))
    for i, v in pairs (xs) do
      xs[i] = string.sub (v .. string.rep (" ", n), 1, n)
    end
    return xs, n
  end
  local function paste (x, y)
    return "  " .. x .. "  " .. y
  end
  local function wrapper (w, i)
    return function (s)
             return string_wrap (s, w, i, 0)
           end
  end
  local optText = ""
  if #optDesc > 0 then
    local cols = list_transpose (list_map (fmtOpt, optDesc))
    local width
    cols[1], width = sameLen (cols[1])
    cols[2] = list_map (wrapper (pageWidth, width + 4), cols[2])
    optText = "\n\n" ..
      table.concat (list_mapWith (paste,
                                  list_transpose ({sameLen (cols[1]),
                                                    cols[2]})),
                    "\n")
  end
  return header .. optText
end

--- Emit a usage message.
function M.usage ()
  local usage, purpose, notes = "[OPTION]... [FILE]...", "", ""
  if prog.usage then
    usage = prog.usage
  end
  if prog.purpose then
    purpose = "\n" .. prog.purpose
  end
  if prog.notes then
    notes = "\n\n"
    if not string.find (prog.notes, "\n") then
      notes = notes .. string_wrap (prog.notes)
    else
      notes = notes .. prog.notes
    end
  end
  writelines (M.usageInfo ("Usage: " .. prog.name .. " " .. usage .. purpose,
                                   options)
                 .. notes)
end


--- Simple getOpt wrapper.
-- Adds <code>-version</code>/<code>-V</code> and
-- <code>-help</code>/<code>-h</code> automatically;
-- stops program if there was an error, or if <code>-help</code> or
-- <code>-version</code> was used.
function M.processArgs ()
  local totArgs = #arg
  options = M.makeOptions (options)
  local errors
  _G.arg, opt, errors = M.getOpt (arg, options)
  if (opt.version or opt.help) and prog.banner then
    writelines (prog.banner)
  end
  if #errors > 0 or opt.help then
    local name = prog.name
    prog.name = nil
    if #errors > 0 then
      warn (table.concat (errors, "\n") .. "\n")
    end
    prog.name = name
    M.usage ()
    if #errors > 0 then
      error ()
    end
  end
  if opt.version or opt.help then
    os.exit ()
  end
end
_G.options = nil


-- A small and hopefully enlightening example:
if type (_DEBUG) == "table" and _DEBUG.std then

  options = makeOptions ({
                           Option {{"verbose", "v"}, "verbosely list files"},
                           Option {{"output", "o"}, "dump to FILE", "Opt", "FILE"},
                           Option {{"name", "n"}, "only dump USER's files", "Req", "USER"},
                       })

  function test (cmdLine)
    local nonOpts, opts, errors = getopt.getOpt (cmdLine, options)
    if #errors == 0 then
      print ("options=" .. tostring (opts) ..
             "  args=" .. tostring (nonOpts) .. "\n")
    else
      print (table.concat (errors, "\n") .. "\n" ..
             getopt.usageInfo ("Usage: foobar [OPTION...] FILE...",
                               options))
    end
  end

  -- FIXME: Turn the following documentation into unit tests
  prog = {name = "foobar"} -- for errors
  -- Example runs:
  test {"foo", "-v"}
  -- options={verbose={1}}  args={1=foo}
  test {"foo", "--", "-v"}
  -- options={}  args={1=foo,2=-v}
  test {"-o", "-V", "-name", "bar", "--name=baz"}
  -- options={name={"baz"},version={1},output={1}}  args={}
  test {"-foo"}
  -- unrecognized option `-foo'
  -- Usage: foobar [OPTION]... [FILE]...
  --
  --   -v, -verbose                verbosely list files
  --   -o, -output[=FILE]          dump to FILE
  --   -n, -name=USER              only dump USER's files
  --   -V, -version                output version information and exit
  --   -h, -help                   display this help and exit

end

return M

