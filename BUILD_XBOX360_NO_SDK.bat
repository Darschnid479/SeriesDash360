@echo off
setlocal EnableExtensions
title SeriesDash360 - FULL SDK-FREE BUILD
cd /d "%~dp0"

echo ============================================================
echo  SeriesDash360 1.0 - FULL SDK-FREE BUILD
echo ============================================================
echo.
echo This build uses:
echo   - WSL/Ubuntu
echo   - OpenXeChain LLVM/Clang
echo   - OpenXeChain newlib/xecorelib
echo   - SeriesDashXEX
echo.
echo It does NOT require Microsoft XDK or imagexex.exe.
echo.

where wsl.exe >nul 2>nul
if errorlevel 1 (
  echo WSL is not installed.
  echo Attempting to install WSL + Ubuntu...
  wsl --install -d Ubuntu
  echo.
  echo Windows may require a reboot. Finish Ubuntu first-run setup,
  echo then run this BAT again.
  pause
  exit /b 2
)

wsl.exe -l -q | findstr /i "Ubuntu" >nul
if errorlevel 1 (
  echo Ubuntu is not installed in WSL.
  echo Installing Ubuntu...
  wsl --install -d Ubuntu
  echo.
  echo Finish Ubuntu first-run setup, then run this BAT again.
  pause
  exit /b 2
)

echo [1/3] Converting project path for WSL...
for /f "usebackq delims=" %%P in (`wsl.exe -d Ubuntu -- wslpath -a "%CD%"`) do set "WSL_PROJECT=%%P"
if "%WSL_PROJECT%"=="" (
  echo ERROR: Could not translate the project path into WSL.
  goto :fail
)
echo   %WSL_PROJECT%
echo.

echo [2/3] Bootstrapping/building OpenXeChain...
echo.
echo First run builds LLVM and is much heavier than later builds.
echo The result is cached under .openxechain.
echo.
wsl.exe -d Ubuntu -- bash -lc "cd '%WSL_PROJECT%' && chmod +x BOOTSTRAP_OPENXECHAIN.sh platform/openxechain/build.sh && ./BOOTSTRAP_OPENXECHAIN.sh"
if errorlevel 1 goto :fail

echo.
echo [3/3] Checking output...
if not exist "build-open\default.xex" (
  echo ERROR: OpenXeChain completed without producing default.xex.
  goto :fail
)

echo.
echo ============================================================
echo  SUCCESS
echo ============================================================
echo.
echo SDK-free XEX:
echo   %CD%\build-open\default.xex
echo.
echo No Microsoft XDK was used.
echo.
pause
exit /b 0

:fail
echo.
echo ============================================================
echo  SDK-FREE BUILD FAILED
echo ============================================================
echo.
echo Read the compiler/toolchain error above.
echo OpenXeChain build log, when present:
echo   .openxechain\src\buildscript\build.log
echo.
pause
exit /b 1
