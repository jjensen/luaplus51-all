@call %~dp0BootstrapJamPlus.bat
@%~dp0Tools\JamPlus\bin\win32\jam --workspace -gen=vs2012 -config=CreateJamWindowsWorkspace.config Jamfile.jam build2012
