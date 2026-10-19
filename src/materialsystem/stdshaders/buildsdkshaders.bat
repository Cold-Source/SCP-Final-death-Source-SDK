@echo off
setlocal

set BUILD_SHADER=call buildshaders.bat

:: ПРАВИЛЬНЫЙ ПУТЬ к корневой папке исходников (где лежит src)
set SOURCEDIR="C:\Users\REKISX\Desktop\scpfd\SCP-Final-death-Source-SDK\src"

:: Путь к папке вашей игры/мода
set GAMEDIR="C:\Users\REKISX\Desktop\scpfd\scpfd\scpfd\hl2"

%BUILD_SHADER% mymod_dx9_20b			-game %GAMEDIR% -source %SOURCEDIR%
%BUILD_SHADER% mymod_dx9_30				-game %GAMEDIR% -source %SOURCEDIR% -dx9_30	-force30

pause
