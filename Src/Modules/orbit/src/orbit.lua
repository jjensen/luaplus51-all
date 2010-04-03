
require "wsapi.request"
require "wsapi.response"
require "wsapi.util"

local _G, setfenv = _G, setfenv
module("orbit")
local _M = _M
setfenv(1, _G)

_M._NAME = "orbit"
_M._VERSION = "2.1.0"
_M._COPYRIGHT = "Copyright (C) 2007-2009 Kepler Project"
_M._DESCRIPTION = "MVC Web Development for the Kepler platform"

_M.mime_types = {
  ez = "application/andrew-inset",
  atom = "application/atom+xml",
  hqx = "application/mac-binhex40",
  cpt = "application/mac-compactpro",
  mathml = "application/mathml+xml",
  doc = "application/msword",
  bin = "application/octet-stream",
  dms = "application/octet-stream",
  lha = "application/octet-stream",
  lzh = "application/octet-stream",
  exe = "application/octet-stream",
  class = "application/octet-stream",
  so = "application/octet-stream",
  dll = "application/octet-stream",
  dmg = "application/octet-stream",
  oda = "application/oda",
  ogg = "application/ogg",
  pdf = "application/pdf",
  ai = "application/postscript",
  eps = "application/postscript",
  ps = "application/postscript",
  rdf = "application/rdf+xml",
  smi = "application/smil",
  smil = "application/smil",
  gram = "application/srgs",
  grxml = "application/srgs+xml",
  mif = "application/vnd.mif",
  xul = "application/vnd.mozilla.xul+xml",
  xls = "application/vnd.ms-excel",
  ppt = "application/vnd.ms-powerpoint",
  rm = "application/vnd.rn-realmedia",
  wbxml = "application/vnd.wap.wbxml",
  wmlc = "application/vnd.wap.wmlc",
  wmlsc = "application/vnd.wap.wmlscriptc",
  vxml = "application/voicexml+xml",
  bcpio = "application/x-bcpio",
  vcd = "application/x-cdlink",
  pgn = "application/x-chess-pgn",
  cpio = "application/x-cpio",
  csh = "application/x-csh",
  dcr = "application/x-director",
  dir = "application/x-director",
  dxr = "application/x-director",
  dvi = "application/x-dvi",
  spl = "application/x-futuresplash",
  gtar = "application/x-gtar",
  hdf = "application/x-hdf",
  xhtml = "application/xhtml+xml",
  xht = "application/xhtml+xml",
  js = "application/x-javascript",
  skp = "application/x-koan",
  skd = "application/x-koan",
  skt = "application/x-koan",
  skm = "application/x-koan",
  latex = "application/x-latex",
  xml = "application/xml",
  xsl = "application/xml",
  dtd = "application/xml-dtd",
  nc = "application/x-netcdf",
  cdf = "application/x-netcdf",
  sh = "application/x-sh",
  shar = "application/x-shar",
  swf = "application/x-shockwave-flash",
  xslt = "application/xslt+xml",
  sit = "application/x-stuffit",
  sv4cpio = "application/x-sv4cpio",
  sv4crc = "application/x-sv4crc",
  tar = "application/x-tar",
  tcl = "application/x-tcl",
  tex = "application/x-tex",
  texinfo = "application/x-texinfo",
  texi = "application/x-texinfo",
  t = "application/x-troff",
  tr = "application/x-troff",
  roff = "application/x-troff",
  man = "application/x-troff-man",
  me = "application/x-troff-me",
  ms = "application/x-troff-ms",
  ustar = "application/x-ustar",
  src = "application/x-wais-source",
  zip = "application/zip",
  au = "audio/basic",
  snd = "audio/basic",
  mid = "audio/midi",
  midi = "audio/midi",
  kar = "audio/midi",
  mpga = "audio/mpeg",
  mp2 = "audio/mpeg",
  mp3 = "audio/mpeg",
  aif = "audio/x-aiff",
  aiff = "audio/x-aiff",
  aifc = "audio/x-aiff",
  m3u = "audio/x-mpegurl",
  ram = "audio/x-pn-realaudio",
  ra = "audio/x-pn-realaudio",
  wav = "audio/x-wav",
  pdb = "chemical/x-pdb",
  xyz = "chemical/x-xyz",
  bmp = "image/bmp",
  cgm = "image/cgm",
  gif = "image/gif",
  ief = "image/ief",
  jpeg = "image/jpeg",
  jpg = "image/jpeg",
  jpe = "image/jpeg",
  png = "image/png",
  svg = "image/svg+xml",
  svgz = "image/svg+xml",
  tiff = "image/tiff",
  tif = "image/tiff",
  djvu = "image/vnd.djvu",
  djv = "image/vnd.djvu",
  wbmp = "image/vnd.wap.wbmp",
  ras = "image/x-cmu-raster",
  ico = "image/x-icon",
  pnm = "image/x-portable-anymap",
  pbm = "image/x-portable-bitmap",
  pgm = "image/x-portable-graymap",
  ppm = "image/x-portable-pixmap",
  rgb = "image/x-rgb",
  xbm = "image/x-xbitmap",
  xpm = "image/x-xpixmap",
  xwd = "image/x-xwindowdump",
  igs = "model/iges",
  iges = "model/iges",
  msh = "model/mesh",
  mesh = "model/mesh",
  silo = "model/mesh",
  wrl = "model/vrml",
  vrml = "model/vrml",
  ics = "text/calendar",
  ifb = "text/calendar",
  css = "text/css",
  html = "text/html",
  htm = "text/html",
  asc = "text/plain",
  txt = "text/plain",
  rtx = "text/richtext",
  rtf = "text/rtf",
  sgml = "text/sgml",
  sgm = "text/sgml",
  tsv = "text/tab-separated-values",
  wml = "text/vnd.wap.wml",
  wmls = "text/vnd.wap.wmlscript",
  etx = "text/x-setext",
  mpeg = "video/mpeg",
  mpg = "video/mpeg",
  mpe = "video/mpeg",
  qt = "video/quicktime",
  mov = "video/quicktime",
  mxu = "video/vnd.mpegurl",
  avi = "video/x-msvideo",
  movie = "video/x-sgi-movie",
  ice = "x-conference/x-cooltalk",
  rss = "application/rss+xml",
  atom = "application/atom+xml"
}

