@echo off
setlocal

REM Define paths
set "OUTPUT_DIR=%~dp0..\x64"
set "ZIP_DIR=%~dp0..\build-output\onnxextension\packages"
set "ZIP_NAME=onnxruntime-extension.zip"

REM Parse argument
if "%~1"=="" (
    echo Usage: %~nx0 [Debug^|Release]
    exit /b 1
)
set "BuildConfig=%~1"

REM Validate argument
if /i not "%BuildConfig%"=="Debug" if /i not "%BuildConfig%"=="Release" (
    echo Invalid build configuration: "%BuildConfig%"
    echo Usage: %~nx0 [Debug^|Release]
    exit /b 1
)

REM Check if PowerShell is available
where powershell >nul 2>&1
if errorlevel 1 (
    echo [ERROR] PowerShell is not installed or not in PATH. Please install PowerShell and ensure it is available in your PATH.
    exit /b 1
)

REM Check if the output directory for the build config exists
if not exist "%OUTPUT_DIR%\%BuildConfig%" (
    echo Output directory not found: "%OUTPUT_DIR%\%BuildConfig%"
    exit /b 1
)

REM Create directory if it doesn't exist
if not exist "%ZIP_DIR%\%BuildConfig%" mkdir "%ZIP_DIR%\%BuildConfig%"

REM Zip files
echo Creating %BuildConfig% zip at "%ZIP_DIR%\%BuildConfig%\%ZIP_NAME%"...
powershell -Command "Compress-Archive -Path '%OUTPUT_DIR%\%BuildConfig%\*' -DestinationPath '%ZIP_DIR%\%BuildConfig%\%ZIP_NAME%' -Force"

echo ONNX extension packaging complete.

endlocal