@echo off
setlocal EnableExtensions

REM DD-SSH Windows checksum helper wrapper.
REM Run from the repository root after creating release artifacts under dist\.

powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0generate-checksums-windows.ps1" %*
exit /b %ERRORLEVEL%
