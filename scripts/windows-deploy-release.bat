@echo off
setlocal EnableExtensions

REM DD-SSH Windows deployment helper
REM Checkpoint: dev 0.1.5.6 — Windows standalone deployment test
REM Run from the DD-SSH repository root in a Windows Command Prompt.
REM
REM Optional overrides:
REM   set QT_DIR=C:\Qt\6.11.1\msvc2022_64
REM   set VCPKG_ROOT=C:\dev\vcpkg
REM   set BUILD_DIR=build-win-release
REM   set DIST_DIR=dist\windows-release

if "%QT_DIR%"=="" set "QT_DIR=C:\Qt\6.11.1\msvc2022_64"
if "%VCPKG_ROOT%"=="" set "VCPKG_ROOT=C:\dev\vcpkg"
if "%BUILD_DIR%"=="" set "BUILD_DIR=build-win-release"
if "%DIST_DIR%"=="" set "DIST_DIR=dist\windows-release"

set "APP_EXE=%BUILD_DIR%\dd-ssh.exe"
set "WDEPLOY=%QT_DIR%\bin\windeployqt.exe"
set "VCPKG_BIN=%VCPKG_ROOT%\installed\x64-windows\bin"

call :require_file "%APP_EXE%" "Release executable" || exit /b 1
call :require_file "%WDEPLOY%" "windeployqt" || exit /b 1
call :require_dir "%VCPKG_BIN%" "vcpkg runtime bin folder" || exit /b 1

echo [DD-SSH] Checkpoint: dev 0.1.5.6 - Windows standalone deployment test
echo [DD-SSH] Qt dir:      %QT_DIR%
echo [DD-SSH] vcpkg root:  %VCPKG_ROOT%
echo [DD-SSH] build dir:   %BUILD_DIR%
echo [DD-SSH] deploy dir:  %DIST_DIR%
echo.

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

echo [DD-SSH] Running windeployqt with Qt WebEngine support...
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
echo [DD-SSH] Running deployment sanity checks...
call :require_file "%DIST_DIR%\dd-ssh.exe" "deployed dd-ssh.exe" || exit /b 1
call :require_file "%DIST_DIR%\platforms\qwindows.dll" "Qt Windows platform plugin" || exit /b 1
call :warn_match "%DIST_DIR%\Qt6Core.dll" "Qt6Core.dll"
call :warn_match "%DIST_DIR%\Qt6Widgets.dll" "Qt6Widgets.dll"
call :warn_match "%DIST_DIR%\Qt6WebEngine*.dll" "Qt WebEngine DLLs"
call :warn_match "%DIST_DIR%\QtWebEngineProcess.exe" "QtWebEngineProcess.exe"
call :warn_dir "%DIST_DIR%\resources" "Qt WebEngine resources folder"
call :warn_dir "%DIST_DIR%\translations" "Qt translations folder"
call :warn_match "%DIST_DIR%\*ssh*.dll" "libssh runtime DLL"
call :warn_match "%DIST_DIR%\libcrypto*.dll" "OpenSSL libcrypto runtime DLL"
call :warn_match "%DIST_DIR%\libssl*.dll" "OpenSSL libssl runtime DLL"
call :warn_match "%DIST_DIR%\zlib*.dll" "zlib runtime DLL"

echo.
echo [DD-SSH] Deployment folder created:
echo "%CD%\%DIST_DIR%"
echo.
echo [DD-SSH] Test from a NEW normal Command Prompt without Qt/vcpkg PATH:
echo cd /d "%CD%\%DIST_DIR%"
echo dd-ssh.exe
echo.
echo [DD-SSH] Then copy the whole folder to a clean Windows 10 machine and run dd-ssh.exe there.
echo.

endlocal
exit /b 0

:require_file
if not exist "%~1" (
    echo [DD-SSH] ERROR: Missing %~2:
    echo [DD-SSH]        %~1
    exit /b 1
)
exit /b 0

:require_dir
if not exist "%~1\NUL" (
    echo [DD-SSH] ERROR: Missing %~2:
    echo [DD-SSH]        %~1
    exit /b 1
)
exit /b 0

:warn_match
if not exist "%~1" (
    echo [DD-SSH] WARNING: Could not find %~2:
    echo [DD-SSH]          %~1
)
exit /b 0

:warn_dir
if not exist "%~1\NUL" (
    echo [DD-SSH] WARNING: Could not find %~2:
    echo [DD-SSH]          %~1
)
exit /b 0
