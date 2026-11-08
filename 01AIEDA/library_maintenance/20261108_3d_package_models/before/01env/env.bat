@echo off
setlocal
set "PCBENV_TARGET=C:\Program0\Cadence\SPB_Data\pcbenv"
if not exist "%PCBENV_TARGET%" mkdir "%PCBENV_TARGET%"
copy /Y "%~dp0env" "%PCBENV_TARGET%\env"
exit /b %errorlevel%
