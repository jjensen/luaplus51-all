@call %~dp0BootstrapJamPlus.bat
@%~dp0Tools\JamPlus\bin\win32\jam --workspace -gen=vs2015 --compiler=vs2015 -config=CreateJamWindowsWorkspace.config Jamfile.jam build2015%1
