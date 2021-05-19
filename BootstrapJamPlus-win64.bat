@echo off
call git submodule update --init
call %~dp0Tools\JamPlus\bootstrap-win64-vc.bat

