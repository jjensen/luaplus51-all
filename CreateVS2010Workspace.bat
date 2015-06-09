@call %~dp0BootstrapJamPlus.bat
@%~dp0Tools\JamPlus\bin\win32\jam --workspace -gen=vs2010 -compiler=vs2010 -config=CreateJamWindowsWorkspace.config Jamfile.jam build2010%1
