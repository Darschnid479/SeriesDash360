@echo off
setlocal
cd /d "%~dp0"
where cmake >nul 2>nul
if errorlevel 1 (
  echo CMake was not found in PATH.
  echo Install CMake and Visual Studio Build Tools, then run this file again.
  pause
  exit /b 1
)
cmake -S . -B build
if errorlevel 1 goto :fail
cmake --build build --config Release
if errorlevel 1 goto :fail
echo.
echo Build completed.
echo Open preview\index.html for the Series-style UI.
pause
exit /b 0
:fail
echo.
echo Build failed. Review the errors above.
pause
exit /b 1
