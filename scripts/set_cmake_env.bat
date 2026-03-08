@echo off
REM Set CMake environment to use D:\DroneAI\cmake-4.3.0-rc2

set CMAKE_ROOT=D:\DroneAI\cmake-4.3.0-rc2
set PATH=%CMAKE_ROOT%\bin;%PATH%

echo CMake environment configured:
echo CMAKE_ROOT=%CMAKE_ROOT%
echo.

cmake --version

