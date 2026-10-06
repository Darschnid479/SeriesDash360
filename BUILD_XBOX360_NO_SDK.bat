@echo off
setlocal EnableExtensions EnableDelayedExpansion
title SeriesDash360 - FULL SDK-FREE Xbox 360 Builder
cd /d "%~dp0"

echo ============================================================
echo  SeriesDash360 - FULL SDK-FREE XBOX 360 BUILDER
echo ============================================================
echo.
echo No Microsoft XDK. No imagexex.exe.
echo.
echo This builder will:
echo   1. Use/install WSL Ubuntu
echo   2. Install open build dependencies
echo   3. Download/build OpenXeChain
echo   4. Compile an Xbox 360 POWERPCBE PE
echo   5. Package it with SeriesDashXEX
echo   6. Produce build-xbox360-open\default.xex
echo.
echo NOTE: The first OpenXeChain build is large and CPU intensive.
echo Subsequent builds reuse the cached toolchain.
echo.

rem If the user supplied a PE explicitly, keep the fast pack-only mode.
if not "%~1"=="" goto :pack_existing

echo [1/6] Checking WSL...
where wsl.exe >nul 2>nul
if errorlevel 1 (
  echo ERROR: wsl.exe is unavailable on this Windows installation.
  echo Enable Windows Subsystem for Linux, then run this BAT again.
  goto :fail
)

wsl.exe --status >nul 2>nul
if errorlevel 1 (
  echo   WSL is installed but not initialized.
)

wsl.exe -d Ubuntu -u root -- bash -lc "exit 0" >nul 2>nul
if errorlevel 1 (
  echo.
  echo Ubuntu is not installed or initialized in WSL.
  echo Attempting automatic installation...
  wsl.exe --install -d Ubuntu
  if errorlevel 1 (
    echo.
    echo ERROR: Automatic Ubuntu installation failed.
    echo Re-run this BAT as Administrator.
    goto :fail
  )
  echo.
  echo Ubuntu was requested. Windows may require a reboot or one
  echo initialization launch before it can be used.
  echo Re-run this BAT after that step.
  echo.
  pause
  exit /b 10
)
echo   WSL Ubuntu found and runnable.
echo.

echo [2/6] Installing/updating open-source build dependencies...
wsl.exe -d Ubuntu -u root -- bash -lc "export DEBIAN_FRONTEND=noninteractive; apt-get update && apt-get install -y clang binutils git cmake make ninja-build python3 python3-yaml bash bzip2 gzip grep findutils sed tar unzip zip gawk zlib1g-dev build-essential ca-certificates"
if errorlevel 1 (
  echo ERROR: Could not install WSL build dependencies.
  goto :fail
)
echo   Dependencies ready.
echo.

echo [3/6] Resolving project path inside WSL...
set "WSL_PROJECT="
for /f "usebackq delims=" %%P in (`wsl.exe -d Ubuntu -u root -- wslpath -a "%CD%"`) do set "WSL_PROJECT=%%P"
if not defined WSL_PROJECT (
  echo ERROR: Could not translate the project path into WSL.
  goto :fail
)
echo   %WSL_PROJECT%
echo.

echo [4/6] Building/reusing OpenXeChain...
wsl.exe -d Ubuntu -u root -- bash "%WSL_PROJECT%/tools/openxechain/build-seriesdash.sh" "%WSL_PROJECT%"
if errorlevel 1 (
  echo.
  echo ERROR: OpenXeChain compile/build step failed.
  echo Check:
  echo   tools\openxechain\openxechain-build.log
  goto :fail
)
echo.

echo [5/6] Checking Xbox PE output...
if not exist "build-open\SeriesDash360.exe" (
  echo ERROR: OpenXeChain did not produce:
  echo   build-open\SeriesDash360.exe
  goto :fail
)
echo   Xbox PE created.
echo.

echo [6/6] Checking final XEX2 output...
if not exist "build-xbox360-open\default.xex" (
  echo ERROR: SeriesDashXEX did not produce:
  echo   build-xbox360-open\default.xex
  goto :fail
)

echo.
echo ============================================================
echo  SUCCESS - SDK-FREE XEX CREATED
echo ============================================================
echo.
echo Xbox PE:
echo   %CD%\build-open\SeriesDash360.exe
echo.
echo Xbox XEX2:
echo   %CD%\build-xbox360-open\default.xex
echo.
echo IMPORTANT:
echo The current OpenXeChain target is the SDK-free compatibility
echo runtime. The full XDK runtime and the open runtime are being kept
echo separate until every native API has an open replacement.
echo.
pause
exit /b 0

:pack_existing
echo Pack-only mode:
echo   %~1
echo.
where python >nul 2>nul
if not errorlevel 1 (
  set "PY=python"
  goto :do_pack
)
where py >nul 2>nul
if not errorlevel 1 (
  set "PY=py -3"
  goto :do_pack
)
echo ERROR: Python 3 was not found.
goto :fail

:do_pack
if not exist "tools\SeriesDashXEX\seriesdashxex.py" (
  echo ERROR: SeriesDashXEX is missing.
  goto :fail
)
if not exist "build-xbox360-open" mkdir "build-xbox360-open"
%PY% "tools\SeriesDashXEX\seriesdashxex.py" pack "%~1" "build-xbox360-open\default.xex" --kernel-build 17559
if errorlevel 1 goto :fail
%PY% "tools\SeriesDashXEX\seriesdashxex.py" verify "build-xbox360-open\default.xex"
if errorlevel 1 goto :fail
echo SUCCESS: build-xbox360-open\default.xex
pause
exit /b 0

:fail
echo.
echo ============================================================
echo  BUILD FAILED
echo ============================================================
echo.
echo Read the exact error above.
echo.
pause
exit /b 1
