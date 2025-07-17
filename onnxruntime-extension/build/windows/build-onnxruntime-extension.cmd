@ECHO off
SETLOCAL ENABLEDELAYEDEXPANSION

REM ============================================================
REM Build Script for ONNX Runtime Extension (Windows)
REM ------------------------------------------------------------
REM This script configures and builds the ONNX Runtime Extension
REM using CMake and Visual Studio 2022.
REM Usage:
REM   build-onnxruntime-extension.cmd [debug|release] [...]
REM Pass one or more build configurations (debug/release).
REM ============================================================

REM --- Check if cmake is available ---
where cmake >nul 2>&1
IF ERRORLEVEL 1 (
    ECHO [ERROR] CMake is not installed or not in PATH. Please install CMake 3.20 or higher and ensure it is available in your PATH.
    EXIT /b 1
)
ECHO [INFO] CMake found in PATH.

REM --- Set up environment variables and directories ---
SET ENL_ROOT=%~dp0..\..\..
SET ONNXRUNTIME_EXTENSION_HOME=%ENL_ROOT%\onnxruntime-extension

REM --- Define build and binary output directories ---
SET "BUILD_DIR=%ONNXRUNTIME_EXTENSION_HOME%\build\x64\build"
SET "BIN_DIR=%ONNXRUNTIME_EXTENSION_HOME%\build\x64\bin"

REM --- Clean up previous build files if they exist ---
IF EXIST "%BUILD_DIR%" (
    ECHO [INFO] Removing existing build files in "%BUILD_DIR%"
    rmdir /s /q "%BUILD_DIR%"
) ELSE (
    ECHO [INFO] No previous build directory found.
)

REM --- Create build and binary directories if they do not exist ---
IF NOT EXIST "%BUILD_DIR%" (
    ECHO [INFO] Creating build directory "%BUILD_DIR%"
    mkdir "%BUILD_DIR%"
)
IF NOT EXIST "%BIN_DIR%" (
    ECHO [INFO] Creating binary directory "%BIN_DIR%"
    mkdir "%BIN_DIR%"
)

REM --- Save current directory to return later ---
PUSHD

REM --- Change to build directory ---
CD /D "%BUILD_DIR%"

:LOOP

REM --- Parse build configuration argument ---
SET "CMAKE_CONFIGURATION=%~1"
IF NOT DEFINED CMAKE_CONFIGURATION SET "CMAKE_CONFIGURATION=release"
IF /I NOT "%CMAKE_CONFIGURATION%"=="debug" IF /I NOT "%CMAKE_CONFIGURATION%"=="release" (
    ECHO [ERROR] Invalid build configuration: "%CMAKE_CONFIGURATION%". Use "debug" or "release".
    POPD
    EXIT /b 1
)
ECHO [INFO] Using build configuration: "!CMAKE_CONFIGURATION!"

REM --- Generate build files with CMake ---
ECHO [INFO] Generating build files in "%BUILD_DIR%"...
cmake -S "%ONNXRUNTIME_EXTENSION_HOME%" -B "%BUILD_DIR%" -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="%BIN_DIR%" -DCMAKE_LIBRARY_OUTPUT_DIRECTORY="%BIN_DIR%" -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="%BIN_DIR%"
IF !ERRORLEVEL! NEQ 0 (
    ECHO [ERROR] Error configuring the project with CMake.
    POPD
    EXIT /b !ERRORLEVEL!
)
ECHO [INFO] CMake configuration completed successfully.

REM --- Build the project ---
ECHO [INFO] Building the project with configuration "!CMAKE_CONFIGURATION!"...
cmake --build "%BUILD_DIR%" --config !CMAKE_CONFIGURATION! --parallel
IF !ERRORLEVEL! NEQ 0 (
    ECHO [ERROR] Error building the project.
    POPD
    EXIT /b !ERRORLEVEL!
)
ECHO [INFO] Build completed for configuration "!CMAKE_CONFIGURATION!".

REM --- Process next argument, if any ---
SHIFT
IF NOT "%~1"=="" GOTO LOOP

REM --- Restore original directory ---
POPD

ECHO [SUCCESS] All builds completed successfully.
EXIT /b 0