#!/usr/bin/env wsapi.cgi
 
require "orbit"
require "orbit.routes"

local R = orbit.routes.R
 
local hello = orbit.new()

hello:dispatch_get(function (web)
		      return string.format('<h1>Welcome to %s!</h1>', web.real_path)
		   end, R'/')

hello:dispatch_get(function(web, params)
		      return string.format('Hello %s!', params.name)
		   end, R'/hello/:name')

hello:dispatch_get(function(web, params)
		      return string.format('Hi %s!', params.splat[1])
		   end, R'/hi/*')

hello:dispatch_get(function(web, params)
		      return string.format('Hey %s!', params.name or "stranger")
		   end, R'/hey/?:name?')
 
return hello
