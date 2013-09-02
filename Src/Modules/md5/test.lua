-- test md5 library

function report(w,s,F)
 print(w,s.."  "..F)
 assert(s==KNOWN)
end

function test(D,known)
 if D==nil then return end
 KNOWN=known
 print""
 print(D.version)
 print""

 assert(io.input(F))
 report("all",D.digest(io.read"*a"),F)

 assert(io.input(F))
 d=D.new()
 while true do
  local c=io.read(1)
  if c==nil then break end
  d:update(c)
 end
 report("loop",d:digest(),F)
 report("again",d:digest(),F)

 assert(io.input(F))
 d:reset()
 while true do
  local c=io.read(math.random(1,16))
  if c==nil then break end
  d:update(c)
 end
 report("reset",d:digest(),F)

 report("known",KNOWN,F)

 local A="hello"
 local B="world"
 local C=A..B
 local a=D.digest(C)
 local b=d:reset():update(C):digest()
 assert(a==b)
 local b=d:reset():update(A,B):digest()
 assert(a==b)
end

F="README"
test(md2,'2ef85a6b0eed63eba808537ce97a58bb')
test(md4,'8805be41e3316bb202e4dd0151768e1c')
test(md5,'eca9cf346fba9f98c4e80f0028a2df91')
test(sha1,'5b3ddaf01a036aa25f27b6591ed23cfcfa6693a2')
test(sha224,'e9f4437b7dcd6bed34c84eed54f14fd3e02cb9956677fc2470ee49dd')
test(sha256,'b5d0e58c310da7b0feb19b4c51088427f1860537aeac485f8a97f2b649fe1fe4')
test(sha384,'28ccc9c6006c7115ca971b65058b637bb9a7990db77b534d083192c6a98a5af32d604b30625ad7235060cdf7290c387d')
test(sha512,'c803f6ffdbafee6b93b71252c3a8379be61666ab1aafea4ff30319fae134c692ad873366eba11352022d76821615f309af5e9de1337cf6bb22ec8d22b3f0db58')
test(ripemd160,'c03d569c2cc11e92885396c5cc61c3c632859258')

-- eof
