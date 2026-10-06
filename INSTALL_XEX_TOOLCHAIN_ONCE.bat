@echo off
setlocal EnableExtensions DisableDelayedExpansion
cd /d "%~dp0"
title SeriesDash360 - One-Time XEX Toolchain Setup

set "ROOT=%CD%"
set "CACHE=.seriesdash360/openxechain"

echo ============================================================
echo  SeriesDash360 - ONE-TIME XEX TOOLCHAIN SETUP
echo ============================================================
echo.
echo This is the SLOW step, but it only needs to succeed once.
echo.
echo Permanent WSL cache:
echo   ~/%CACHE%
echo.
echo After this finishes, use:
echo   BUILD_DEFAULT_XEX_FAST.bat
echo.
echo ============================================================
echo.

where wsl.exe >nul 2>nul
if errorlevel 1 goto NO_WSL

echo [1/5] Checking Ubuntu...
wsl.exe -d Ubuntu -- bash -lc "echo WSL_OK"
if errorlevel 1 goto NO_UBUNTU
echo [OK]
echo.

echo [2/5] Converting project path...
set "PATHFILE=%TEMP%\sd360_xex_setup_%RANDOM%.txt"
wsl.exe -d Ubuntu -- wslpath -a "%ROOT%" > "%PATHFILE%" 2>nul
if errorlevel 1 goto PATH_FAIL
set "WSLROOT="
set /p WSLROOT=<"%PATHFILE%"
del /q "%PATHFILE%" >nul 2>nul
if not defined WSLROOT goto PATH_FAIL
echo [OK] %WSLROOT%
echo.

echo [3/5] Preparing permanent WSL toolchain cache...
wsl.exe -d Ubuntu -- bash -lc "mkdir -p ~/%CACHE%"
if errorlevel 1 goto PREP_FAIL
echo [OK]
echo.

echo [4/5] Building/installing OpenXeChain ONE TIME...
echo.
echo IMPORTANT:
echo   - Ubuntu may ask for your Linux sudo password.
echo   - Password characters are NOT shown while typing.
echo   - This LLVM build can take a long time the first time.
echo   - Do NOT delete ~/%CACHE% afterwards.
echo.

wsl.exe -d Ubuntu -- bash -lc "set -o pipefail; cd '%WSLROOT%' && chmod +x BOOTSTRAP_OPENXECHAIN.sh platform/openxechain/build.sh && export OPENXECHAIN_WORK=$HOME/%CACHE% && export OPENXECHAIN_PREFIX=$HOME/%CACHE%/sysroot && export OPENXECHAIN_JOBS=$(nproc) && ./BOOTSTRAP_OPENXECHAIN.sh 2>&1 | tee $HOME/%CACHE%/setup.log"
if errorlevel 1 goto SETUP_FAIL

echo.
echo [5/5] Verifying cached compiler...
wsl.exe -d Ubuntu -- bash -lc "test -x $HOME/%CACHE%/sysroot/bin/clang && $HOME/%CACHE%/sysroot/bin/clang --version | head -n 1"
if errorlevel 1 goto VERIFY_FAIL

echo.
echo ============================================================
echo  TOOLCHAIN READY
echo ============================================================
echo.
echo Permanent cache:
echo   ~/%CACHE%
echo.
echo From now on run:
echo.
echo   BUILD_DEFAULT_XEX_FAST.bat
echo.
echo The fast BAT will NOT rebuild LLVM.
echo.
pause
exit /b 0

:NO_WSL
echo ERROR: WSL is not installed.
pause
exit /b 1

:NO_UBUNTU
echo ERROR: Ubuntu could not start.
echo Try: wsl -d Ubuntu
pause
exit /b 1

:PATH_FAIL
echo ERROR: Could not translate project path to WSL.
pause
exit /b 1

:PREP_FAIL
echo ERROR: Could not create the permanent WSL cache.
pause
exit /b 1

:SETUP_FAIL
echo.
echo ============================================================
echo TOOLCHAIN SETUP FAILED
echo ============================================================
echo.
echo Show the last 120 lines with:
echo   wsl -d Ubuntu -- tail -n 120 ~/%CACHE%/setup.log
echo.
pause
exit /b 1

:VERIFY_FAIL
echo ERROR: Setup ended but cached clang was not found.
echo Expected:
echo   ~/%CACHE%/sysroot/bin/clang
pause
exit /b 1
