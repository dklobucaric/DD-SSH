@echo off
setlocal EnableExtensions

REM DD-SSH Windows deployment helper
REM Checkpoint: dev 0.1.5.4 — Windows deployment experiment
REM Run from the DD-SSH repository root in a Windows command prompt.

if "%QT_DIR%"=="" set "QT_DIR=C:\Qt\6.11.1\msvc2022_64"
if "%VCPKG_ROOT%"=="" set "VCPKG_ROOT=C:\dev\vcpkg"
if "%BUILD_DIR%"=="" set "BUILD_DIR=build-win-release"
if "%DIST_DIR%"=="" set "DIST_DIR=dist\windows-release"

set "APP_EXE=%BUILD_DIR%\dd-ssh.exe"
set "WDEPLOY=%QT_DIR%\bin\windeployqt.exe"
set "VCPKG_BIN=%VCPKG_ROOT%\installed\x64-windows\bin"

if not exist "%APP_EXE%" (
    echo [DD-SSH] ERROR: %APP_EXE% not found.
    echo [DD-SSH] Build Release first. See docs\WINDOWS_BUILD.md.
    exit /b 1
)

if not exist "%WDEPLOY%" (
    echo [DD-SSH] ERROR: windeployqt not found at:
    echo %WDEPLOY%
    echo [DD-SSH] Set QT_DIR or install Qt MSVC 2022 64-bit.
    exit /b 1
)

if not exist "%VCPKG_BIN%" (
    echo [DD-SSH] ERROR: vcpkg runtime bin folder not found at:
    echo %VCPKG_BIN%
    echo [DD-SSH] Set VCPKG_ROOT or install libssh with vcpkg.
    exit /b 1
)

echo [DD-SSH] Cleaning deploy folder: %DIST_DIR%
if exist "%DIST_DIR%" rmdir /s /q "%DIST_DIR%"
mkdir "%DIST_DIR%"

if errorlevel 1 (
    echo [DD-SSH] ERROR: Could not create deploy folder.
    exit /b 1
)

echo [DD-SSH] Copying executable...
copy /y "%APP_EXE%" "%DIST_DIR%\" >nul

if errorlevel 1 (
    echo [DD-SSH] ERROR: Could not copy executable.
    exit /b 1
)

echo [DD-SSH] Running windeployqt...
"%WDEPLOY%" --release --compiler-runtime --webengine "%DIST_DIR%\dd-ssh.exe"

if errorlevel 1 (
    echo [DD-SSH] ERROR: windeployqt failed.
    exit /b 1
)

echo [DD-SSH] Copying vcpkg runtime DLLs...
copy /y "%VCPKG_BIN%\*.dll" "%DIST_DIR%\" >nul

if errorlevel 1 (
    echo [DD-SSH] WARNING: Could not copy one or more vcpkg DLLs.
    echo [DD-SSH] Check %VCPKG_BIN% manually if the app fails to start.
)

echo.
echo [DD-SSH] Deployment folder created:
echo %CD%\%DIST_DIR%
echo.
echo [DD-SSH] Test from a new normal Command Prompt without Qt/vcpkg PATH:
echo cd /d %CD%\%DIST_DIR%
echo dd-ssh.exe
echo.

endlocal
