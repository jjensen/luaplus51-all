-- luac.lua - partial reimplementation of luac in Lua.
-- http://lua-users.org/wiki/LuaCompilerInLua
-- David Manura et al.
-- Licensed under the same terms as Lua (MIT license).

local outfile = 'luac.out'

-- Parse options.
local chunks = {}
local allowoptions = true
local iserror = false
local parseonly = false
while arg[1] do
  if     allowoptions and arg[1] == '-' then
    chunks[#chunks + 1] = arg[1]
    allowoptions = false
  elseif allowoptions and arg[1] == '-l' then
    io.stderr:write('-l option not implemented\n')
    iserror = true
  elseif allowoptions and arg[1] == '-o' then
    outfile = assert(arg[2], '-o needs argument')
    table.remove(arg, 1)
  elseif allowoptions and arg[1] == '-p' then
    parseonly = true
  elseif allowoptions and arg[1] == '-s' then
    io.stderr:write("-s option ignored\n")
  elseif allowoptions and arg[1] == '-v' then
    io.stdout:write(_VERSION .. " Copyright (C) 1994-2008 Lua.org, PUC-Rio\n")
  elseif allowoptions and arg[1] == '--' then
    allowoptions = false
  elseif allowoptions and arg[1]:sub(1,1) == '-' then
    io.stderr:write("luac: unrecognized option '" .. arg[1] .. "'\n")
    iserror = true
    break
  else
    chunks[#chunks + 1] = arg[1]
  end
  table.remove(arg, 1)
end
if #chunks == 0 then
  io.stderr:write("luac: no input files given\n")
  iserror = true
end

if iserror then
  io.stdout:write[[
usage: luac [options] [filenames].
Available options are:
  -        process stdin
  -l       list
  -o name  output to file 'name' (default is "luac.out")
  -p       parse only
  -s       strip debug information
  -v       show version information
  --       stop handling options
]]
  os.exit(1)
end

-- Load/compile chunks.
for i,filename in ipairs(chunks) do
  chunks[i] = assert(loadfile(filename ~= '-' and filename or nil))
end

if parseonly then
  os.exit(0)
end

-- Combine chunks.
if #chunks == 1 then
  chunks = chunks[1]
else
  -- Note: the reliance on loadstring is possibly not ideal,
  -- though likely unavoidable.
  local ts = { "local loadstring=loadstring;"  }
  for i,f in ipairs(chunks) do
    ts[i] = ("loadstring%q(...);"):format(string.dump(f))
  end
  --possible extension: ts[#ts] = 'return ' .. ts[#ts]
  chunks = assert(loadstring(table.concat(ts)))
end

-- Output.
local out = outfile == '-' and io.stdout or assert(io.open(outfile, "wb"))
out:write(string.dump(chunks))
if out ~= io.stdout then out:close() end
