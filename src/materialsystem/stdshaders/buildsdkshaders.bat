@echo off
setlocal
pushd "%~dp0"
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0buildpbrshaders.ps1"
set "build_result=%ERRORLEVEL%"
popd
exit /b %build_result%
