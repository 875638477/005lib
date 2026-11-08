@echo off
setlocal DisableDelayedExpansion
rem Safe cleanup: preview by default, /apply to remove disposable outputs.
rem The script directory is the target. No archive extraction or shared settings changes.
set "EDA_CLEAN_LOOK=%~dp0"
:EDA_CLEAN_FIND
if exist "%EDA_CLEAN_LOOK%01script_file\Clean-EDA.ps1" goto EDA_CLEAN_RUN
for %%I in ("%EDA_CLEAN_LOOK%..") do set "EDA_CLEAN_PARENT=%%~fI\"
if /i "%EDA_CLEAN_PARENT%"=="%EDA_CLEAN_LOOK%" goto EDA_CLEAN_MISSING
set "EDA_CLEAN_LOOK=%EDA_CLEAN_PARENT%"
goto EDA_CLEAN_FIND
:EDA_CLEAN_RUN
"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%EDA_CLEAN_LOOK%01script_file\Clean-EDA.ps1" -Root "%~dp0."  %*
set "EDA_CLEAN_RESULT=%errorlevel%"
if "%~1"=="" pause
exit /b %EDA_CLEAN_RESULT%
:EDA_CLEAN_MISSING
echo Safe cleanup engine not found. No files were changed.
if "%~1"=="" pause
exit /b 2
