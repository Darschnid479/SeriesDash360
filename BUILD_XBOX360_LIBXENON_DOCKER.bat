@echo off
setlocal EnableExtensions DisableDelayedExpansion
cd /d "%~dp0"
title SeriesDash360 - LibXenon Docker Build

set "ROOT=%CD%"
set "IMAGE=free60/libxenon:latest"
set "OUT=%ROOT%\build-libxenon"

echo ============================================================
echo  SeriesDash360 - LIBXENON FAST BUILD
echo ============================================================
echo.
echo No OpenXeChain.
echo No LLVM source build.
echo No Xbox 360 XDK.
echo.
echo Output: LibXenon ELF for XeLL Reloaded
echo.

where docker.exe >nul 2>nul
if errorlevel 1 goto NO_DOCKER

echo [1/5] Checking Docker...
docker info >nul 2>nul
if errorlevel 1 goto DOCKER_NOT_RUNNING
echo [OK] Docker is running.
echo.

echo [2/5] Checking prebuilt LibXenon image...
docker image inspect "%IMAGE%" >nul 2>nul
if errorlevel 1 (
    echo Image not found locally. Downloading once...
    docker pull "%IMAGE%"
    if errorlevel 1 goto PULL_FAIL
) else (
    echo [OK] Using cached %IMAGE%
)
echo.

echo [3/5] Building SeriesDash360...
echo.
docker run --rm -v "%ROOT%:/app" -w /app/platform/libxenon "%IMAGE%" bash -lc "make clean && make -j$(nproc)"
if errorlevel 1 goto BUILD_FAIL
echo.

echo [4/5] Checking output...
if not exist "%ROOT%\platform\libxenon\SeriesDash360.elf32" goto NO_OUTPUT
echo [OK] SeriesDash360.elf32 created.
echo.

echo [5/5] Preparing build-libxenon...
if not exist "%OUT%" mkdir "%OUT%"
copy /y "%ROOT%\platform\libxenon\SeriesDash360.elf32" "%OUT%\SeriesDash360.elf32" >nul
if errorlevel 1 goto COPY_FAIL
copy /y "%ROOT%\platform\libxenon\SeriesDash360.elf32" "%OUT%\xenon.elf" >nul
if errorlevel 1 goto COPY_FAIL

for %%F in ("%OUT%\SeriesDash360.elf32") do set "SIZE=%%~zF"

echo.
echo ============================================================
echo  SUCCESS - LIBXENON BUILD COMPLETE
echo ============================================================
echo.
echo Main output:
echo   %OUT%\SeriesDash360.elf32
echo.
echo XeLL USB convenience copy:
echo   %OUT%\xenon.elf
echo.
echo Size:
echo   %SIZE% bytes
echo.
echo IMPORTANT:
echo This is a LibXenon/XeLL bare-metal ELF.
echo It is NOT a default.xex and will not launch as a normal XEX.
echo.
pause
exit /b 0

:NO_DOCKER
echo.
echo ============================================================
echo ERROR - DOCKER NOT INSTALLED
echo ============================================================
echo.
echo Install Docker Desktop, start it, then run this BAT again.
echo.
pause
exit /b 1

:DOCKER_NOT_RUNNING
echo.
echo ============================================================
echo ERROR - DOCKER IS NOT RUNNING
echo ============================================================
echo.
echo Start Docker Desktop and wait until its engine is ready.
echo Then run this BAT again.
echo.
pause
exit /b 1

:PULL_FAIL
echo.
echo ============================================================
echo ERROR - COULD NOT DOWNLOAD LIBXENON IMAGE
echo ============================================================
echo.
echo Test manually:
echo   docker pull %IMAGE%
echo.
pause
exit /b 1

:BUILD_FAIL
echo.
echo ============================================================
echo ERROR - LIBXENON BUILD FAILED
echo ============================================================
echo.
echo Docker works, so the error above is from the actual build.
echo Copy the last part of the compiler output and send it to me.
echo.
pause
exit /b 1

:NO_OUTPUT
echo.
echo ============================================================
echo ERROR - BUILD FINISHED WITHOUT SeriesDash360.elf32
echo ============================================================
echo.
echo Expected:
echo   %ROOT%\platform\libxenon\SeriesDash360.elf32
echo.
pause
exit /b 1

:COPY_FAIL
echo.
echo ============================================================
echo ERROR - COULD NOT COPY BUILD OUTPUT
echo ============================================================
echo.
pause
exit /b 1
