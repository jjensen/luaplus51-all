-------------------------------------------------------------------------------
-- Coroutine safe xpcall and pcall versions
--
-- Encapsulates the protected calls with a coroutine based loop, so errors can
-- be dealed without the usual Lua 5.x pcall/xpcall issues with coroutines
-- yielding inside the call to pcall or xpcall.
--
-- Authors: Roberto Ierusalimschy and Andre Carregal
-- Contributors: Thomas Harning Jr., Ignacio Burgueño, Fabio Mascarenhas
--
-- Copyright 2005 - Kepler Project
--
-- $Id: coxpcall.lua,v 1.13 2008/05/19 19:20:02 mascarenhas Exp $
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Checks if (x)pcall function is coroutine safe
-------------------------------------------------------------------------------
local function isCoroutineSafe(func)
    local co = coroutine.create(function()
        return func(coroutine.yield, function() end)
    end)

    coroutine.resume(co)
    return coroutine.resume(co)
end

-- No need to do anything if pcall and xpcall are already safe.
if isCoroutineSafe(pcall) and isCoroutineSafe(xpcall) then
    copcall = pcall
    coxpcall = xpcall
    return { pcall = pcall, xpcall = xpcall, running = coroutine.running }
end

-------------------------------------------------------------------------------
-- Implements xpcall with coroutines
-------------------------------------------------------------------------------
local performResume, handleReturnValue
local oldpcall = pcall
local pack = table.pack or function(...) return {n = select("#", ...), ...} end
local unpack = table.unpack or unpack
local running = coroutine.running
local coromap = setmetatable({}, { __mode = "k" })
  
function handleReturnValue(err, co, status, ...)
    if not status then
        return false, err(debug.traceback(co, (...)), ...)
    end
    if coroutine.status(co) == 'suspended' then
        return performResume(err, co, coroutine.yield(...))
    else
        return true, ...
    end
end

function performResume(err, co, ...)
    return handleReturnValue(err, co, coroutine.resume(co, ...))
end

function coxpcall(f, err, ...)
    local res, co = oldpcall(coroutine.create, f)
    if not res then
        local params = pack(...)
        local newf = function() return f(unpack(params, 1, params.n)) end
        co = coroutine.create(newf)
    end
    coromap[co] = (running() or "mainthread")
    return performResume(err, co, ...)
end

local function corunning(coro)
  if coro ~= nil then
    assert(type(coro)=="thread", "Bad argument; expected thread, got: "..type(coro))
  else
    coro = running()
  end
  while coromap[coro] do
    coro = coromap[coro]
  end
  if coro == "mainthread" then return nil end
  return coro
end

-------------------------------------------------------------------------------
-- Implements pcall with coroutines
-------------------------------------------------------------------------------

local function id(trace, ...)
  return ...
end

function copcall(f, ...)
    return coxpcall(f, id, ...)
end

return { pcall = copcall, xpcall = coxpcall, running = corunning }
