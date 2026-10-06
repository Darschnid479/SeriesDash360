@echo off
setlocal EnableExtensions DisableDelayedExpansion
cd /d "%~dp0"

set "ROOT=%CD%"
set "LOG=%ROOT%\ZERO_ADMIN_BUILD.log"
set "TOOLS=%ROOT%\.tools"
set "MSYS=%TOOLS%\msys64"
set "BASH=%MSYS%\usr\bin\bash.exe"
set "CYGPATH=%MSYS%\usr\bin\cygpath.exe"
set "SFX=%TOOLS%\msys2-base.sfx.exe"

> "%LOG%" echo SeriesDash360 Zero Admin Build Log
>>"%LOG%" echo Started: %DATE% %TIME%
>>"%LOG%" echo Project: %ROOT%
>>"%LOG%" echo.

call :banner
call :log "Worker started."

if not exist "%TOOLS%" (
  mkdir "%TOOLS%" >>"%LOG%" 2>&1
  if errorlevel 1 call :die "Could not create .tools folder."
)

call :log "[1/8] Checking Windows tools..."
where cmd.exe >>"%LOG%" 2>&1 || call :die "cmd.exe was not found."
where powershell.exe >>"%LOG%" 2>&1
if errorlevel 1 (
  where curl.exe >>"%LOG%" 2>&1
  if errorlevel 1 call :die "Neither PowerShell nor curl.exe is available."
)
echo [1/8] Windows tools OK.

if exist "%BASH%" goto :msys_ready

echo.
echo [2/8] Downloading portable MSYS2...
call :log "[2/8] Downloading portable MSYS2."

where curl.exe >nul 2>nul
if not errorlevel 1 (
  curl.exe -L --fail --retry 3 --connect-timeout 30 ^
    "https://github.com/msys2/msys2-installer/releases/download/2026-09-27/msys2-base-x86_64-20260927.sfx.exe" ^
    -o "%SFX%" >>"%LOG%" 2>&1
) else (
  powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -Command ^
    "$ProgressPreference='SilentlyContinue'; $ErrorActionPreference='Stop'; Invoke-WebRequest -Uri 'https://github.com/msys2/msys2-installer/releases/download/2026-09-27/msys2-base-x86_64-20260927.sfx.exe' -OutFile '%SFX%'" ^
    >>"%LOG%" 2>&1
)
if errorlevel 1 call :die "Portable MSYS2 download failed. See ZERO_ADMIN_BUILD.log."

if not exist "%SFX%" call :die "MSYS2 download reported success but the file is missing."

for %%A in ("%SFX%") do set "SFXSIZE=%%~zA"
if "%SFXSIZE%"=="0" call :die "Downloaded MSYS2 file is empty."

echo [3/8] Extracting portable MSYS2...
call :log "[3/8] Extracting MSYS2 SFX."
"%SFX%" -y -o"%TOOLS%" >>"%LOG%" 2>&1
if errorlevel 1 call :die "MSYS2 extraction failed."

if not exist "%BASH%" (
  call :log "Expected bash: %BASH%"
  dir /s /b "%TOOLS%\bash.exe" >>"%LOG%" 2>&1
  call :die "MSYS2 extracted, but bash.exe was not found in the expected location."
)

del /q "%SFX%" >>"%LOG%" 2>&1

:msys_ready
echo.
echo [4/8] Initializing portable MSYS2...
call :log "[4/8] Initializing portable MSYS2."

set "CHERE_INVOKING=1"
set "MSYS2_PATH_TYPE=minimal"
"%BASH%" --login -lc "echo MSYS2_READY" >>"%LOG%" 2>&1
if errorlevel 1 call :die "MSYS2 bash failed to start."

echo [5/8] Updating local MSYS2 package database...
call :log "[5/8] pacman database update."
"%BASH%" --login -lc "pacman -Sy --noconfirm" >>"%LOG%" 2>&1
if errorlevel 1 call :die "pacman database update failed."

echo [6/8] Installing local build packages...
call :log "[6/8] Installing local packages."
"%BASH%" --login -lc "pacman -S --needed --noconfirm base-devel git clang cmake ninja make python python-pip python-yaml binutils bzip2 gzip grep findutils sed tar unzip zip gawk zlib-devel" >>"%LOG%" 2>&1
if errorlevel 1 call :die "Local MSYS2 package installation failed."

if not exist "%CYGPATH%" call :die "cygpath.exe is missing after MSYS2 setup."

for /f "usebackq delims=" %%P in (`"%CYGPATH%" -u "%ROOT%"`) do set "UNIXROOT=%%P"
if not defined UNIXROOT call :die "Could not translate the project path for MSYS2."

echo [7/8] Building OpenXeChain and SeriesDash360...
echo         Detailed output is being written to ZERO_ADMIN_BUILD.log
call :log "[7/8] OpenXeChain build. UNIXROOT=%UNIXROOT%"

"%BASH%" --login -lc "cd '%UNIXROOT%' && chmod +x BOOTSTRAP_OPENXECHAIN_NO_ADMIN.sh platform/openxechain/build.sh && ./BOOTSTRAP_OPENXECHAIN_NO_ADMIN.sh" >>"%LOG%" 2>&1
if errorlevel 1 call :die "OpenXeChain/SeriesDash360 build failed."

echo [8/8] Checking default.xex...
call :log "[8/8] Checking output."
if not exist "%ROOT%\build-open\default.xex" call :die "Build finished without build-open\default.xex."

echo.
echo ============================================================
echo  SUCCESS
echo ============================================================
echo.
echo Created:
echo   %ROOT%\build-open\default.xex
echo.
echo Full log:
echo   %LOG%
echo.
call :log "SUCCESS: build-open/default.xex created."
goto :end

:banner
echo ============================================================
echo  SeriesDash360 1.0 - ZERO ADMIN SDK-FREE BUILD
echo ============================================================
echo.
echo NO ADMIN / NO WSL / NO DOCKER / NO XDK
echo.
echo Full log:
echo   %LOG%
echo.
exit /b 0

:log
echo %~1
>>"%LOG%" echo [%DATE% %TIME%] %~1
exit /b 0

:die
echo.
echo ============================================================
echo  ERROR
echo ============================================================
echo.
echo %~1
echo.
echo Log file:
echo   %LOG%
echo.
>>"%LOG%" echo.
>>"%LOG%" echo ERROR: %~1
>>"%LOG%" echo Failed: %DATE% %TIME%
goto :end

:end
echo.
echo Press any key to close this worker.
pause >nul
exit /b
