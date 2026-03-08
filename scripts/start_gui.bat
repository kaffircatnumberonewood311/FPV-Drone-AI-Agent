@echo off
setlocal

REM Use custom CMake version
set CMAKE_ROOT=D:\DroneAI\cmake-4.3.0-rc2
set PATH=%CMAKE_ROOT%\bin;%PATH%

set SCRIPT_DIR=%~dp0
for %%I in ("%SCRIPT_DIR%..") do set PROJECT_ROOT=%%~fI

set GUI_DEV=%PROJECT_ROOT%\build\dev\nanohawk_agent.exe
set GUI_CI=%PROJECT_ROOT%\build\ci\nanohawk_agent.exe
set CLI_DEV=%PROJECT_ROOT%\build\dev\nanohawk_agent_cli.exe
set CLI_CI=%PROJECT_ROOT%\build\ci\nanohawk_agent_cli.exe

if exist "%GUI_DEV%" (
  "%GUI_DEV%"
  exit /b %errorlevel%
)

if exist "%GUI_CI%" (
  "%GUI_CI%"
  exit /b %errorlevel%
)

if exist "%CLI_DEV%" (
  "%CLI_DEV%"
  exit /b %errorlevel%
)

if exist "%CLI_CI%" (
  "%CLI_CI%"
  exit /b %errorlevel%
)

echo No built executable found. Build with: cmake --build --preset dev --target nanohawk_agent_cli
exit /b 1
