@echo off
setlocal
cd /d "%~dp0"
if "%XEDK%"=="" (echo ERROR: XEDK is not set.& exit /b 1)
where cmake >nul 2>nul || (echo ERROR: cmake missing& exit /b 1)
where ninja >nul 2>nul || (echo ERROR: ninja missing& exit /b 1)
cmake --preset xbox360-xdk || exit /b 1
cmake --build --preset xbox360-xdk || exit /b 1
if not exist build-xbox360\SeriesDash360.xex (echo ERROR: XEX not produced& exit /b 1)
copy /y build-xbox360\SeriesDash360.xex build-xbox360\default.xex >nul
echo SUCCESS: build-xbox360\default.xex
