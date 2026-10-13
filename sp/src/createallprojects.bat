@echo off
pushd "%~dp0"
devtools\bin\vpc.exe /win32 /sdk2013ce +everything /mksln Everything.sln
set "vpcResult=%errorlevel%"
popd

pause
exit /b %vpcResult%
