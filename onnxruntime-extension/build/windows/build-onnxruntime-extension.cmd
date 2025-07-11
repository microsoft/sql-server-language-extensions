@ECHO off
SETLOCAL ENABLEDELAYEDEXPANSION

REM Check if cmake is available
where cmake >nul 2>&1
IF ERRORLEVEL 1 (
    ECHO [ERROR] CMake is not installed or not in PATH. Please install CMake 3.20 or higher and ensure it is available in your PATH.
    EXIT /b 1
)

REM Nuget packages directory and location of the build
REM
SET ENL_ROOT=%~dp0..\..\..
SET ONNXRUNTIME_EXTENSION_HOME=%ENL_ROOT%\onnxruntime-extension
SET DEFAULT_CMAKE_ROOT=%ONNXRUNTIME_EXTENSION_HOME%\packages\CMake-win64.3.15.5

REM Set build directory (always the same)
SET "BUILD_DIR=%ONNXRUNTIME_EXTENSION_HOME%\build\x64\build"
SET "BIN_DIR=%ONNXRUNTIME_EXTENSION_HOME%\build\x64\bin"

REM remove existing build files
IF EXIST "%BUILD_DIR%" (
    ECHO Removing existing build files in %BUILD_DIR%
    rmdir /s /q "%BUILD_DIR%"
)

REM create build and binary directories
IF NOT EXIST "%BUILD_DIR%" (
    ECHO Creating build directory %BUILD_DIR%
    mkdir "%BUILD_DIR%"
)
IF NOT EXIST "%BIN_DIR%" (
    ECHO Creating binary directory %BIN_DIR%
    mkdir "%BIN_DIR%"
)

REM save current directory
PUSHD "%CD%"

REM navigate to build directory
CD /D "%BUILD_DIR%"

:LOOP

REM Set cmake config to first arg
REM
REM Set CMAKE_CONFIGURATION to the first argument, defaulting to "release" if not provided
SET "CMAKE_CONFIGURATION=%~1"
IF NOT DEFINED CMAKE_CONFIGURATION SET "CMAKE_CONFIGURATION=release"
IF /I NOT "%CMAKE_CONFIGURATION%"=="debug" SET "CMAKE_CONFIGURATION=release"

REM build the project (always generate build files in BUILD_DIR)
ECHO Generating build files in %BUILD_DIR%
cmake -S "%ONNXRUNTIME_EXTENSION_HOME%" -B "%BUILD_DIR%" -G "Visual Studio 17 2022" -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="%BIN_DIR%" -DCMAKE_LIBRARY_OUTPUT_DIRECTORY="%BIN_DIR%" -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="%BIN_DIR%" -DCMAKE_BUILD_TYPE=%CMAKE_CONFIGURATION%
IF !ERRORLEVEL! NEQ 0 (
    ECHO Error configuring the project.
    POPD
    EXIT /b !ERRORLEVEL!
)

cmake --build "%BUILD_DIR%" --config !CMAKE_CONFIGURATION!
IF !ERRORLEVEL! NEQ 0 (
    ECHO Error building the project.
    POPD
    EXIT /b !ERRORLEVEL!
)

REM Advance arg passed to build-onnxruntime-extension.cmd
REM
SHIFT

REM Continue building using more configs until argv has been exhausted
REM
IF NOT "%~1"=="" GOTO LOOP

REM return to the original directory
POPD

ECHO Build completed successfully.
EXIT /b 0