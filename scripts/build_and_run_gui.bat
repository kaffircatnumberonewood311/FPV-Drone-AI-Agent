@echo off
REM Build and run GUI with custom CMake
setlocal

set CUSTOM_CMAKE=D:\DroneAI\cmake-4.3.0-rc2-built\bin\cmake.exe
set CUSTOM_CMAKE_ALT=D:\DroneAI\cmake-4.3.0-rc2\bin\cmake.exe

REM Check for custom CMake installation
if exist "%CUSTOM_CMAKE%" (
  set CMAKE_EXE=%CUSTOM_CMAKE%
  echo Using custom CMake: %CUSTOM_CMAKE%
) else if exist "%CUSTOM_CMAKE_ALT%" (
  set CMAKE_EXE=%CUSTOM_CMAKE_ALT%
  echo Using custom CMake: %CUSTOM_CMAKE_ALT%
) else (
  set CMAKE_EXE=cmake
  echo Using system CMake
)

set SCRIPT_DIR=%~dp0
for %%I in ("%SCRIPT_DIR%..") do set PROJECT_ROOT=%%~fI

cd /d "%PROJECT_ROOT%"

echo ==================================================
echo Building GUI
echo ==================================================
echo CMake: %CMAKE_EXE%
echo.

"%CMAKE_EXE%" --version
echo.

REM Build with dev preset (includes GUI)
"%CMAKE_EXE%" --preset dev
if %errorlevel% neq 0 (
  echo Configuration failed!
  exit /b %errorlevel%
)

"%CMAKE_EXE%" --build --preset dev --target nanohawk_agent
if %errorlevel% neq 0 (
  echo Build failed!
  exit /b %errorlevel%
)

echo.
echo ==================================================
echo Build completed! Launching GUI...
echo ==================================================
echo.

set GUI_EXE=%PROJECT_ROOT%\build\dev\nanohawk_agent.exe

if exist "%GUI_EXE%" (
  "%GUI_EXE%"
  exit /b %errorlevel%
) else (
  echo ERROR: GUI executable not found at %GUI_EXE%
  exit /b 1
)

