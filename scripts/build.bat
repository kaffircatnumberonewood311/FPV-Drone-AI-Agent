@echo off
REM Build script using custom CMake D:\DroneAI\cmake-4.3.0-rc2
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
echo Building nanohawk-agent
echo ==================================================
echo CMake: %CMAKE_EXE%
echo Project: %PROJECT_ROOT%
echo.

REM Show CMake version
"%CMAKE_EXE%" --version
echo.

REM Default to 'ci' preset if no argument provided
set PRESET=%1
if "%PRESET%"=="" set PRESET=ci

echo Configuring with preset: %PRESET%
"%CMAKE_EXE%" --preset %PRESET%
if %errorlevel% neq 0 (
  echo Configuration failed!
  exit /b %errorlevel%
)

echo.
echo Building with preset: %PRESET%
"%CMAKE_EXE%" --build --preset %PRESET%
if %errorlevel% neq 0 (
  echo Build failed!
  exit /b %errorlevel%
)

echo.
echo ==================================================
echo Build completed successfully!
echo ==================================================
echo Executables in: %PROJECT_ROOT%\build\%PRESET%
echo.