_M.app_module_methods = {}
local app_module_methods = _M.app_module_methods

_M.web_methods = {}
local web_methods = _M.web_methods

local function flatten(t)
   local res = {}
   for _, item in ipairs(t) do
      if type(item) == "table" then
	 res[#res + 1] = flatten(item)
      else
	 res[#res + 1] = item
      end
   end
   return table.concat(res)
end

local function make_tag(name, data, class)
  if class then class = ' class="' .. class .. '"' else class = "" end
  if not data then
    return "<" .. name .. class .. "/>"
  elseif type(data) == "string" then
    return "<" .. name .. class .. ">" .. data ..
      "</" .. name .. ">"
  else
    local attrs = {}
    for k, v in pairs(data) do
      if type(k) == "string" then
        table.insert(attrs, k .. '="' .. tostring(v) .. '"')
      end
    end
    local open_tag = "<" .. name .. class .. " " ..
      table.concat(attrs, " ") .. ">"
    local close_tag = "</" .. name .. ">"
    return open_tag .. flatten(data) .. close_tag       
  end      
end

function _M.new(app_module)
   if type(app_module) == "string" then
      app_module = { _NAME = app_module }
   else
      app_module = app_module or {}
   end
   for k, v in pairs(app_module_methods) do
      app_module[k] = v
   end
   app_module.run = function (wsapi_env) 
		       return _M.run(app_module, wsapi_env)
		    end
   app_module.real_path = wsapi.app_path or "."
   app_module.mapper = { default = true }
   app_module.not_found = function (web)
			     web.status = "404 Not Found"
			     return [[<html>
				   <head><title>Not Found</title></head>
				      <body><p>Not found!</p></body></html>]]
			  end  
   app_module.server_error = function (web, msg)
				web.status = "500 Server Error"
				return [[<html>
				      <head><title>Server Error</title></head>
					 <body><pre>]] .. msg .. [[</pre></body></html>]]
			     end
   app_module.dispatch_table = { get = {}, post = {}, put = {}, delete = {} }
   return app_module
end

local function serve_file(app_module)
   return function (web)
	     local filename = web.real_path .. web.path_info
	     return app_module:serve_static(web, filename)
	  end
end

function app_module_methods.dispatch_get(app_module, func, ...)
   for _, pat in ipairs{ ... } do
      table.insert(app_module.dispatch_table.get, { pattern = pat, 
		      handler = func })
   end
end

function app_module_methods.dispatch_post(app_module, func, ...)
   for _, pat in ipairs{ ... } do
      table.insert(app_module.dispatch_table.post, { pattern = pat, 
		      handler = func })
   end
end

function app_module_methods.dispatch_put(app_module, func, ...)
   for _, pat in ipairs{ ... } do
      table.insert(app_module.dispatch_table.put, { pattern = pat, 
		      handler = func })
   end
end

function app_module_methods.dispatch_delete(app_module, func, ...)
   for _, pat in ipairs{ ... } do
      table.insert(app_module.dispatch_table.delete, { pattern = pat, 
		      handler = func })
   end
end

function app_module_methods.dispatch_wsapi(app_module, func, ...)
  for _, pat in ipairs{ ... } do
    for _, tab in pairs(app_module.dispatch_table) do
      table.insert(tab, { pattern = pat, handler = func, wsapi = true })
    end
  end
end

function app_module_methods.dispatch_static(app_module, ...)
   app_module:dispatch_get(serve_file(app_module), ...)
end

function app_module_methods.serve_static(app_module, web, filename)
   local ext = string.match(filename, "%.([^%.]+)$")
   if app_module.use_xsendfile then
      web.headers["Content-Type"] = _M.mime_types[ext] or 
	 "application/octet-stream"
      web.headers["X-Sendfile"] = filename
      return "xsendfile"
   else
      local file = io.open(filename, "rb")
      if not file then
	 return app_module.not_found(web)
      else
	 web.headers["Content-Type"] = _M.mime_types[ext] or 
	    "application/octet-stream"
	 local contents = file:read("*a")
	 file:close()
	 return contents
      end
   end

end

local function newtag(name)
  local tag = {}
  setmetatable(tag, {
                 __call = function (_, data)
                            return make_tag(name, data)
                          end,
                 __index = function(_, class)
                             return function (data)
                                      return make_tag(name, data, class)
                                    end
                           end
               })
  return tag
end

local function htmlify_func(func)
  local tags = {}
  local env = { H = function (name)
		      local tag = tags[name]
		      if not tag then
			tag = newtag(name)
			tags[name] = tag
		      end
		      return tag
		    end
	      }
  local old_env = getfenv(func)
  setmetatable(env, { __index = function (env, name)
				  if old_env[name] then
				    return old_env[name]
				  else
				    local tag = newtag(name)
				    rawset(env, name, tag)
				    return tag
				  end
				end })
  setfenv(func, env)
end

function _M.htmlify(app_module, ...)
   if type(app_module) == "function" then
      htmlify_func(app_module)
      for _, func in ipairs{...} do
	htmlify_func(func)
      end
   else
      local patterns = { ... }
      for _, patt in ipairs(patterns) do
	if type(patt) == "function" then
	  htmlify_func(patt)
	else
	  for name, func in pairs(app_module) do
	    if string.match(name, "^" .. patt .. "$") and 
	         type(func) == "function" then
	       htmlify_func(func)
	    end
	  end
	end
      end
   end
end

app_module_methods.htmlify = _M.htmlify

function app_module_methods.model(app_module, ...)
   if app_module.mapper.default then
      local table_prefix = (app_module._NAME and app_module._NAME .. "_") or ""
      if not orbit.model then
	 require "orbit.model"
      end
      app_module.mapper = orbit.model.new(table_prefix, app_module.mapper.conn)
   end
   return app_module.mapper:new(...)
end

function web_methods:redirect(url)
  self.status = "302 Found"
  self.headers["Location"] = url
  return "redirect"
end

function web_methods:link(url, params)
  local link = {}
  local prefix = self.prefix or ""
  local suffix = self.suffix or ""
  for k, v in pairs(params or {}) do
    link[#link + 1] = k .. "=" .. wsapi.util.url_encode(v)
  end
  local qs = table.concat(link, "&")
  if qs and qs ~= "" then
    return prefix .. url .. suffix .. "?" .. qs
  else
    return prefix .. url .. suffix
  end
end

function web_methods:static_link(url)
  local prefix = self.prefix or ""
  local is_script = prefix:match("(%.%w+)$")
  if not is_script then return self:link(url) end
  local vpath = self.path_translated:sub(#self.doc_root+1):match("(.*)/")
  return vpath .. url
end

function web_methods:empty(s)
  return not s or string.match(s, "^%s*$")
end

function web_methods:content_type(s)
  self.headers["Content-Type"] = s
end

function web_methods:page(name, env)
  if not orbit.pages then
    require "orbit.pages"
  end
  local filename
  if name:sub(1, 1) == "/" then
    filename = self.doc_root .. name
  else
    filename = self.real_path .. "/" .. name
  end
  local template = orbit.pages.load(filename)
  if template then
    return orbit.pages.fill(self, template, env)
  end
end

function web_methods:page_inline(contents, env)
  if not orbit.pages then
    require "orbit.pages"
  end
  local template = orbit.pages.load(nil, contents)
  if template then
    return orbit.pages.fill(self, template, env)
  end
end

function web_methods:empty_param(param)
  return self:empty(self.input[param])
end

for name, func in pairs(wsapi.util) do
  web_methods[name] = function (self, ...)
			return func(...)
		      end
end

local function dispatcher(app_module, method, path)
   if #app_module.dispatch_table[method] == 0 then
      return app_module["handle_" .. method], {}
   else
      for _, item in ipairs(app_module.dispatch_table[method]) do
	 local captures
	 if type(item.pattern) == "string" then
	    captures = { string.match(path, "^" .. item.pattern .. "$") }
	 else
	    captures = { item.pattern:match(path) }
	 end
	 if #captures > 0 then
	    for i = 1, #captures do
	       if type(captures[i]) == "string" then
		  captures[i] = wsapi.util.url_decode(captures[i])
	       end
	    end
	    return item.handler, captures, item.wsapi
	 end
      end
   end
end

local function make_web_object(app_module, wsapi_env)
  local web = { status = "200 Ok", response = "",
		headers = { ["Content-Type"]= "text/html" },
		cookies = {} }
  setmetatable(web, { __index = web_methods })
  web.vars = wsapi_env
  web.prefix = app_module.prefix or wsapi_env.SCRIPT_NAME
  web.suffix = app_module.suffix
  if wsapi_env.APP_PATH == "" then
    web.real_path = app_module.real_path or "."
  else
    web.real_path = wsapi_env.APP_PATH
  end
  web.doc_root = wsapi_env.DOCUMENT_ROOT
  local req = wsapi.request.new(wsapi_env)
  local res = wsapi.response.new(web.status, web.headers)
  web.set_cookie = function (_, name, value)
		     res:set_cookie(name, value)
		   end
  web.delete_cookie = function (_, name, path)
			res:delete_cookie(name, path)
		      end
  web.path_info = req.path_info
  web.path_translated = wsapi_env.PATH_TRANSLATED
  if web.path_translated == "" then web.path_translated = wsapi_env.SCRIPT_FILENAME end
  web.script_name = wsapi_env.SCRIPT_NAME
  web.method = string.lower(req.method)
  web.input, web.cookies = req.params, req.cookies
  web.GET, web.POST = req.GET, req.POST
  return web, res
end

function _M.run(app_module, wsapi_env)
  local handler, captures, wsapi_handler = dispatcher(app_module,
						      string.lower(wsapi_env.REQUEST_METHOD),
						      wsapi_env.PATH_INFO)
  handler = handler or app_module.not_found
  captures = captures or {}
  if wsapi_handler then
    local ok, status, headers, res = xpcall(function () 
					      return handler(wsapi_env, unpack(captures)) 
					    end, debug.traceback)
    if ok then
      return status, headers, res
    else
      handler, captures = app_module.server_error, { status }
    end
  end
  local web, res = make_web_object(app_module, wsapi_env)
  local ok, response = xpcall(function () 
				return handler(web, unpack(captures)) 
			      end, debug.traceback)
  if not ok then
    res.status = "500 Internal Server Error"
    res:write(app_module.server_error(web, response))
  else
    res.status = web.status
    res:write(response)
  end
  return res:finish()
end

return _M
