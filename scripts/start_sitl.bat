@echo off
setlocal

set SCRIPT_DIR=%~dp0
for %%I in ("%SCRIPT_DIR%..\..") do set DRONEAI_ROOT=%%~fI
set SITL_SCRIPT=%DRONEAI_ROOT%\ardupilot-master\Tools\autotest\sim_vehicle.py

if not exist "%SITL_SCRIPT%" (
  echo sim_vehicle.py not found at %SITL_SCRIPT%
  exit /b 1
)

python "%SITL_SCRIPT%" -v ArduCopter --console --map
