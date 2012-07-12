-- Tests Copas with a simple Echo server
--
-- Run the test file and the connect to the server by telnet on the used port
-- to stop the test just send the command "quit"

local luaevent = require("luaevent")
local socket = require("socket")

local oldPrint = print
print = function(...)
	oldPrint("SRV", ...)
end

local function echoHandler(skt)
  while true do
    local data,ret = luaevent.receive(skt, 10)
    --print("GOT: ", data, ret)
    if data == "quit" or ret == 'closed' then
      break
    end
    luaevent.send(skt, data)
    collectgarbage()
  end
  skt:close()
  --print("DONE")
end
local server = assert(socket.bind("localhost", 20000))
server:settimeout(0)
local coro = coroutine.create
coroutine.create = function(...)
	local ret = coro(...)
	return ret
end
luaevent.addserver(server, echoHandler)
luaevent.loop()
