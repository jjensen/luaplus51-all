#!/usr/bin/env cgilua.cgi
<html>
<head><title>Embeded Lua Test</title></head>

<body>
cgilua.QUERY =
<?lua
cgilua.serialize = require"cgilua.serialize".serialize
cgilua.serialize (cgilua.QUERY, cgilua.put)
?>
<br>
cgilua.POST =
<?lua
cgilua.serialize (cgilua.POST, cgilua.put)
?>
<br>
Remote address: <%= cgilua.servervariable"REMOTE_ADDR" %>
<br>
Is persistent = <%= tostring (SAPI.Info.ispersistent) %>
<br>
ap = <?lua= tostring(ap) ?> <br>
lfcgi = <% = tostring(lfcgi) %> <br>
<%= (ap and ap.handler()) or "" %> <br>

</body>
<small>$Id: test_main.lp,v 1.5 2005/02/11 14:39:21 tomas Exp $</small>
</html>
