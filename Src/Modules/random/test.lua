-- test random library

local random=require"random"

print(random.version)
print""

function test(w,r)
 print(w,r(),r(),r())
end

r=random.new(12345)
test("new",r)
test("more",r)
test("reset",r:seed(12345))
test("seed",r:seed(56789))
test("seed",r:seed(3063121584))
test("seed",r:seed(2428144928))
s=r:seed():clone()
test("seed",r)
test("more",r)
test("clone",s)

r:seed(os.time())
N=100000
print""
print("range","","distribution",N)
print("","",1,2,3,4,5,6,7,8)

function test(N,a,b)
 local S={0,0,0,0,0,0,0,0,0,0,0}
 for i=1,N do
  local i=r:value(a,b)
  if i~=nil then S[i]=S[i]+1 end
 end
 for i=1,9 do
  S[i]=math.floor(100*S[i]/N+0.5)
 end
 print(a..":"..b,"",S[1],S[2],S[3],S[4],S[5],S[6],S[7],S[8])
end

test(N,1,8)
test(N,2,4)
test(N,3,7)
test(N,7,3)
test(N,1.2,4.6)
test(N,4.6,1.2)
test(N,7.1,2.9)
test(N,2.9,7.1)
test(N,2.1,2.2)
test(N,2.2,2.1)

function test(w,f)
 local t=os.clock()
 for i=1,N do
  f()
 end
 t=os.clock()-t
 print(w,math.floor(N/t/1000),N,t)
end

N=4*N
print""
print("","1000/s","N","time")
test("math",function () return math.random() end)
test("math",math.random)
test("random",function () return random.value(r) end)
test("random",function () return r:value() end)
test("random",r)

print""
print(random.version)

-- eof
