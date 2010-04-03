module("wsapi.util", package.seeall)

----------------------------------------------------------------------------
-- Decode an URL-encoded string (see RFC 2396)
----------------------------------------------------------------------------
function url_decode(str)
  if not str then return nil end
  str = string.gsub (str, "+", " ")
  str = string.gsub (str, "%%(%x%x)", function(h) return string.char(tonumber(h,16)) end)
  str = string.gsub (str, "\r\n", "\n")
  return str
end

----------------------------------------------------------------------------
-- URL-encode a string (see RFC 2396)
----------------------------------------------------------------------------
function url_encode(str)
  if not str then return nil end
  str = string.gsub (str, "\n", "\r\n")
  str = string.gsub (str, "([^%w ])",
	function (c) return string.format ("%%%02X", string.byte(c)) end)
  str = string.gsub (str, " ", "+")
  return str
end

function sanitize(text)
   return text:gsub(">", "&gt;"):gsub("<", "&lt;")
end

function is_empty(s)
  if s and s ~= "" then return s else return nil end
end

function make_rewindable(wsapi_env)
   local new_env = { input = { position = 1, contents = "" } }
   function new_env.input:read(size)
      local left = #self.contents - self.position + 1
      local s
      if left < size then
	 self.contents = self.contents .. wsapi_env.input:read(size - left)
	 s = self.contents:sub(self.position)
	 self.position = #self.contents + 1
      else
	 s = self.contents:sub(self.position, self.position + size)
	 self.position = self.position + size
      end
      if s == "" then return nil else return s end
   end
   function new_env.input:rewind()
      self.position = 1
   end
   return setmetatable(new_env, { __index = wsapi_env, __newindex = wsapi_env })
end

-- getopt, POSIX style command line argument parser
-- param arg contains the command line arguments in a standard table.
-- param options is a string with the letters that expect string values.
-- returns a table where associated keys are true, nil, or a string value.
-- The following example styles are supported
--   -a one  ==> opts["a"]=="one"
--   -bone   ==> opts["b"]=="one"
--   -c      ==> opts["c"]==true
--   --c=one ==> opts["c"]=="one"
--   -cdaone ==> opts["c"]==true opts["d"]==true opts["a"]=="one"
-- note POSIX demands the parser ends at the first non option
--      this behavior isn't implemented.

function getopt( arg, options )
  local tab, args = {}, {}
  local k = 1
  while k <= #arg do
    local v = arg[k]
    if string.sub( v, 1, 2) == "--" then
      local x = string.find( v, "=", 1, true )
      if x then tab[ string.sub( v, 3, x-1 ) ] = string.sub( v, x+1 )
      else      tab[ string.sub( v, 3 ) ] = true
      end
      k = k + 1
    elseif string.sub( v, 1, 1 ) == "-" then
      local y = 2
      local l = string.len(v)
      local jopt
      local next = 1
      while ( y <= l ) do
        jopt = string.sub( v, y, y )
        if string.find( options, jopt, 1, true ) then
          if y < l then
            tab[ jopt ] = string.sub( v, y+1 )
            y = l
          else
            tab[ jopt ] = arg[ k + 1 ]
	    next = 2
          end
        else
          tab[ jopt ] = true
        end
        y = y + 1
      end
      k = k + next
    else
      args[#args + 1] = v
      k = k + 1
    end
  end
  return tab, args
end
