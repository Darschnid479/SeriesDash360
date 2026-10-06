@echo off
setlocal EnableExtensions EnableDelayedExpansion
title SeriesDash360 - Native Windows SDK-Free Builder
cd /d "%~dp0"

echo ============================================================
echo  SeriesDash360 1.0 - NATIVE WINDOWS SDK-FREE BUILD
echo ============================================================
echo.
echo NO WSL. NO LINUX. NO DOCKER. NO MICROSOFT XDK.
echo.
echo This bootstrap prepares a native Windows build environment for
echo the open Xbox 360 toolchain and SeriesDashXEX.
echo.

where winget >nul 2>nul || (
  echo ERROR: winget is required for automatic prerequisite setup.
  echo Install/update App Installer from Microsoft Store and retry.
  goto :fail
)

call :ensure Git.Git git
if errorlevel 1 goto :fail
call :ensure Kitware.CMake cmake
if errorlevel 1 goto :fail
call :ensure Ninja-build.Ninja ninja
if errorlevel 1 goto :fail
call :ensure Python.Python.3.13 python
if errorlevel 1 goto :fail
call :ensure LLVM.LLVM clang
if errorlevel 1 goto :fail

echo.
echo [1/5] Preparing native Windows OpenXeChain sources...
if not exist ".openxechain-win" mkdir ".openxechain-win"
if not exist ".openxechain-win\llvm\.git" (
  git clone --depth 1 https://github.com/OpenXeChain/llvm.git ".openxechain-win\llvm"
  if errorlevel 1 goto :fail
) else (
  git -C ".openxechain-win\llvm" fetch --depth 1 origin main
  git -C ".openxechain-win\llvm" reset --hard origin/main
)

echo.
echo [2/5] Configuring Xbox-patched LLVM for a Windows host...
if not exist ".openxechain-win\llvm-build" mkdir ".openxechain-win\llvm-build"
cmake -S ".openxechain-win\llvm\llvm" -B ".openxechain-win\llvm-build" -G Ninja ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_C_COMPILER=clang ^
  -DCMAKE_CXX_COMPILER=clang++ ^
  -DCMAKE_INSTALL_PREFIX="%CD%\.openxechain-win\toolchain" ^
  -DLLVM_ENABLE_PROJECTS="lld;clang" ^
  -DLLVM_TARGETS_TO_BUILD=PowerPC ^
  -DLLVM_DEFAULT_TARGET_TRIPLE=ppc32-xbox360 ^
  -DLLVM_INSTALL_TOOLCHAIN_ONLY=ON
if errorlevel 1 goto :fail

echo.
echo [3/5] Building native Windows OpenXeChain clang/lld...
cmake --build ".openxechain-win\llvm-build" --target install
if errorlevel 1 goto :fail

set "OXCLANG=%CD%\.openxechain-win\toolchain\bin\clang.exe"
if not exist "%OXCLANG%" (
  echo ERROR: Xbox-patched clang.exe was not produced.
  goto :fail
)

echo.
echo [4/5] Checking SDK-free runtime support...
echo.
echo The compiler is now native Windows and Xbox-target aware.
echo SeriesDash360 still needs the OpenXeChain newlib/xecorelib runtime
echo installed for this Windows-hosted compiler before the dashboard PE
echo can be linked without Microsoft libraries.
echo.
echo This BAT intentionally stops here instead of pretending that stock
echo Windows LLVM can link a working Xbox dashboard.
echo.
echo Native compiler:
echo   %OXCLANG%
echo.
echo [5/5] SeriesDashXEX remains ready for the resulting Xbox PE.
echo.
echo ============================================================
echo  WINDOWS TOOLCHAIN STAGE COMPLETE
echo ============================================================
echo.
echo No WSL/Linux/XDK was used.
echo.
echo Remaining repo work: native-Windows build/install rules for
echo OpenXeChain compiler-rt + newlib + xecorelib, then compile the
echo platform/openxechain runtime and run SeriesDashXEX automatically.
echo.
pause
exit /b 0

:ensure
set "PKG=%~1"
set "CMD=%~2"
where %CMD% >nul 2>nul
if not errorlevel 1 (
  echo Found %CMD%.
  exit /b 0
)
echo Installing %PKG%...
winget install --id %PKG% -e --accept-package-agreements --accept-source-agreements
if errorlevel 1 exit /b 1
call :refresh_path
where %CMD% >nul 2>nul
if errorlevel 1 (
  echo ERROR: %CMD% is still unavailable. Open a new Command Prompt and retry.
  exit /b 1
)
exit /b 0

:refresh_path
for /f "tokens=2,*" %%A in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v Path 2^>nul ^| findstr /i "Path"') do set "MP=%%B"
for /f "tokens=2,*" %%A in ('reg query "HKCU\Environment" /v Path 2^>nul ^| findstr /i "Path"') do set "UP=%%B"
set "PATH=%MP%;%UP%;%PATH%"
exit /b 0

:fail
echo.
echo ============================================================
echo  BUILD SETUP FAILED
echo ============================================================
echo Read the exact error above.
echo.
pause
exit /b 1
