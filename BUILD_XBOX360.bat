@echo off
setlocal EnableExtensions
title SeriesDash360 - Xbox 360 XEX Builder
cd /d "%~dp0"

echo ============================================================
echo  SeriesDash360 1.0 - Xbox 360 XEX Builder
echo ============================================================
echo.
echo Project folder:
echo   %CD%
echo.

echo [1/7] Looking for Xbox 360 XDK...
if not "%XEDK%"=="" goto :have_xedk

for %%D in (
  "C:\Program Files (x86)\Microsoft Xbox 360 SDK"
  "C:\Program Files\Microsoft Xbox 360 SDK"
  "C:\Xbox360SDK"
  "C:\XDK"
) do (
  if exist "%%~D\bin\win32\imagexex.exe" (
    set "XEDK=%%~D"
    goto :have_xedk
  )
)

echo.
echo ERROR: Xbox 360 XDK was not found.
echo.
echo The build needs an installed Xbox 360 XDK containing:
echo   bin\win32\cl.exe
echo   bin\win32\link.exe
echo   bin\win32\imagexex.exe
echo.
echo If your XDK is installed somewhere else, run:
echo   set "XEDK=C:\path\to\your\Xbox 360 SDK"
echo   BUILD_XBOX360.bat
echo.
goto :fail

:have_xedk
echo   XEDK=%XEDK%
if not exist "%XEDK%\bin\win32\cl.exe" (
  echo ERROR: cl.exe not found in "%XEDK%\bin\win32"
  goto :fail
)
if not exist "%XEDK%\bin\win32\link.exe" (
  echo ERROR: link.exe not found in "%XEDK%\bin\win32"
  goto :fail
)
if not exist "%XEDK%\bin\win32\imagexex.exe" (
  echo ERROR: imagexex.exe not found in "%XEDK%\bin\win32"
  goto :fail
)
echo   XDK tools found.
echo.

echo [2/7] Checking CMake...
where cmake >nul 2>nul
if errorlevel 1 (
  echo ERROR: cmake.exe was not found in PATH.
  echo Install CMake and enable "Add CMake to PATH" during setup.
  goto :fail
)
for /f "delims=" %%V in ('cmake --version 2^>nul ^| findstr /b /c:"cmake version"') do echo   %%V
echo.

echo [3/7] Checking Ninja...
where ninja >nul 2>nul
if errorlevel 1 (
  echo ERROR: ninja.exe was not found in PATH.
  echo Install Ninja or place ninja.exe somewhere in PATH.
  goto :fail
)
for /f "delims=" %%V in ('ninja --version 2^>nul') do echo   Ninja %%V
echo.

echo [4/7] Checking project files...
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
echo   Project files OK.
echo.

echo [5/7] Configuring Xbox 360 build...
cmake --preset xbox360-xdk
if errorlevel 1 (
  echo.
  echo ERROR: CMake configure failed.
  echo Check the messages above for the exact cause.
  goto :fail
)
echo.

echo [6/7] Building SeriesDash360.xex...
cmake --build --preset xbox360-xdk --verbose
if errorlevel 1 (
  echo.
  echo ERROR: Xbox 360 build failed.
  echo Check the compiler/linker error above.
  goto :fail
)
echo.

echo [7/7] Verifying output...
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
echo Copy the SeriesDash360 folder and default.xex to the
echo location used by your compatible Xbox 360 homebrew host.
echo.
pause
exit /b 0

:fail
echo.
echo ============================================================
echo  BUILD FAILED
echo ============================================================
echo.
echo Nothing was hidden. The exact error should be visible above.
echo If you send me a screenshot or copy the error text, I can
echo fix the next problem in the build.
echo.
pause
exit /b 1
