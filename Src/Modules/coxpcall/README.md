# Coxpcall

Coxpcall encapsulates the protected calls with a coroutine based loop,
so errors can be handled without the usual pcall/xpcall issues with coroutines
for Lua 5.1.

http://keplerproject.github.io/coxpcall/

Using Coxpcall usually consists in simply loading the module and then replacing Lua
pcall and xpcall by copcall and coxpcall.

Coxpcall is free software and uses the same license as Lua 5.1.

Coxpcall can be downloaded from its GitHub page. You can also get Coxpcall using LuaRocks:
luarocks install coxpcall

Lua 5.2 was extended with the Coxpcall functionality and hence it is no longer required. The
5.2+ compatibility by coxpcall means that it maintains backward compatibility while using
the built-in Lua implementation.
