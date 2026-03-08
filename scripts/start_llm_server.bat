@echo off
setlocal

set SCRIPT_DIR=%~dp0
for %%I in ("%SCRIPT_DIR%..\..") do set DRONEAI_ROOT=%%~fI
for %%I in ("%SCRIPT_DIR%..") do set PROJECT_ROOT=%%~fI

set LLAMA_SERVER=%DRONEAI_ROOT%\llama.cpp\build\bin\Release\llama-server.exe
set MODEL_PATH=%PROJECT_ROOT%\models\llm\model.gguf

if not exist "%LLAMA_SERVER%" (
  echo llama-server executable not found at %LLAMA_SERVER%
  exit /b 1
)

"%LLAMA_SERVER%" -m "%MODEL_PATH%" --host 127.0.0.1 --port 8080
