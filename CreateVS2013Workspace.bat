@call %~dp0BootstrapJamPlus.bat
@%~dp0Tools\JamPlus\bin\win32\jam --workspace -gen=vs2013 --compiler=vs2013 -config=CreateJamWindowsWorkspace.config Jamfile.jam build2013%1
