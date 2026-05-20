@echo off
setlocal EnableExtensions

REM DD-SSH Windows deployment helper
REM Checkpoint: dev 0.1.5.7 — Known-host multi-key portability polish
REM This is the simple working deployment BAT validated during the 0.1.5.6 Windows standalone test.
REM Run from the DD-SSH repository root after building build-win-release\dd-ssh.exe.

REM --- DEBUG SWITCH ---
if "%DEBUG%"=="1" (
    set "LOG=echo"
) else (
    set "LOG=rem"
)

%LOG% [DEBUG] starting script...

REM --- DEFAULTS ---
if not defined QT_DIR set "QT_DIR=C:\Qt\6.11.1\msvc2022_64"
if not defined BUILD_DIR set "BUILD_DIR=build-win-release"
if not defined DIST_DIR set "DIST_DIR=dist\windows-release"
if not defined VCPKG_ROOT set "VCPKG_ROOT=C:\dev\vcpkg"

REM --- VCPKG VALIDATION ---
%LOG% Checking VCPKG_ROOT...
if exist "%VCPKG_ROOT%\installed\x64-windows\bin" goto :vcpkg_ok

%LOG% Falling back to C:\dev\vcpkg
set "VCPKG_ROOT=C:\dev\vcpkg"

:vcpkg_ok

REM --- PATHS ---
set "APP_EXE=%BUILD_DIR%\dd-ssh.exe"
set "WDEPLOY=%QT_DIR%\bin\windeployqt.exe"
set "VCPKG_BIN=%VCPKG_ROOT%\installed\x64-windows\bin"

%LOG% APP_EXE=%APP_EXE%
%LOG% WDEPLOY=%WDEPLOY%
%LOG% VCPKG_BIN=%VCPKG_BIN%

REM --- CHECKS ---
call :require_file "%APP_EXE%"
if errorlevel 1 goto :fail

call :require_file "%WDEPLOY%"
if errorlevel 1 goto :fail

call :require_dir "%VCPKG_BIN%"
if errorlevel 1 goto :fail

REM --- CLEAN DIST ---
%LOG% Cleaning dist...
if exist "%DIST_DIR%" rmdir /s /q "%DIST_DIR%"
mkdir "%DIST_DIR%" || goto :fail

REM --- COPY EXE ---
%LOG% Copy exe...
copy /y "%APP_EXE%" "%DIST_DIR%\" >nul || goto :fail

REM --- WINDEPLOYQT ---
%LOG% Running windeployqt...
"%WDEPLOY%" --release "%DIST_DIR%\dd-ssh.exe" >nul
if errorlevel 1 goto :fail

REM --- COPY DLLS ---
%LOG% Copying DLLs...
copy /y "%VCPKG_BIN%\*.dll" "%DIST_DIR%\" >nul

REM --- FINAL CHECK ---
call :require_file "%DIST_DIR%\dd-ssh.exe"
if errorlevel 1 goto :fail

REM --- RUN APP ---
%LOG% Starting app...
cd /d "%DIST_DIR%"
start "" dd-ssh.exe

exit /b 0


:fail
echo ERROR
exit /b 1


:require_file
if exist "%~1" exit /b 0
%LOG% Missing file: %~1
exit /b 1


:require_dir
if exist "%~1" exit /b 0
%LOG% Missing dir: %~1
exit /b 1
