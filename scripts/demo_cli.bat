@echo off
setlocal

set SCRIPT_DIR=%~dp0
for %%I in ("%SCRIPT_DIR%..") do set PROJECT_ROOT=%%~fI

set CLI_DEV=%PROJECT_ROOT%\build\dev\nanohawk_agent_cli.exe
set CLI_CI=%PROJECT_ROOT%\build\ci\nanohawk_agent_cli.exe

set CLI_EXE=
if exist "%CLI_DEV%" set CLI_EXE=%CLI_DEV%
if "%CLI_EXE%"=="" if exist "%CLI_CI%" set CLI_EXE=%CLI_CI%

if "%CLI_EXE%"=="" (
  echo No CLI executable found. Build first with: cmake --build --preset dev --target nanohawk_agent_cli
  exit /b 1
)

echo.
echo === NanoHawk Agent CLI - Demo Mode ===
echo.
echo Testing single-shot command mode:
echo.
"%CLI_EXE%" "takeoff to 2 meters and hover"
echo.
echo.
echo === To use interactive mode, run: ===
echo %CLI_EXE%
echo.
echo Then type mission prompts and press Enter.
echo Type 'quit' to exit.
echo.
pause
