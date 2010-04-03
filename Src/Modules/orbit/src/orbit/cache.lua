
require "lfs"

module("orbit.cache", package.seeall)

local function pathinfo_to_file(path_info)
   local atom = path_info:find("/xml$")
   if atom then
      path_info = path_info:sub(2, atom - 1)
   else
      path_info = path_info:sub(2, #path_info)
   end
   path_info = string.gsub(path_info, "/", "-")
   if path_info == "" then path_info = "index" end
   if atom then
      return path_info .. '.xml'
   else
      return path_info .. '.html'
   end
end

function get(cache, key)
  if not cache.base_path then
     local headers = {}
     if key:find("/xml$") then 
	headers["Content-Type"] = "text/xml"
     else
	headers["Content-Type"] = "text/html"
     end
     return cache.values[key], headers
  else
     local filename = cache.base_path .. "/" .. pathinfo_to_file(key)
     local web = { headers = {} }
     if lfs.attributes(filename, "mode") == "file" then
	local response = cache.app:serve_static(web, filename)
	return response, web.headers
     end
  end
end

local function writefile(filename, contents)
  local file = assert(io.open(filename, "wb"))
  if lfs.lock(file, "w") then
     file:write(contents)
     lfs.unlock(file)
     file:close()
  else
     file:close()
  end
end

function set(cache, key, value)
  if not cache.base_path then
     cache.values[key] = value
  else
     local filename = cache.base_path .. "/" .. pathinfo_to_file(key)
     writefile(filename, value)
  end
end

local function cached(cache, f)
  return function (web, ...)
	    local body, headers = cache:get(web.path_info)
	    if body then
	      for k, v in pairs(headers) do
		web.headers[k] = v
	      end
	      return body
	    else
	      local key = web.path_info
	      local body = f(web, ...)
	      cache:set(key, body)
	      return body
	    end
	 end
end

function invalidate(cache, ...)
   for _, key in ipairs{...} do
      if not cache.base_path then
	 cache.values[key] = nil
      else
	 local filename = cache.base_path .. "/" .. pathinfo_to_file(key)
	 os.remove(filename)
      end
   end
end

function nuke(cache)
   if not cache.base_path then 
      cache.values = {}
   else
      for file in lfs.dir(cache.base_path) do
	 if file ~= "." and file ~= ".." then 
	    os.remove(cache.base_path .. "/" .. file) 
	 end
      end
   end
end

function new(app, base_path)
   local values
   if not base_path then
      values = {}
   else
      local dir = lfs.attributes(base_path, "mode")
      if not dir then
	 assert(lfs.mkdir(base_path))
      elseif dir ~= "directory" then
	 error("base path of cache " .. base_path .. " not a directory")
      end
   end
   local cache = { app = app, values = values, 
		   base_path = base_path }
   setmetatable(cache, { __index = _M, __call = function (tab, f)
						   return cached(tab, f)
						end })
   return cache
end
