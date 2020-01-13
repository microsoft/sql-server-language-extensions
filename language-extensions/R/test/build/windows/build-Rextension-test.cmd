@ECHO off
SETLOCAL

REM Set environment variables
SET ENL_ROOT=%~dp0..\..\..\..\..
SET REXTENSIONTEST_HOME=%ENL_ROOT%\language-extensions\R\test
SET REXTENSIONTEST_WORKING_DIR=%ENL_ROOT%\.build\Rextension-test\windows
SET CMAKE_ROOT=%ENL_ROOT%\packages\CMake-win64.3.15.5
RMDIR /s /q %REXTENSIONTEST_WORKING_DIR%
MKDIR %REXTENSIONTEST_WORKING_DIR%

:LOOP

REM Set cmake config to first arg
SET CMAKE_CONFIGURATION=%1

REM *Setting CMAKE_CONFIGURATION to anything but "debug" will set MSVC_BUILD_CONFIGURATION to "release".
REM The string comparison for CMAKE_CONFIGURATION is case-insensitive.
IF NOT DEFINED CMAKE_CONFIGURATION (SET CMAKE_CONFIGURATION=debug)
IF /I %CMAKE_CONFIGURATION%==debug (SET MSVC_BUILD_CONFIGURATION=debug) ELSE (SET MSVC_BUILD_CONFIGURATION=release)

REM VSCMD_START_DIR set the working directory to this variable after calling VsDevCmd.bat
REM otherwise, it will default to %USERPROFILE%\Source
SET VSCMD_START_DIR=%REXTENSIONTEST_WORKING_DIR%

REM Do not call VsDevCmd if the environment is already set. Otherwise, it will keep appending
REM to the PATH environment variable and it will be too long for windows to handle.
if not defined DevEnvDir (
	call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64
)

ECHO "[INFO] Generating R extension test project build files using CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%"

REM Call cmake
call "%CMAKE_ROOT%\bin\cmake.exe" ^
	-G "Visual Studio 15 2017 Win64" ^
	-DCMAKE_INSTALL_PREFIX:PATH="%REXTENSIONTEST_WORKING_DIR%\\%CMAKE_CONFIGURATION%" ^
	-DENL_ROOT=%ENL_ROOT% ^
	-DCMAKE_CONFIGURATION=%CMAKE_CONFIGURATION% ^
	 %REXTENSIONTEST_HOME%\src
IF %ERRORLEVEL% NEQ 0 GOTO CLEANUP

ECHO "[INFO] Building R extension test project using MSVC_BUILD_CONFIGURATION=%MSVC_BUILD_CONFIGURATION%"

REM Build the project
msbuild %REXTENSIONTEST_WORKING_DIR%\Rextension-test.vcxproj /m /property:Configuration=%MSVC_BUILD_CONFIGURATION% /property:Platform=x64

IF %ERRORLEVEL% NEQ 0 GOTO CLEANUP

SET EX=%ERRORLEVEL%

if "%EX%" neq "0" (
	echo "Error: Failed to build Rextension-test.vcxproj"
	GOTO CLEANUP
)

REM Advance arg passed to build-Rextension-test.cmd
SHIFT

REM Continue building using more configs until argv has been exhausted
IF NOT "%~1"=="" GOTO LOOP

REM Save exit code of compiler
SET EX=%ERRORLEVEL%

:CLEANUP

if "%EX%" neq "0" (
	echo "Build failed"
)

EXIT /b %ERRORLEVEL%
