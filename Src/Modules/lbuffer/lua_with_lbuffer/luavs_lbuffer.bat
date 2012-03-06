@rem Script to build Lua under "Visual Studio .NET Command Prompt".
@rem Do not run from this directory; run it from the toplevel: etc\luavs.bat .
@rem It creates lua51.dll, lua51.lib, lua.exe, and luac.exe in src.
@rem (contributed by David Manura and Mike Pall)
@if not .%1 == . goto :compile
@echo usage: %0 lua_src_folder (without trailing slash)
@goto :end

:compile

@setlocal
@set LUA_SRC=%1/src
@set MYCOMPILE=cl /nologo /MT /O2 /W3 /c /D_CRT_SECURE_NO_DEPRECATE
@set LBLIB_CFLAGS=/I "%LUA_SRC%" /DLUA_LIB /DLB_REPLACE_LUA_API /FI "%~dp0..\lbuffer.h" /wd4005
@set MYLINK=link /nologo
@set MYLIB=lib /nologo
@set MYMT=mt /nologo

%MYCOMPILE% /DLUA_BUILD_AS_DLL %LUA_SRC%\l*.c
%MYCOMPILE% /DLUA_BUILD_AS_DLL /I "%LUA_SRC%" /I "%~dp0.." ..\*.c *.c
%MYCOMPILE% /DLUA_BUILD_AS_DLL /Dlbaselib_c %LBLIB_CFLAGS% %LUA_SRC%\lbaselib.c
%MYCOMPILE% /DLUA_BUILD_AS_DLL /Dliolib_c %LBLIB_CFLAGS% %LUA_SRC%\liolib.c
%MYCOMPILE% /DLUA_BUILD_AS_DLL /Dlmathlib_c %LBLIB_CFLAGS% %LUA_SRC%\lmathlib.c
%MYCOMPILE% /DLUA_BUILD_AS_DLL /Dloslib_c %LBLIB_CFLAGS% %LUA_SRC%\loslib.c
%MYCOMPILE% /DLUA_BUILD_AS_DLL /Dlstrlib_c %LBLIB_CFLAGS% %LUA_SRC%\lstrlib.c
%MYCOMPILE% /DLUA_BUILD_AS_DLL /Dltablib_c %LBLIB_CFLAGS% %LUA_SRC%\ltablib.c
del lua.obj luac.obj linit.obj
%MYLINK% /DLL /out:lua51.dll l*.obj
move lua51.lib lua51dyn.lib
%MYLIB% /out:lua51.lib l*.obj
if exist lua51.dll.manifest^
  %MYMT% -manifest lua51.dll.manifest -outputresource:lua51.dll;2
%MYCOMPILE% /DLUA_BUILD_AS_DLL %LUA_SRC%\lua.c
%MYLINK% /out:lua.exe lua.obj lua51dyn.lib
if exist lua.exe.manifest^
  %MYMT% -manifest lua.exe.manifest -outputresource:lua.exe
%MYCOMPILE% %LUA_SRC%\l*.c %LUA_SRC%\print.c
del lua.obj linit.obj lbaselib.obj ldblib.obj liolib.obj lmathlib.obj^
    loslib.obj ltablib.obj lstrlib.obj loadlib.obj linit_modified.obj
%MYLINK% /out:luac.exe *.obj
if exist luac.exe.manifest^
  %MYMT% -manifest luac.exe.manifest -outputresource:luac.exe
del *.obj *.manifest

:end
