-- Generate the rockspecs

require "std"

if select ("#", ...) < 1 then
  io.stderr:write "Usage: mkrockspecs VERSION\n"
  os.exit ()
end

version = select (1, ...)

function format (x, indent)
  indent = indent or ""
  if type (x) == "table" then
    local s = "{\n"
    for i, v in pairs (x) do
      if type (i) ~= "number" then
        s = s..indent..i.." = "..format (v, indent.."  ")..",\n"
      end
    end
    for i, v in ipairs (x) do
      s = s..indent..format (v, indent.."  ")..",\n"
    end
    return s..indent:sub(1, -3).."}"
  elseif type (x) == "string" then
    return string.format ("%q", x)
  else
    return tostring (x)
  end
end

for f, spec in pairs (loadfile ("rockspecs.lua") ()) do
  if f ~= "default" then
    local specfile = "lrexlib-"..f:lower ().."-"..version.."-1.rockspec"
    h = io.open (specfile, "w")
    assert (h)
    flavour = f -- a global, visible in loadfile
    local specs = loadfile ("rockspecs.lua") () -- reload to get current flavour interpolated
    local spec = table.merge (specs.default, specs[f])
    local s = ""
    for i, v in pairs (spec) do
      s = s..i.." = "..format (v, "  ").."\n"
    end
    h:write (s)
    h:close ()
    os.execute ("luarocks lint " .. specfile)
  end
end
