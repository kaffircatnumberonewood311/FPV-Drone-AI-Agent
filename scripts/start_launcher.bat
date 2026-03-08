@echo off
setlocal

set SCRIPT_DIR=%~dp0
for %%I in ("%SCRIPT_DIR%..") do set PROJECT_ROOT=%%~fI
set LAUNCHER_EXE=%PROJECT_ROOT%\build\ci\launcher.exe

if exist "%LAUNCHER_EXE%" (
  "%LAUNCHER_EXE%"
  exit /b %errorlevel%
)

echo launcher.exe not found. Build tools/launcher first.
exit /b 1
