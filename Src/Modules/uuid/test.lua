-- test uuid library

require"uuid"

print(uuid.version)
print""

u=uuid.new()
print(u,"default",uuid.time(u))

u=uuid.new("default")
print(u,"default",uuid.time(u))

r=uuid.new("random")
print(r,"random",uuid.time(r))

t=uuid.new("time")
print(t,"time",uuid.time(t))

os.execute"sleep 2"

t=uuid.new("time")
print(t,"time",uuid.time(t))
print""

function test(x,ok)
 print(x,uuid.isvalid(x),ok)
end

test("84949cc5-4701-4a84-895b-354c584a981b", true)
test("84949CC5-4701-4A84-895B-354C584A981B", true)
test("84949cc5-4701-4a84-895b-354c584a981bc", false)
test("84949cc5-4701-4a84-895b-354c584a981", false)
test("84949cc5x4701-4a84-895b-354c584a981b", false)
test("84949cc504701-4a84-895b-354c584a981b", false)
test("84949cc5-470104a84-895b-354c584a981b", false)
test("84949cc5-4701-4a840895b-354c584a981b", false)
test("84949cc5-4701-4a84-895b0354c584a981b", false)
test("g4949cc5-4701-4a84-895b-354c584a981b", false)
test("84949cc5-4701-4a84-895b-354c584a981g", false)

print""
print(uuid.version)
