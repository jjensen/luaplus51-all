@echo off
call git submodule update --init
call %~dp0Tools\JamPlus\bootstrap-win32.bat

