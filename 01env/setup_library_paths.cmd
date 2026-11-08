@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0setup_library_paths.ps1" %*
exit /b %errorlevel%
