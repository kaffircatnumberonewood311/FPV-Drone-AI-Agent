@echo off
REM Simple GUI build and run script
setlocal enabledelayedexpansion

echo.
echo ==================================================
echo  Building nanohawk-agent GUI
echo ==================================================
echo.

cd /d D:\DroneAI\nanohawk-agent

REM Configure
echo [1/3] Configuring with dev preset...
cmake --preset dev
if errorlevel 1 (
    echo ERROR: Configuration failed
    echo Check that Qt 6.5+ is installed and vcpkg path is correct
    pause
    exit /b 1
)

REM Build
echo.
echo [2/3] Building GUI target...
cmake --build --preset dev --target nanohawk_agent
if errorlevel 1 (
    echo ERROR: Build failed
    pause
    exit /b 1
)

REM Run
echo.
echo [3/3] Launching GUI...
echo ==================================================
echo.

if exist "build\dev\nanohawk_agent.exe" (
    start "" "build\dev\nanohawk_agent.exe"
    echo GUI launched successfully!
    echo.
    echo The application should now be running.
    echo Check the VideoPane for your drone's camera feed.
    echo.
    pause
) else (
    echo ERROR: GUI executable not found
    echo Expected: build\dev\nanohawk_agent.exe
    pause
    exit /b 1
)

