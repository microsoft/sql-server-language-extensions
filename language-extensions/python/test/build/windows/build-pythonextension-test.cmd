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

SET BOOST_VERSION=1.79.0
SET BOOST_VERSION_IN_UNDERSCORE=1_79_0
SET DEFAULT_BOOST_ROOT=%PACKAGES_ROOT%\boost_%BOOST_VERSION_IN_UNDERSCORE%
SET DEFAULT_BOOST_PYTHON_ROOT=%DEFAULT_BOOST_ROOT%\stage\lib
SET DEFAULT_PYTHONHOME=C:\Python310
SET DEFAULT_CMAKE_ROOT=%PACKAGES_ROOT%\CMake-win64.3.15.5

REM Find boost, python, and cmake paths from user, or set to default for tests.
REM
SET ENVVAR_NOT_FOUND=203

IF "%BOOST_ROOT%" == "" (
	IF EXIST "%DEFAULT_BOOST_ROOT%" (
		SET BOOST_ROOT=%DEFAULT_BOOST_ROOT%
	) ELSE (
		CALL :CHECKERROR %ENVVAR_NOT_FOUND% "Error: BOOST_ROOT variable must be set to build the python extension test" || EXIT /b %ENVVAR_NOT_FOUND%
	)
)

IF "%BOOST_PYTHON_ROOT%" == "" (
	IF EXIST %DEFAULT_BOOST_PYTHON_ROOT% (
		SET BOOST_PYTHON_ROOT=%DEFAULT_BOOST_PYTHON_ROOT%
	) ELSE (
		CALL :CHECKERROR %ENVVAR_NOT_FOUND% "Error: BOOST_PYTHON_ROOT variable must be set to build the python extension test" || EXIT /b %ENVVAR_NOT_FOUND%
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
IF NOT DEFINED CMAKE_CONFIGURATION (SET CMAKE_CONFIGURATION=release)
IF /I %CMAKE_CONFIGURATION%==debug (SET CMAKE_CONFIGURATION=debug) ELSE (SET CMAKE_CONFIGURATION=release)

REM VSCMD_START_DIR set the working directory to this variable after calling VsDevCmd.bat
REM otherwise, it will default to %USERPROFILE%\Source
REM
SET VSCMD_START_DIR=%PYTHONEXTENSION_TEST_WORKING_DIR%

REM Do not call VsDevCmd if the environment is already set. Otherwise, it will keep appending
REM to the PATH environment variable and it will be too long for windows to handle.
REM
if not defined DevEnvDir (
	call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64
)

ECHO "[INFO] Generating pythonextension test project build files using CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%"

SET BUILD_OUTPUT=%PYTHONEXTENSION_TEST_WORKING_DIR%\%CMAKE_CONFIGURATION%
MKDIR %BUILD_OUTPUT%
PUSHD %BUILD_OUTPUT%

REM Call cmake
REM
call "%CMAKE_ROOT%\bin\cmake.exe" ^
	-G "Visual Studio 16 2019" ^
	-DPLATFORM=windows ^
	-DENL_ROOT="%ENL_ROOT%" ^
	-DCMAKE_CONFIGURATION=%CMAKE_CONFIGURATION% ^
	-DPYTHONHOME="%PYTHONHOME%" ^
	-DBOOST_ROOT="%BOOST_ROOT%" ^
	-DBOOST_PYTHON_ROOT="%BOOST_PYTHON_ROOT%" ^
	 %PYTHONEXTENSION_TEST_HOME%\src

CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to generate make files for CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%" || EXIT /b %ERRORLEVEL%

ECHO "[INFO] Building pythonextension test project using CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%"


REM Call cmake build
REM
CALL "%CMAKE_ROOT%\bin\cmake.exe" --build . --config %CMAKE_CONFIGURATION% --target INSTALL

CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to build Python extension test for CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%" || EXIT /b %ERRORLEVEL%

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
