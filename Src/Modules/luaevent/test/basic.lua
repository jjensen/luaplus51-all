local luaevent = require("luaevent")

print("Version:", luaevent._NAME.." "..luaevent._VERSION)
print("libevent version:", luaevent.core.libevent_version())
print("")
base = luaevent.core.new()
print("Testing creating base object:", type(base) == "userdata" and "OK" or "FAIL")
print("libevent backend:", base:method())
