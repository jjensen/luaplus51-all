@call %~dp0BootstrapJamPlus.bat
@%~dp0Tools\JamPlus\bin\win32\jam --workspace -gen=vs2019 --compiler=vs2019 -config=CreateJamWindowsWorkspace.config Jamfile.jam build2019%1
