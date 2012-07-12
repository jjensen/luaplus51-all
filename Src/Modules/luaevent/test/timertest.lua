local core = require("luaevent.core")

c = core.new()
local f = 100
local function createEvent()
	return c:addevent(nil, core.EV_TIMEOUT, function(ev)
		io.write(".." .. f)
		f = f - 1
		if f < 0 then
			return -1
		end
		collectgarbage()
	end, 0.01)
end
ev = createEvent()
print("TESTING Garbage-collect-safe version")
c:loop()
assert(f < 0, "DID NOT FINISH LOOPING")
io.write("\n")
print("TESTING Garbage-collect unsafe version")
f = 100
createEvent()
c:loop()
assert(f >= 0, "Did not perform expected collection")
io.write("\n")
print("Completed both tests")
