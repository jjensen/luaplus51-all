
require "lpeg"
require "re"
require "wsapi.util"

module("orbit.routes", package.seeall)

local function foldr(t, f, acc)
   for i = #t, 1, -1 do
      acc = f(t[i], acc)
   end
   return acc
end

param = re.compile[[ [/%.] ':' {[%w_]+} &('/' / {'.'} / !.) ]] / 
             function (name, dot)
		local extra = { inner = (lpeg.P(1) - lpeg.S("/" .. (dot or "")))^1,
				close = lpeg.P"/" + lpeg.P(dot or -1) + lpeg.P(-1) }
		return { cap = lpeg.Carg(1) * re.compile([[ [/%.] {%inner+} &(%close) ]], extra) / 
		                   function (params, item, delim)
				      params[name] = wsapi.util.url_decode(item)
				   end,
				clean = re.compile([[ [/%.] %inner &(%close) ]], extra) }
	     end

opt_param = re.compile[[ [/%.] '?:' {[%w_]+} '?' &('/' / {'.'} / !.) ]] / 
             function (name, dot)
		local extra = { inner = (lpeg.P(1) - lpeg.S("/" .. (dot or "")))^1,
				close = lpeg.P"/" + lpeg.P(dot or -1) + lpeg.P(-1) }
		return { cap = (lpeg.Carg(1) * re.compile([[ [/%.] {%inner+} &(%close) ]], extra) / 
		                   function (params, item, delim)
				      params[name] = wsapi.util.url_decode(item)
				   end)^-1,
				clean = re.compile([[ [/%.] %inner &(%close) ]], extra)^-1 }
	     end

splat = re.compile[[ {[/%.]} {'*'} &('/' / '.' / !.) ]]

rest = lpeg.C((lpeg.P(1) - param - opt_param - splat)^1)

fold_caps = function (cap, acc)
	       if cap == "*" then
		  return { cap = (lpeg.Carg(1) * lpeg.C((lpeg.P(1) - acc.clean)^1) / 
                                      function (params, splat)
					 if not params.splat then params.splat = {} end
					 params.splat[#params.splat+1] = wsapi.util.url_decode(splat)
				      end) * acc.cap,
			   clean = (lpeg.P(1) - acc.clean)^1 * acc.clean }
	       elseif type(cap) == "string" then
		  return { cap = lpeg.P(cap) * acc.cap, clean = lpeg.P(cap) * acc.clean }
	       else
		  return { cap = cap.cap * acc.cap, clean = cap.clean * acc.clean }
	       end
	    end

route = lpeg.Ct((param + opt_param + splat + rest)^1 * lpeg.P(-1)) / 
           function (caps)
	      return foldr(caps, fold_caps, { cap = lpeg.P("/")^-1 * lpeg.P(-1), clean = lpeg.P("/")^-1 * lpeg.P(-1) })
	   end

function R(path)
   local p = route:match(path)
   return setmetatable({ patt = p.cap }, { __index = { match = function (t, s)
							      local params = {}
							      if t.patt:match(s, 1, params) then
								 return params
							      else
								 return nil
							      end
							   end } })
end

