@ECHO off
SETLOCAL

REM Set environment variables
REM
SET ENL_ROOT=%~dp0..\..\..\..\..
SET REXTENSIONTEST_HOME=%ENL_ROOT%\language-extensions\R\test
SET REXTENSIONTEST_WORKING_DIR=%ENL_ROOT%\build-output\RExtension-test\windows
SET PACKAGES_ROOT=%ENL_ROOT%\packages
SET DEFAULT_R_HOME=%PACKAGES_ROOT%\R-4.0.5-win
SET DEFAULT_CMAKE_ROOT=%PACKAGES_ROOT%\CMake-win64.3.15.5

REM Find R_HOME and CMAKE_ROOT from user, or set to default.
REM Error code 203 is ENVVAR_NOT_FOUND.
REM
SET ENVVAR_NOT_FOUND=203

IF "%R_HOME%" == "" (
	IF EXIST %DEFAULT_R_HOME% (
		SET R_HOME=%DEFAULT_R_HOME%
	) ELSE (
		CALL :CHECKERROR %ENVVAR_NOT_FOUND% "Error: R_HOME variable must be set to build RExtension-test" || EXIT /b %ENVVAR_NOT_FOUND%
	)
)

IF "%CMAKE_ROOT%" == "" (
	IF EXIST %DEFAULT_CMAKE_ROOT% (
		SET CMAKE_ROOT=%DEFAULT_CMAKE_ROOT%
	) ELSE (
		CALL :CHECKERROR %ENVVAR_NOT_FOUND% "Error: CMAKE_ROOT variable must be set to build RExtension-test" || EXIT /b %ENVVAR_NOT_FOUND%
	)
)

REM Create build working directory
REM
RMDIR /s /q %REXTENSIONTEST_WORKING_DIR%
MKDIR %REXTENSIONTEST_WORKING_DIR%

:LOOP

REM Set cmake config to first arg
REM
SET CMAKE_CONFIGURATION=%1

REM The string comparison for CMAKE_CONFIGURATION is case-insensitive.
REM
IF NOT DEFINED CMAKE_CONFIGURATION (SET CMAKE_CONFIGURATION=release)
IF /I NOT %CMAKE_CONFIGURATION%==debug (SET CMAKE_CONFIGURATION=release)

SET BUILD_OUTPUT=%REXTENSIONTEST_WORKING_DIR%\%CMAKE_CONFIGURATION%
MKDIR %BUILD_OUTPUT%
PUSHD %BUILD_OUTPUT%

REM Make sure g++ is in the PATH.
REM Do not enclose the C:\Rtools\mingw_64\bin path in quotes - cmake test fails
REM Also need to have R_HOME\bin\x64 in the PATH so that linker finds definitions for R functions.
REM
SET PATH=C:\rtools40\mingw64\bin;C:\Rtools\mingw_64\bin;%R_HOME%\bin\x64;%PATH%

ECHO "[INFO] Generating RExtension test project build files using CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%"

REM Call cmake
call "%CMAKE_ROOT%\bin\cmake.exe" ^
	-G "MinGW Makefiles" ^
	-DCMAKE_INSTALL_PREFIX:PATH="%REXTENSIONTEST_WORKING_DIR%\\%CMAKE_CONFIGURATION%" ^
	-DENL_ROOT="%ENL_ROOT%" ^
	-DCMAKE_MAKE_PROGRAM=mingw32-make ^
	-DCMAKE_CONFIGURATION=%CMAKE_CONFIGURATION% ^
	-DPLATFORM=windows ^
	-DR_HOME="%R_HOME%" ^
	"%REXTENSIONTEST_HOME%\src"
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to generate make files for CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%" || EXIT /b %ERRORLEVEL%

ECHO "[INFO] Building RExtension test project using CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%"
REM Call cmake build
REM
CALL "mingw32-make.exe" -j all
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to build RExtension-test for CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%" || EXIT /b %ERRORLEVEL%

REM Advance arg passed to build-RExtension-test.cmd
REM
SHIFT

REM Continue building using more configs until argv has been exhausted
REM
IF NOT "%~1"=="" GOTO LOOP

EXIT /b %ERRORLEVEL%

:CHECKERROR
	IF %1 NEQ 0 (
		ECHO %2
		EXIT /b %1
	)

	EXIT /b 0
