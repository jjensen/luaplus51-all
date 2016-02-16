-----------------------------------------------------------------------------
-- Xavante HTTP handler
--
-- Authors: Javier Guerra and Andre Carregal
-- Copyright (c) 2004-2007 Kepler Project
--
-- $Id: httpd.lua,v 1.45 2009/08/10 20:00:59 mascarenhas Exp $
-----------------------------------------------------------------------------

local string = require "string"
local table = require "table"
local os = require "os"
local io = require "io"

local socket = require "socket"
local url = require "socket.url"
local copas = require "copas"

local unpack = table.unpack or unpack

local _M = {}

local _serversoftware = ""

local _serverports = {}

local handle_request = {}

function _M.strsplit (str)
        local words = {}

        for w in string.gmatch (str, "%S+") do
                table.insert (words, w)
        end

        return words
end


-- Manages one connection, maybe several requests
-- params:
--              skt : client socket

function _M.connection (skt)
        copas.setErrorHandler (_M.errorhandler)

        skt:setoption ("tcp-nodelay", true)
        local srv, port = skt:getsockname ()
        local req = {
                rawskt = skt,
                srv = srv,
                port = port,
                copasskt = copas.wrap (skt),
        }
        req.socket = req.copasskt
        req.serversoftware = _serversoftware

        while _M.read_method (req) do
                local res
                _M.read_headers (req)

                repeat
                        req.params = nil
                        _M.parse_url (req)
                        res = _M.make_response (req)
                until handle_request[skt.parent] (req, res) ~= "reparse"
                _M.send_response (req, res)

                req.socket:flush ()
                if not res.keep_alive then
                        break
                end
        end
end


function _M.errorhandler (msg, co, skt)
    msg = tostring(msg)
        io.stderr:write("  Xavante Error: "..msg.."\n", "  "..tostring(co).."\n", "  "..tostring(skt).."\n")
        skt:send ("HTTP/1.0 200 OK\r\n")
        skt:send (string.format ("Date: %s\r\n\r\n", os.date ("!%a, %d %b %Y %H:%M:%S GMT")))
        skt:send (string.format ([[
<html><head><title>Xavante Error!</title></head>
<body>
<h1>Xavante Error!</h1>
<p>%s</p>
</body></html>
]], string.gsub (msg, "\n", "<br/>\n")))
end

-- gets and parses the request line
-- params:
--              req: request object
-- returns:
--              true if ok
--              false if connection closed
-- sets:
--              req.cmd_mth: http method
--              req.cmd_url: url requested (as sent by the client)
--              req.cmd_version: http version (usually 'HTTP/1.1')
function _M.read_method (req)
        local err
        req.cmdline, err = req.socket:receive ()

        if not req.cmdline then return nil end
        req.cmd_mth, req.cmd_url, req.cmd_version = unpack (_M.strsplit (req.cmdline))
        req.cmd_mth = string.upper (req.cmd_mth or 'GET')

        -- Account for requests that assume we can handle a proxy.
        -- Just strip off the URL.
        local new_cmd_url = req.cmd_url:match('http://.-(/.+)')
        if not new_cmd_url then
                new_cmd_url = req.cmd_url:match('https://.-(/.+)')
        end
        if new_cmd_url then
                req.cmd_url = new_cmd_url
        end
        req.cmd_url = req.cmd_url or '/'

        return true
end

-- gets and parses the request header fields
-- params:
--              req: request object
-- sets:
--              req.headers: table of header fields, as name => value
function _M.read_headers (req)
        local headers = {}
        local prevval, prevname

        while 1 do
                local l,err = req.socket:receive ()
                if (not l or l == "") then
                        req.headers = headers
                        return
                end
                local _,_, name, value = string.find (l, "^([^: ]+)%s*:%s*(.+)")
                name = string.lower (name or '')
                if name then
                        prevval = headers [name]
                        if prevval then
                                value = prevval .. "," .. value
                        end
                        headers [name] = value
                        prevname = name
                elseif prevname then
                        headers [prevname] = headers [prevname] .. l
                end
        end
end

function _M.parse_url (req)
        local def_url = string.format ("http://%s%s", req.headers.host or "", req.cmd_url or "")

        req.parsed_url = url.parse (def_url or '')
        req.parsed_url.port = req.parsed_url.port or req.port
        req.built_url = url.build (req.parsed_url)

        req.relpath = url.unescape (req.parsed_url.path)
end


-- sets the default response headers
function _M.default_headers (req)
        return  {
                Date = os.date ("!%a, %d %b %Y %H:%M:%S GMT"),
                Server = _serversoftware,
        }
end

function _M.add_res_header (res, h, v)
    if string.lower(h) == "status" then
        res.statusline = "HTTP/1.1 "..v
    else
        local prevval = res.headers [h]
        if (prevval  == nil) then
            res.headers[h] = v
        elseif type (prevval) == "table" then
            table.insert (prevval, v)
        else
            res.headers[h] = {prevval, v}
        end
    end
end


