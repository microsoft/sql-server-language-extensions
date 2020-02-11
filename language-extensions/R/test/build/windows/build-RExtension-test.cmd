@ECHO off
SETLOCAL

REM Set environment variables
REM
SET ENL_ROOT=%~dp0..\..\..\..\..
SET REXTENSIONTEST_HOME=%ENL_ROOT%\language-extensions\R\test
SET REXTENSIONTEST_WORKING_DIR=%ENL_ROOT%\.build\RExtension-test\windows
SET PACKAGES_ROOT=%ENL_ROOT%\packages
SET CMAKE_ROOT=%PACKAGES_ROOT%\CMake-win64.3.15.5

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
IF NOT DEFINED CMAKE_CONFIGURATION (SET CMAKE_CONFIGURATION=debug)
IF /I NOT %CMAKE_CONFIGURATION%==debug (SET CMAKE_CONFIGURATION=release)

SET BUILD_OUTPUT=%REXTENSIONTEST_WORKING_DIR%\%CMAKE_CONFIGURATION%
MKDIR %BUILD_OUTPUT%
PUSHD %BUILD_OUTPUT%

REM Make sure g++ is in the PATH.
REM Do not enclose the C:\Rtools\mingw_64\bin path in quotes - cmake test fails
REM
SET PATH=C:\Rtools\bin;C:\Rtools\mingw_64\bin;%PATH%

ECHO "[INFO] Generating RExtension test project build files using CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%"

REM Call cmake
call "%CMAKE_ROOT%\bin\cmake.exe" ^
	-G "MinGW Makefiles" ^
	-DCMAKE_INSTALL_PREFIX:PATH="%REXTENSIONTEST_WORKING_DIR%\\%CMAKE_CONFIGURATION%" ^
	-DENL_ROOT=%ENL_ROOT% ^
	-DCMAKE_MAKE_PROGRAM=mingw32-make ^
	-DCMAKE_CONFIGURATION=%CMAKE_CONFIGURATION% ^
	-DPLATFORM=windows ^
	 %REXTENSIONTEST_HOME%\src
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to generate make files for CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%" || EXIT /b %ERRORLEVEL%

ECHO "[INFO] Building RExtension test project using CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%"
REM Call cmake build
REM
CALL "mingw32-make.exe" all
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
