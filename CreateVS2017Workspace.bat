@call %~dp0BootstrapJamPlus.bat
@%~dp0Tools\JamPlus\bin\win32\jam --workspace -gen=vs2017 --compiler=vs2017 -config=CreateJamWindowsWorkspace.config Jamfile.jam build2017%1