-- sends the response headers
-- params:
--              res: response object
-- uses:
--              res.sent_headers : if true, headers are already sent, does nothing
--              res.statusline : response status, if nil, sends 200 OK
--              res.headers : table of header fields to send
local function send_res_headers (res)
        if (res.sent_headers) then
                return
        end

        if package.loaded["xavante.cookies"] then
          local cookies = require "xavante.cookies"
          xavante.cookies.set_res_cookies (res)
        end

        res.statusline = res.statusline or "HTTP/1.1 200 OK"

        res.socket:send (res.statusline.."\r\n")
        for name, value in pairs (res.headers) do
                if type(value) == "table" then
                  for _, value in ipairs(value) do
                    res.socket:send (string.format ("%s: %s\r\n", name, value))
                  end
                else
                  res.socket:send (string.format ("%s: %s\r\n", name, value))
                end
        end
        res.socket:send ("\r\n")

        res.sent_headers = true;
end

-- sends content directly to client
--              sends headers first, if necesary
-- params:
--              res ; response object
--              data : content data to send
local function send_res_data (res, data)

        if not res.sent_headers then
                send_res_headers (res)
        end

        if not data or data == "" then
                return
        end

        if data then
                if res.chunked then
                        res.socket:send (string.format ("%X\r\n", string.len (data)))
                        res.socket:send (data)
                        res.socket:send ("\r\n")
                else
                        res.socket:send (data)
                end
        end
end

function _M.make_response (req)
        local res = {
                req = req,
                socket = req.socket,
                headers = _M.default_headers (req),
                add_header = _M.add_res_header,
                send_headers = send_res_headers,
                send_data = send_res_data,
        }

        return res
end

-- sends prebuilt content to the client
--              if possible, sets Content-Length: header field
-- params:
--              req : request object
--              res : response object
-- uses:
--              res.content : content data to send
-- sets:
--              res.keep_alive : if possible to keep using the same connection
function _M.send_response (req, res)

        if res.content then
                if not res.sent_headers then
                        if (type (res.content) == "table" and not res.chunked) then
                                res.content = table.concat (res.content)
                        end
                        if type (res.content) == "string" then
                                res.headers["Content-Length"] = string.len (res.content)
                        end
                end
        else
                if not res.sent_headers then
                        res.statusline = "HTTP/1.1 204 No Content"
                        res.headers["Content-Length"] = 0
                end
        end

    if res.chunked then
        res:add_header ("Transfer-Encoding", "chunked")
    end

        if res.chunked or ((res.headers ["Content-Length"]) and req.headers ["connection"] == "Keep-Alive")
        then
                res.headers ["Connection"] = "Keep-Alive"
                res.keep_alive = true
        else
                res.keep_alive = nil
        end

        if res.content then
                if type (res.content) == "table" then
                        for _,v in ipairs (res.content) do res:send_data (v) end
                else
                        res:send_data (res.content)
                end
        else
                res:send_headers ()
        end

        if res.chunked then
                res.socket:send ("0\r\n\r\n")
        end
end

function _M.getparams (req)
        if not req.parsed_url.query then return nil end
        if req.params then return req.params end

        local params = {}
        req.params = params

        for parm in string.gmatch (req.parsed_url.query, "([^&]+)") do
                local k,v = string.match (parm, "(.*)=(.*)")
                k = url.unescape (k)
                v = url.unescape (v)
                if k ~= nil then
                        if params[k] == nil then
                                params[k] = v
                        elseif type (params[k]) == "table" then
                                table.insert (params[k], v)
                        else
                                params[k] = {params[k], v}
                        end
                end
        end

        return params
end

function _M.err_404 (req, res)
        res.statusline = "HTTP/1.1 404 Not Found"
        res.headers ["Content-Type"] = "text/html"
        res.content = string.format ([[
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
<HTML><HEAD>
<TITLE>404 Not Found</TITLE>
</HEAD><BODY>
<H1>Not Found</H1>
The requested URL %s was not found on this server.<P>
</BODY></HTML>]], req.built_url);
        return res
end

function _M.err_403 (req, res)
        res.statusline = "HTTP/1.1 403 Forbidden"
        res.headers ["Content-Type"] = "text/html"
        res.content = string.format ([[
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
<HTML><HEAD>
<TITLE>403 Forbidden</TITLE>
</HEAD><BODY>
<H1>Forbidden</H1>
You are not allowed to access the requested URL %s .<P>
</BODY></HTML>]], req.built_url);
        return res
end

function _M.err_405 (req, res)
        res.statusline = "HTTP/1.1 405 Method Not Allowed"
        res.content = string.format ([[
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
<HTML><HEAD>
<TITLE>405 Method Not Allowed</TITLE>
</HEAD><BODY>
<H1>Not Found</H1>
The Method %s is not allowed for URL %s on this server.<P>
</BODY></HTML>]], req.cmd_mth, req.built_url);
        return res
end

function _M.redirect (res, d)
        res.headers ["Location"] = d
        res.statusline = "HTTP/1.1 302 Found"
        res.content = "redirect"
end

function _M.register (host, port, serversoftware, ssl_params, func)
        local _server = assert(socket.bind(host, port))
        _serversoftware = serversoftware
        local _ip, _port = _server:getsockname()
        _serverports[_port] = true
        handle_request[_server] = func
        copas.addserver(_server, _M.connection, nil, ssl_params)
end

function _M.get_ports()
  local ports = {}
  for k, _ in pairs(_serverports) do
    table.insert(ports, tostring(k))
  end
  return ports
end

return _M
