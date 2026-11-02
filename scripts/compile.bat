@echo off
setlocal
cd /d "%~dp0.."
where cmake >nul 2>nul
if errorlevel 1 (
  echo CMake is required. Use a Visual Studio Developer Command Prompt with CMake installed.
  exit /b 1
)
cmake -S . -B build -A x64 -DBUILD_TESTING=ON
if errorlevel 1 exit /b 1
cmake --build build --config Release
if errorlevel 1 exit /b 1
if not exist dist mkdir dist
copy /y build\Release\AeroflyBridge.dll dist\AeroflyBridge.dll
exit /b %errorlevel%
