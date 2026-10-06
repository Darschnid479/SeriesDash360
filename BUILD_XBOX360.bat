@echo off
setlocal EnableExtensions EnableDelayedExpansion
title SeriesDash360 - Xbox 360 XEX Builder
cd /d "%~dp0"

echo ============================================================
echo  SeriesDash360 1.0 - Xbox 360 XEX Builder
echo ============================================================
echo.
echo Project folder:
echo   %CD%
echo.

rem ------------------------------------------------------------
rem Helper: refresh PATH after winget installs
rem ------------------------------------------------------------
set "MACHINE_PATH="
set "USER_PATH="
for /f "tokens=2,*" %%A in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v Path 2^>nul ^| findstr /i "Path"') do set "MACHINE_PATH=%%B"
for /f "tokens=2,*" %%A in ('reg query "HKCU\Environment" /v Path 2^>nul ^| findstr /i "Path"') do set "USER_PATH=%%B"
if defined MACHINE_PATH set "PATH=%MACHINE_PATH%;%USER_PATH%;%PATH%"

echo [1/8] Checking required Windows package manager...
where winget >nul 2>nul
if errorlevel 1 (
  echo   winget not found.
  echo   Automatic installation of CMake/Ninja is unavailable.
  echo   The builder will continue and tell you exactly what is missing.
) else (
  echo   winget found.
)
echo.

echo [2/8] Checking CMake...
where cmake >nul 2>nul
if errorlevel 1 (
  echo   CMake is missing.
  where winget >nul 2>nul
  if errorlevel 1 (
    echo ERROR: Cannot install CMake automatically because winget is missing.
    echo Install CMake manually from Kitware and add it to PATH.
    goto :fail
  )
  echo   Installing CMake automatically...
  winget install --id Kitware.CMake -e --accept-package-agreements --accept-source-agreements
  if errorlevel 1 (
    echo ERROR: Automatic CMake installation failed.
    goto :fail
  )
  call :refresh_path
)
where cmake >nul 2>nul || (
  echo ERROR: cmake.exe is still not available after installation.
  echo Close this window, open a new Command Prompt and run the BAT again.
  goto :fail
)
for /f "delims=" %%V in ('cmake --version 2^>nul ^| findstr /b /c:"cmake version"') do echo   %%V
echo.

echo [3/8] Checking Ninja...
where ninja >nul 2>nul
if errorlevel 1 (
  echo   Ninja is missing.
  where winget >nul 2>nul
  if errorlevel 1 (
    echo ERROR: Cannot install Ninja automatically because winget is missing.
    echo Install Ninja manually and add ninja.exe to PATH.
    goto :fail
  )
  echo   Installing Ninja automatically...
  winget install --id Ninja-build.Ninja -e --accept-package-agreements --accept-source-agreements
  if errorlevel 1 (
    echo ERROR: Automatic Ninja installation failed.
    goto :fail
  )
  call :refresh_path
)
where ninja >nul 2>nul || (
  echo ERROR: ninja.exe is still not available after installation.
  echo Close this window, open a new Command Prompt and run the BAT again.
  goto :fail
)
for /f "delims=" %%V in ('ninja --version 2^>nul') do echo   Ninja %%V
echo.

echo [4/8] Looking for Xbox 360 XDK...

rem Use an already configured environment variable first.
if defined XEDK if exist "%XEDK%\bin\win32\imagexex.exe" goto :have_xedk

rem Try a persisted user environment variable.
for /f "tokens=2,*" %%A in ('reg query "HKCU\Environment" /v XEDK 2^>nul ^| findstr /i "XEDK"') do set "XEDK=%%B"
if defined XEDK if exist "%XEDK%\bin\win32\imagexex.exe" goto :have_xedk

rem Try common install folders.
for %%D in (
  "C:\Program Files (x86)\Microsoft Xbox 360 SDK"
  "C:\Program Files\Microsoft Xbox 360 SDK"
  "C:\Program Files (x86)\Microsoft Xbox 360 SDK\"
  "C:\Xbox360SDK"
  "C:\Xbox 360 SDK"
  "C:\XDK"
  "D:\Xbox360SDK"
  "D:\Xbox 360 SDK"
  "D:\XDK"
) do (
  if exist "%%~D\bin\win32\imagexex.exe" (
    set "XEDK=%%~D"
    goto :save_xedk
  )
)

echo.
echo Xbox 360 XDK was not found automatically.
echo.
echo IMPORTANT:
echo   CMake and Ninja can be downloaded automatically.
echo   The Xbox 360 XDK cannot be legally bundled or downloaded by
echo   this project. You must already have an authorized/local copy.
echo.
echo Required files inside the XDK folder:
echo   bin\win32\cl.exe
echo   bin\win32\link.exe
echo   bin\win32\imagexex.exe
echo.
echo If you already have the XDK somewhere on this PC, paste its
echo ROOT folder below. Example:
echo   C:\Program Files (x86)\Microsoft Xbox 360 SDK
echo.
set /p "XEDK_INPUT=XDK folder (leave blank to stop): "
if "%XEDK_INPUT%"=="" goto :missing_xdk
set "XEDK=%XEDK_INPUT%"
set "XEDK=%XEDK:"=%"

