@echo off
pushd "%~dp0"
devtools\bin\vpc.exe /win32 /sdk2013ce +game /mksln Game.sln
set "vpcResult=%errorlevel%"
popd
pause
exit /b %vpcResult%
