@echo off
setlocal
cd /d "%~dp0"
title SeriesDash360 - Zero Admin Diagnostics
echo ============================================================
echo  SeriesDash360 - ZERO ADMIN DIAGNOSTICS
echo ============================================================
echo.
echo Folder:
echo   %CD%
echo.
echo Windows:
ver
echo.
echo Command processor:
where cmd.exe
echo.
echo PowerShell:
where powershell.exe 2>nul
echo.
echo Curl:
where curl.exe 2>nul
echo.
echo Existing portable MSYS2:
if exist ".tools\msys64\usr\bin\bash.exe" (
  echo   FOUND
  ".tools\msys64\usr\bin\bash.exe" --version
) else (
  echo   NOT FOUND
)
echo.
echo Existing log:
if exist "ZERO_ADMIN_BUILD.log" (
  echo ------------------------------------------------------------
  type "ZERO_ADMIN_BUILD.log"
  echo ------------------------------------------------------------
) else (
  echo   No log exists yet.
)
echo.
pause
