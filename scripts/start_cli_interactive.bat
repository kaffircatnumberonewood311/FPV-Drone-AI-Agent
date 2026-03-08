@echo off
setlocal

set SCRIPT_DIR=%~dp0
for %%I in ("%SCRIPT_DIR%..") do set PROJECT_ROOT=%%~fI

set CLI_DEV=%PROJECT_ROOT%\build\dev\nanohawk_agent_cli.exe
set CLI_CI=%PROJECT_ROOT%\build\ci\nanohawk_agent_cli.exe

echo === Starting NanoHawk Agent CLI ===
echo.
echo This is an interactive mission planning interface.
echo Enter natural language commands to control the drone.
echo Type 'quit' to exit.
echo.

if exist "%CLI_DEV%" (
  "%CLI_DEV%"
  exit /b %errorlevel%
)

if exist "%CLI_CI%" (
  "%CLI_CI%"
  exit /b %errorlevel%
)

echo No CLI executable found. Build first with: cmake --build --preset dev --target nanohawk_agent_cli
exit /b 1
