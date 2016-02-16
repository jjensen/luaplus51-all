-----------------------------------------------------------------------------
-- Xavante CodeWeb
--
-- Author: Javier Guerra
-- Copyright (c) 2005 Kepler Project
-----------------------------------------------------------------------------

local prep = require "cgilua.prep"
local httpd = require "xavante.httpd"

local loadstring = loadstring or load

local _M = {}

-- Adds a module as a website;
-- each public function is registered as a page handler
--      host,urlpath : where to insert the handlers
--      m : table with functions, a module
--      register_as_tree: if true, each handler is registered as a directory too
function _M.addModule (host, urlpath, m, register_as_tree)
        if m.__main then
                httpd.addHandler (host, urlpath, m.__main)
        end
        for k,v in pairs (m) do
                if      type (k) == "string" and
                        string.sub (k,1,1) ~= "_" and
                        type (v) == "function"
                then
                        local pth = urlpath.."/"..k
                        httpd.addHandler (host, pth, v)
                        if register_as_tree then
                                httpd.addHandler (host, pth.."/", v)
                                httpd.addHandler (host, pth.."/*", v)
                        end
                end
        end
end

-- Builds a handler from a template
-- the template (loaded from file 'fname') uses LuaPage syntax
-- and is converted to a function in the form: "function (req,res,...)"
-- if env is a table, the resulting function uses it as environment
-- if it's a function, it's environment is used
-- if it's a number, it select's the environment from the call stack
-- if ommited, 1 is implied, (the calling function's environment)
function _M.load_cw (fname, env)

        env = env or 1

        if type (env) == "function" then
                env = getfenv (env)
        elseif type (env) == "number" then
                env = getfenv (env+1)
        end

        local fh = assert (io.open (fname))
        local prog = fh:read("*a")
        fh:close()
        prep.setoutfunc ("res:send_data")
        prog = prep.translate (prog, "file "..fname)
        prog = "return function (req,res,...)\n" .. prog .. "\nend"
        if prog then
                local f, err = loadstring (prog, "@"..fname)
                if f then
                        local f_cw = f()
                        if env then setfenv (f_cw, env) end
                        return f_cw
                else
                        error (err)
                end
        end
end

