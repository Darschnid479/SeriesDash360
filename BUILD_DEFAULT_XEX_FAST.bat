@echo off
setlocal EnableExtensions DisableDelayedExpansion
cd /d "%~dp0"
title SeriesDash360 - FAST default.xex Build

set "ROOT=%CD%"
set "CACHE=.seriesdash360/openxechain"
set "SRC=.seriesdash360/SeriesDash360-src"
set "OUT=%ROOT%\build-open"

echo ============================================================
echo  SeriesDash360 - FAST default.xex BUILD
echo ============================================================
echo.
echo Reuses permanent WSL OpenXeChain cache.
echo NO LLVM rebuild.
echo NO full-project cache copy.
echo.
echo ============================================================
echo.

where wsl.exe >nul 2>nul
if errorlevel 1 goto NO_WSL

echo [1/7] Checking Ubuntu...
wsl.exe -d Ubuntu -- bash -lc "echo WSL_OK"
if errorlevel 1 goto NO_UBUNTU
echo [OK]
echo.

echo [2/7] Checking cached XEX compiler...
wsl.exe -d Ubuntu -- bash -lc "test -x $HOME/%CACHE%/sysroot/bin/clang"
if errorlevel 1 goto NO_TOOLCHAIN
echo [OK] Cached OpenXeChain found.
echo.

echo [3/7] Converting Windows project path...
set "PATHFILE=%TEMP%\sd360_fast_xex_%RANDOM%.txt"
wsl.exe -d Ubuntu -- wslpath -a "%ROOT%" > "%PATHFILE%" 2>nul
if errorlevel 1 goto PATH_FAIL
set "WSLROOT="
set /p WSLROOT=<"%PATHFILE%"
del /q "%PATHFILE%" >nul 2>nul
if not defined WSLROOT goto PATH_FAIL
echo [OK] %WSLROOT%
echo.

echo [4/7] Fast-syncing source into WSL...
wsl.exe -d Ubuntu -- bash -lc "rm -rf $HOME/%SRC% && mkdir -p $HOME/%SRC% && cd '%WSLROOT%' && tar --exclude='./.git' --exclude='./.openxechain' --exclude='./.openxechain-win' --exclude='./.tools' --exclude='./build' --exclude='./build-*' --exclude='./node_modules' --exclude='./__pycache__' --exclude='./.vs' --exclude='./.vscode' -cf - . | tar -xf - -C $HOME/%SRC%"
if errorlevel 1 goto SYNC_FAIL
echo [OK]
echo.

echo [5/7] Compiling SeriesDash360 PE...
echo.
wsl.exe -d Ubuntu -- bash -lc "set -o pipefail; cd $HOME/%SRC% && chmod +x platform/openxechain/build.sh && OPENXECHAIN_PREFIX=$HOME/%CACHE%/sysroot bash platform/openxechain/build.sh 2>&1 | tee $HOME/%CACHE%/last-build.log"
if errorlevel 1 goto BUILD_FAIL

echo.
echo [6/7] Verifying default.xex...
wsl.exe -d Ubuntu -- bash -lc "test -s $HOME/%SRC%/build-open/default.xex && python3 $HOME/%SRC%/tools/SeriesDashXEX/seriesdashxex.py verify $HOME/%SRC%/build-open/default.xex"
if errorlevel 1 goto VERIFY_FAIL
echo [OK]
echo.

echo [7/7] Copying default.xex back to Windows...
if not exist "%OUT%" mkdir "%OUT%"
wsl.exe -d Ubuntu -- bash -lc "cp $HOME/%SRC%/build-open/default.xex '%WSLROOT%/build-open/default.xex' && cp $HOME/%SRC%/build-open/SeriesDash360.exe '%WSLROOT%/build-open/SeriesDash360.exe'"
if errorlevel 1 goto COPY_FAIL

if not exist "%OUT%\default.xex" goto COPY_FAIL

for %%F in ("%OUT%\default.xex") do set "SIZE=%%~zF"

echo.
echo ============================================================
echo  default.xex CREATED
echo ============================================================
echo.
echo   %OUT%\default.xex
echo.
echo Size:
echo   %SIZE% bytes
echo.
echo Intermediate PE:
echo   %OUT%\SeriesDash360.exe
echo.
echo IMPORTANT:
echo This is an unsigned homebrew XEX for an already-patched
echo Xbox 360 homebrew environment. It is NOT retail-signed.
echo.
echo Current OpenXeChain target is the bootstrap runtime;
echo full XDK-dashboard feature parity still requires porting.
echo.
pause
exit /b 0

:NO_WSL
echo ERROR: WSL is not installed.
pause
exit /b 1

:NO_UBUNTU
echo ERROR: Ubuntu could not start.
pause
exit /b 1

:NO_TOOLCHAIN
echo.
echo ============================================================
echo ONE-TIME TOOLCHAIN IS NOT READY
echo ============================================================
echo.
echo Run this once:
echo   INSTALL_XEX_TOOLCHAIN_ONCE.bat
echo.
pause
exit /b 2

:PATH_FAIL
echo ERROR: Could not translate project path.
pause
exit /b 1

:SYNC_FAIL
echo ERROR: Fast source sync failed.
pause
exit /b 1

:BUILD_FAIL
echo.
echo ============================================================
echo XEX BUILD FAILED
echo ============================================================
echo.
echo Show the last 120 lines:
echo   wsl -d Ubuntu -- tail -n 120 ~/%CACHE%/last-build.log
echo.
pause
exit /b 1

:VERIFY_FAIL
echo ERROR: default.xex was created but structural verification failed.
pause
exit /b 1

:COPY_FAIL
echo ERROR: default.xex could not be copied back to Windows.
pause
exit /b 1
