#!/usr/bin/env cgilua.cgi
-- $Id: test_main.lua,v 1.11 2006/01/06 16:33:57 tomas Exp $

cgilua.serialize = require"cgilua.serialize".serialize

cgilua.htmlheader()
cgilua.put[[
<html>
<head><title>Script Lua Test</title></head>

<body>
cgilua.QUERY =
]]
cgilua.serialize (cgilua.QUERY, cgilua.put)
cgilua.put[[
<br>
cgilua.POST =
]]
cgilua.serialize (cgilua.POST, cgilua.put)
cgilua.put "<br>\n"
cgilua.put ("Remote address: "..cgilua.servervariable"REMOTE_ADDR")
cgilua.put "<br>\n"
cgilua.put ("Is persistent = "..tostring (SAPI.Info.ispersistent).."<br>\n")
cgilua.put ("ap="..tostring(ap).."<br>\n")
cgilua.put ("lfcgi="..tostring(lfcgi).."<br>\n")

-- Checking Virtual Environment
local my_output = cgilua.put
cgilua.put = nil
local status, err = pcall (function ()
	assert (cgilua.put == nil, "cannot change cgilua.put value")
end)
cgilua.put = my_output
assert (status == true, err)

-- Checking require
local status, err = pcall (function () require"unknown_module" end)
assert (status == false, "<tt>unknown_module</tt> loaded!")
--assert (package == nil, "Access to <tt>package</tt> table allowed!")

cgilua.put[[
<p>
<small>$Id: test_main.lua,v 1.11 2006/01/06 16:33:57 tomas Exp $</small>
</body>
</html>
]]
cgilua = nil
