@ECHO off
SETLOCAL

REM Set the root directory
SET "ENL_ROOT=%~dp0..\..\.."
SET "ONNXRUNTIME_EXTENSION_HOME=%ENL_ROOT%\onnxruntime-extension"

REM Check if nuget.exe is available
where nuget >nul 2>&1
IF ERRORLEVEL 1 (
    ECHO [ERROR] nuget.exe is not installed or not in PATH. Please install NuGet CLI and ensure it is available in your PATH.
    EXIT /b 1
)

REM Check if packages.config exists
IF NOT EXIST "%ONNXRUNTIME_EXTENSION_HOME%\packages.config" (
    ECHO [ERROR] packages.config not found at "%ONNXRUNTIME_EXTENSION_HOME%\packages.config".
    EXIT /b 1
)

REM Restore NuGet packages
nuget restore "%ONNXRUNTIME_EXTENSION_HOME%\packages.config" -PackagesDirectory "%ONNXRUNTIME_EXTENSION_HOME%\packages"
IF %ERRORLEVEL% NEQ 0 (
    ECHO Error restoring NuGet packages.
    EXIT /b %ERRORLEVEL%
)

REM Exit successfully
EXIT /b 0