if not exist "%XEDK%\bin\win32\imagexex.exe" (
  echo.
  echo ERROR: imagexex.exe was not found here:
  echo   %XEDK%\bin\win32\imagexex.exe
  echo.
  echo That does not appear to be the XDK root folder.
  goto :fail
)

:save_xedk
echo.
echo   Valid XDK found:
echo   %XEDK%
echo.
echo   Saving XEDK for future builds...
setx XEDK "%XEDK%" >nul
if errorlevel 1 (
  echo   WARNING: Could not persist XEDK with setx.
  echo   This build can still continue.
) else (
  echo   Saved user environment variable XEDK.
)

:have_xedk
echo.
echo   XEDK=%XEDK%

if not exist "%XEDK%\bin\win32\cl.exe" (
  echo ERROR: cl.exe not found:
  echo   %XEDK%\bin\win32\cl.exe
  goto :fail
)
if not exist "%XEDK%\bin\win32\link.exe" (
  echo ERROR: link.exe not found:
  echo   %XEDK%\bin\win32\link.exe
  goto :fail
)
if not exist "%XEDK%\bin\win32\imagexex.exe" (
  echo ERROR: imagexex.exe not found:
  echo   %XEDK%\bin\win32\imagexex.exe
  goto :fail
)
echo   XDK compiler/linker/XEX tools found.
echo.

echo [5/8] Checking project files...
if not exist "CMakePresets.json" (
  echo ERROR: CMakePresets.json is missing.
  goto :fail
)
if not exist "platform\xbox360\CMakeLists.txt" (
  echo ERROR: platform\xbox360\CMakeLists.txt is missing.
  goto :fail
)
if not exist "platform\xbox360\NativeMain.cpp" (
  echo ERROR: platform\xbox360\NativeMain.cpp is missing.
  goto :fail
)
if not exist "cmake\XdkXenon.toolchain.cmake" (
  echo ERROR: cmake\XdkXenon.toolchain.cmake is missing.
  goto :fail
)
if not exist "cmake\XdkXex.cmake" (
  echo ERROR: cmake\XdkXex.cmake is missing.
  goto :fail
)
echo   Project files OK.
echo.

echo [6/8] Configuring Xbox 360 build...
cmake --preset xbox360-xdk
if errorlevel 1 (
  echo.
  echo ERROR: CMake configure failed.
  echo Check the messages above for the exact cause.
  goto :fail
)
echo.

echo [7/8] Building SeriesDash360.xex...
cmake --build --preset xbox360-xdk --verbose
if errorlevel 1 (
  echo.
  echo ERROR: Xbox 360 build failed.
  echo Check the compiler/linker error above.
  goto :fail
)
echo.

echo [8/8] Verifying output...
if not exist "build-xbox360\SeriesDash360.xex" (
  echo ERROR: Build completed but SeriesDash360.xex was not produced.
  echo Expected:
  echo   %CD%\build-xbox360\SeriesDash360.xex
  goto :fail
)

copy /y "build-xbox360\SeriesDash360.xex" "build-xbox360\default.xex" >nul
if errorlevel 1 (
  echo ERROR: Could not create default.xex.
  goto :fail
)

echo.
echo ============================================================
echo  SUCCESS
echo ============================================================
echo.
echo Xbox 360 XEX created:
echo   %CD%\build-xbox360\default.xex
echo.
echo Original output:
echo   %CD%\build-xbox360\SeriesDash360.xex
echo.
echo XEDK saved as:
echo   %XEDK%
echo.
pause
exit /b 0

:missing_xdk
echo.
echo ============================================================
echo  XDK REQUIRED
echo ============================================================
echo.
echo CMake/Ninja can be installed automatically, but the Xbox 360
echo XDK itself is proprietary and is not downloaded or redistributed
echo by SeriesDash360.
echo.
echo Once you have a legitimate/local XDK installation, run this BAT
echo again and paste the XDK root folder when prompted.
echo.
pause
exit /b 2

:refresh_path
set "MACHINE_PATH="
set "USER_PATH="
for /f "tokens=2,*" %%A in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v Path 2^>nul ^| findstr /i "Path"') do set "MACHINE_PATH=%%B"
for /f "tokens=2,*" %%A in ('reg query "HKCU\Environment" /v Path 2^>nul ^| findstr /i "Path"') do set "USER_PATH=%%B"
if defined MACHINE_PATH set "PATH=%MACHINE_PATH%;%USER_PATH%;%PATH%"
exit /b 0

:fail
echo.
echo ============================================================
echo  BUILD FAILED
echo ============================================================
echo.
echo The exact error should be visible above.
echo Copy that error text back to me and I can fix the next issue.
echo.
pause
exit /b 1
