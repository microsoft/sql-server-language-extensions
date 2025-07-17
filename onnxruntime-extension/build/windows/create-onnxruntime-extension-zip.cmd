@echo off
setlocal

REM ============================================
REM Define paths for output and zip directories
REM ============================================
set "OUTPUT_DIR=%~dp0..\x64\bin"
set "ZIP_DIR=%~dp0..\build-output\onnxextension\windows"
set "ZIP_NAME=onnxruntime-extension.zip"

echo [INFO] OUTPUT_DIR: "%OUTPUT_DIR%"
echo [INFO] ZIP_DIR: "%ZIP_DIR%"
echo [INFO] ZIP_NAME: "%ZIP_NAME%"

REM ============================================
REM Check if PowerShell is available in PATH
REM ============================================
where powershell >nul 2>&1
if errorlevel 1 (
    echo [ERROR] PowerShell is not installed or not in PATH. Please install PowerShell and ensure it is available in your PATH.
    exit /b 1
)
echo [INFO] PowerShell found in PATH.

REM ============================================
REM Parse arguments: default to both Debug and Release if none specified
REM ============================================
if "%~1"=="" (
    set "BuildConfigs=Debug Release"
    echo [INFO] No build config specified, defaulting to Debug and Release.
) else (
    set "BuildConfigs="
)

:collect_args
if "%~1"=="" goto after_collect
if /i "%~1"=="Debug" (
    set "BuildConfigs=%BuildConfigs% Debug"
) else if /i "%~1"=="Release" (
    set "BuildConfigs=%BuildConfigs% Release"
) else (
    echo [ERROR] Invalid build configuration: "%~1"
    echo Usage: %~nx0 [Debug] [Release]
    exit /b 1
)
shift
goto collect_args
:after_collect

REM ============================================
REM Track if any valid build configuration is found
REM ============================================
set "found=0"

REM ============================================
REM Process each specified build configuration
REM ============================================
for %%C in (%BuildConfigs%) do (
    set "BuildConfig=%%C"
    call :process_config
)

REM ============================================
REM If no valid build configuration was found, exit with error
REM ============================================
if "%found%"=="0" (
    echo [ERROR] No valid build configuration specified.
    echo Usage: %~nx0 [Debug] [Release]
    exit /b 1
)

echo [INFO] ONNX extension packaging complete.
endlocal
exit /b 0

REM ============================================
REM Subroutine: process_config
REM Handles zipping for a single build configuration
REM ============================================
:process_config
echo [INFO] Processing build configuration: "%BuildConfig%"

REM Check if the output directory for the build config exists
if not exist "%OUTPUT_DIR%\%BuildConfig%" (
    echo [ERROR] Output directory not found: "%OUTPUT_DIR%\%BuildConfig%"
    goto :eof
)

REM Mark that a valid configuration was found
set "found=1"

REM Create output directories if they do not exist
if not exist "%ZIP_DIR%\%BuildConfig%" (
    echo [INFO] Creating directory: "%ZIP_DIR%\%BuildConfig%"
    mkdir "%ZIP_DIR%\%BuildConfig%"
    if errorlevel 1 (
        echo [ERROR] Failed to create directory: "%ZIP_DIR%\%BuildConfig%"
        goto :eof
    )
)
if not exist "%ZIP_DIR%\%BuildConfig%\packages" (
    echo [INFO] Creating directory: "%ZIP_DIR%\%BuildConfig%\packages"
    mkdir "%ZIP_DIR%\%BuildConfig%\packages"
    if errorlevel 1 (
        echo [ERROR] Failed to create directory: "%ZIP_DIR%\%BuildConfig%\packages"
        goto :eof
    )
)

REM Zip the build output using PowerShell's Compress-Archive
echo [INFO] Creating %BuildConfig% zip at "%ZIP_DIR%\%BuildConfig%\packages\%ZIP_NAME%"...
powershell -Command "Compress-Archive -Path \"%OUTPUT_DIR%\%BuildConfig%\*\" -DestinationPath \"%ZIP_DIR%\%BuildConfig%\packages\%ZIP_NAME%\" -Force"
if errorlevel 1 (
    echo [ERROR] Compress-Archive failed for "%BuildConfig%". Check PowerShell output above.
    goto :eof
) else (
    echo [INFO] Successfully created zip for "%BuildConfig%".
)

goto :eof