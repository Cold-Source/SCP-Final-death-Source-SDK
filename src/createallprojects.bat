@echo off
pushd "%~dp0"
devtools\bin\vpc.exe /2022 /win64 /hl2 /define:SOURCESDK +everything /mksln everything.sln
set "vpcResult=%errorlevel%"
popd
pause
exit /b %vpcResult%
