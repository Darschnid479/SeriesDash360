@echo off
setlocal EnableExtensions EnableDelayedExpansion
title SeriesDash360 - ZERO ADMIN SDK-FREE BUILD
cd /d "%~dp0"

set "ROOT=%CD%"
set "TOOLS=%ROOT%\.tools"
set "MSYS=%TOOLS%\msys64"
set "BASH=%MSYS%\usr\bin\bash.exe"
set "SFX=%TOOLS%\msys2-base.sfx.exe"

echo ============================================================
echo  SeriesDash360 1.0 - ZERO ADMIN SDK-FREE BUILD
echo ============================================================
echo.
echo This build is designed to run WITHOUT administrator rights.
echo.
echo It uses only files inside this SeriesDash360 folder:
echo   .tools\msys64
echo   .openxechain
echo   build-open
echo.
echo NO WSL
echo NO Docker
echo NO Microsoft XDK
echo NO Program Files installation
echo NO registry changes
echo NO permanent PATH changes
echo.
echo NOTE: The first OpenXeChain build is large and CPU/disk heavy.
echo.

if not exist "%TOOLS%" mkdir "%TOOLS%" >nul 2>nul
if errorlevel 1 (
  echo ERROR: Could not create:
  echo   %TOOLS%
  goto :fail
)

echo [1/8] Checking PowerShell...
where powershell.exe >nul 2>nul
if errorlevel 1 (
  echo ERROR: Windows PowerShell was not found.
  goto :fail
)
echo   PowerShell OK.
echo.

if exist "%BASH%" goto :msys_ready

echo [2/8] Downloading portable MSYS2...
echo   This is extracted locally and does not require admin.
echo.

powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -Command ^
  "$ErrorActionPreference='Stop';" ^
  "$h=@{'User-Agent'='SeriesDash360-ZeroAdmin'};" ^
  "$r=Invoke-RestMethod -Headers $h -Uri 'https://api.github.com/repos/msys2/msys2-installer/releases/latest';" ^
  "$a=$r.assets | Where-Object { $_.name -match '^msys2-base-x86_64-.*\.sfx\.exe$' } | Select-Object -First 1;" ^
  "if(-not $a){throw 'Portable MSYS2 SFX asset not found'};" ^
  "Write-Host ('  Downloading ' + $a.name);" ^
  "Invoke-WebRequest -Headers $h -Uri $a.browser_download_url -OutFile '%SFX%';"
if errorlevel 1 (
  echo ERROR: Could not download portable MSYS2.
  goto :fail
)

echo.
echo [3/8] Extracting portable MSYS2...
"%SFX%" -y -o"%TOOLS%"
if errorlevel 1 (
  echo ERROR: MSYS2 extraction failed.
  goto :fail
)

if not exist "%BASH%" (
  echo ERROR: Portable MSYS2 did not produce:
  echo   %BASH%
  goto :fail
)

del /q "%SFX%" >nul 2>nul

:msys_ready
echo [2/8] Portable MSYS2 ready:
echo   %MSYS%
echo.

echo [4/8] Initializing local MSYS2 environment...
set "CHERE_INVOKING=1"
set "MSYS2_PATH_TYPE=inherit"
"%BASH%" --login -lc "true"
if errorlevel 1 (
  echo ERROR: MSYS2 initialization failed.
  goto :fail
)

echo.
echo [5/8] Updating local package database...
"%BASH%" --login -lc "pacman -Sy --noconfirm"
if errorlevel 1 (
  echo ERROR: pacman database update failed.
  goto :fail
)

echo.
echo [6/8] Installing build tools LOCALLY inside .tools\msys64...
"%BASH%" --login -lc "pacman -S --needed --noconfirm base-devel git clang cmake ninja make python python-pip python-yaml binutils bzip2 gzip grep findutils sed tar unzip zip gawk zlib-devel"
if errorlevel 1 (
  echo ERROR: Local MSYS2 package installation failed.
  goto :fail
)

echo.
echo [7/8] Building OpenXeChain + SeriesDash360...
echo.
echo This step builds the Xbox-patched LLVM toolchain locally.
echo Nothing is installed system-wide.
echo.

for /f "usebackq delims=" %%P in (`"%MSYS%\usr\bin\cygpath.exe" -u "%ROOT%"`) do set "UNIXROOT=%%P"
if not defined UNIXROOT (
  echo ERROR: Could not convert the project path for portable MSYS2.
  goto :fail
)

"%BASH%" --login -lc "cd '%UNIXROOT%' && chmod +x BOOTSTRAP_OPENXECHAIN_NO_ADMIN.sh platform/openxechain/build.sh && ./BOOTSTRAP_OPENXECHAIN_NO_ADMIN.sh"
if errorlevel 1 (
  echo.
  echo ERROR: OpenXeChain/SeriesDash360 build failed.
  echo Check:
  echo   .openxechain\src\buildscript\build.log
  goto :fail
)

echo.
echo [8/8] Checking output...
if not exist "build-open\default.xex" (
  echo ERROR: Build completed without producing:
  echo   %ROOT%\build-open\default.xex
  goto :fail
)

echo.
echo ============================================================
echo  SUCCESS - ZERO ADMIN BUILD
echo ============================================================
echo.
echo XEX created:
echo   %ROOT%\build-open\default.xex
echo.
echo Local toolchain:
echo   %ROOT%\.openxechain\sysroot
echo.
echo Portable Windows build environment:
echo   %ROOT%\.tools\msys64
echo.
echo No administrator rights, WSL, Docker or Microsoft XDK used.
echo.
pause
exit /b 0

:fail
echo.
echo ============================================================
echo  ZERO ADMIN BUILD FAILED
echo ============================================================
echo.
echo Read the first ERROR above.
echo No system-wide changes were made by this BAT.
echo.
pause
exit /b 1
