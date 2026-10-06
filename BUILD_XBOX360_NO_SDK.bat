@echo off
setlocal EnableExtensions
title SeriesDash360 - NO SDK XEX Packer
cd /d "%~dp0"

echo ============================================================
echo  SeriesDash360 - SDK-FREE XEX2 PACKER
echo ============================================================
echo.
echo This path does NOT use Microsoft XDK or imagexex.exe.
echo It packages an Xbox 360 POWERPCBE/XBOX-subsystem PE into an
echo unsigned homebrew XEX2 for an already patched/homebrew host.
echo.

where python >nul 2>nul
if errorlevel 1 (
  where py >nul 2>nul
  if not errorlevel 1 goto :use_py
  echo Python is missing. Trying winget...
  where winget >nul 2>nul || goto :python_fail
  winget install --id Python.Python.3.13 -e --accept-package-agreements --accept-source-agreements
  if errorlevel 1 goto :python_fail
)

:use_python
set "PY=python"
goto :python_ready

:use_py
set "PY=py -3"

:python_ready
set "INPUT=%~1"
if "%INPUT%"=="" (
  if exist "build-open\SeriesDash360.exe" set "INPUT=build-open\SeriesDash360.exe"
)
if "%INPUT%"=="" (
  if exist "build-open\SeriesDash360.pe" set "INPUT=build-open\SeriesDash360.pe"
)
if "%INPUT%"=="" (
  echo.
  echo No Xbox 360 PE input was found.
  echo.
  echo Usage:
  echo   BUILD_XBOX360_NO_SDK.bat path\to\SeriesDash360.exe
  echo.
  echo The input must already be compiled for:
  echo   Machine:   POWERPCBE 0x01F2
  echo   Subsystem: XBOX      0x000E
  echo.
  echo SeriesDashXEX replaces imagexex/XDK packaging only.
  echo An open compiler/linker such as OpenXeChain must create the PE.
  echo.
  pause
  exit /b 2
)

if not exist "%INPUT%" (
  echo ERROR: Input PE not found: %INPUT%
  pause
  exit /b 1
)

if not exist "tools\SeriesDashXEX\seriesdashxex.py" (
  echo ERROR: SeriesDashXEX packer is missing.
  pause
  exit /b 1
)

if not exist "build-xbox360-open" mkdir "build-xbox360-open"

echo Packing:
echo   %INPUT%
echo.
%PY% "tools\SeriesDashXEX\seriesdashxex.py" pack "%INPUT%" "build-xbox360-open\default.xex" --kernel-build 17559
if errorlevel 1 goto :fail

%PY% "tools\SeriesDashXEX\seriesdashxex.py" verify "build-xbox360-open\default.xex"
if errorlevel 1 goto :fail

echo.
echo ============================================================
echo  SUCCESS - SDK-FREE XEX2 CREATED
echo ============================================================
echo.
echo   %CD%\build-xbox360-open\default.xex
echo.
echo NOTE: This XEX is unsigned and intended for an already patched
echo BadUpdate/FreeMyXe/XeUnshackle-style homebrew environment.
echo It will not run as a retail-signed title on an unmodified console.
echo.
pause
exit /b 0

:python_fail
echo ERROR: Python could not be installed automatically.
pause
exit /b 1

:fail
echo.
echo BUILD FAILED. Read the SeriesDashXEX error above.
echo.
pause
exit /b 1
