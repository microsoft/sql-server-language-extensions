@ECHO off
SETLOCAL

REM Set environment variables
REM
SET ENL_ROOT=%~dp0..\..\..\..\..
SET PACKAGES_ROOT=%ENL_ROOT%\packages
SET PYTHONEXTENSION_TEST_HOME=%ENL_ROOT%\language-extensions\python\test
SET PYTHONEXTENSION_TEST_WORKING_DIR=%ENL_ROOT%\build-output\pythonextension-test\windows

RMDIR /s /q %PYTHONEXTENSION_TEST_WORKING_DIR%
MKDIR %PYTHONEXTENSION_TEST_WORKING_DIR%

SET DEFAULT_BOOST_ROOT=%PACKAGES_ROOT%\boost.1.69.0.0\lib\native
SET DEFAULT_PYTHONHOME=%PACKAGES_ROOT%\python
SET DEFAULT_CMAKE_ROOT=%PACKAGES_ROOT%\CMake-win64.3.15.5

REM Find boost, python, and cmake paths from user, or set to default for tests.
REM 
SET ENVVAR_NOT_FOUND=203

IF "%BOOST_ROOT%" == "" (
	IF EXIST %DEFAULT_BOOST_ROOT% (
		SET BOOST_ROOT=%DEFAULT_BOOST_ROOT%
	) ELSE (
		CALL :CHECKERROR %ENVVAR_NOT_FOUND% "Error: BOOST_ROOT variable must be set to build the python extension test" || EXIT /b %ENVVAR_NOT_FOUND%
	)
)

IF "%PYTHONHOME%" == "" (
	IF EXIST %DEFAULT_PYTHONHOME% (
		SET PYTHONHOME=%DEFAULT_PYTHONHOME%
	) ELSE (
		CALL :CHECKERROR %ENVVAR_NOT_FOUND% "Error: PYTHONHOME variable must be set to build the python extension test" || EXIT /b %ENVVAR_NOT_FOUND%
	)
)

IF "%CMAKE_ROOT%" == "" (
	IF EXIST %DEFAULT_CMAKE_ROOT% (
		SET CMAKE_ROOT=%DEFAULT_CMAKE_ROOT%
	) ELSE (
		CALL :CHECKERROR %ENVVAR_NOT_FOUND% "Error: CMAKE_ROOT variable must be set to build the python extension test" || EXIT /b %ENVVAR_NOT_FOUND%
	)
)

:LOOP

REM Set cmake config to first arg
REM
SET CMAKE_CONFIGURATION=%1

REM *Setting CMAKE_CONFIGURATION to anything but "debug" will set CMAKE_CONFIGURATION to "release".
REM The string comparison for CMAKE_CONFIGURATION is case-insensitive.
REM
IF NOT DEFINED CMAKE_CONFIGURATION (SET CMAKE_CONFIGURATION=debug)
IF /I %CMAKE_CONFIGURATION%==debug (SET CMAKE_CONFIGURATION=debug) ELSE (SET CMAKE_CONFIGURATION=release)

REM VSCMD_START_DIR set the working directory to this variable after calling VsDevCmd.bat
REM otherwise, it will default to %USERPROFILE%\Source
REM
SET VSCMD_START_DIR=%PYTHONEXTENSION_TEST_WORKING_DIR%

REM Do not call VsDevCmd if the environment is already set. Otherwise, it will keep appending
REM to the PATH environment variable and it will be too long for windows to handle.
REM
if not defined DevEnvDir (
	call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64
)

ECHO "[INFO] Generating pythonextension test project build files using CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%"

REM Call cmake
REM
call "%CMAKE_ROOT%\bin\cmake.exe" ^
	-G "Visual Studio 15 2017 Win64" ^
	-DCMAKE_INSTALL_PREFIX:PATH="%PYTHONEXTENSION_TEST_WORKING_DIR%/%CMAKE_CONFIGURATION%" ^
	-DENL_ROOT="%ENL_ROOT%" ^
	-DCMAKE_CONFIGURATION=%CMAKE_CONFIGURATION% ^
	-DPLATFORM=windows ^
	-DPYTHONHOME="%PYTHONHOME%" ^
	-DBOOST_ROOT="%BOOST_ROOT%" ^
	 %PYTHONEXTENSION_TEST_HOME%\src

IF %ERRORLEVEL% NEQ 0 GOTO CLEANUP

ECHO "[INFO] Building pythonextension test project using CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%"

REM Build the project
REM
msbuild %PYTHONEXTENSION_TEST_WORKING_DIR%\pythonextension-test.vcxproj /m /property:Configuration=%CMAKE_CONFIGURATION% /property:Platform=x64

IF %ERRORLEVEL% NEQ 0 GOTO CLEANUP

SET EX=%ERRORLEVEL%

if "%EX%" neq "0" (
	echo "Error: Failed to build pythonextension-test.vcxproj"
	GOTO CLEANUP
)

REM Advance arg passed to build-pythonextension-test.cmd
REM
SHIFT

REM Continue building using more configs until argv has been exhausted
REM
IF NOT "%~1"=="" GOTO LOOP

REM Save exit code of compiler
REM
SET EX=%ERRORLEVEL%

:CLEANUP

if "%EX%" neq "0" (
	echo "Build failed"
)

EXIT /b %ERRORLEVEL%

:CHECKERROR
	IF %1 NEQ 0 (
		ECHO %2
		EXIT /b %1
	)

	EXIT /b 0
