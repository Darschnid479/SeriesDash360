@echo off
setlocal
cd /d "%~dp0"
title SeriesDash360 - Zero Admin Build Launcher
echo ============================================================
echo  SeriesDash360 - ZERO ADMIN BUILD LAUNCHER
echo ============================================================
echo.
echo This window will stay open even if the worker crashes.
echo.
cmd.exe /d /k call "%~dp0BUILD_XBOX360_ZERO_ADMIN_WORKER.bat"
