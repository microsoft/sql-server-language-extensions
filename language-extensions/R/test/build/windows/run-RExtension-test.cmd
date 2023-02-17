@ECHO off
SETLOCAL

REM Set environment variables
REM
SET ENL_ROOT=%~dp0..\..\..\..\..
SET REXTENSIONTEST_WORKING_DIR=%ENL_ROOT%\build-output\RExtension-test\windows
SET REXTENSION_WORKING_DIR=%ENL_ROOT%\build-output\RExtension\windows
SET PACKAGES_ROOT=%ENL_ROOT%\packages
SET GTEST_HOME=%PACKAGES_ROOT%\Microsoft.googletest.v140.windesktop.msvcstl.dyn.rt-dyn.1.8.1.3
SET GTEST_LIB_PATH=%GTEST_HOME%\lib\native\v140\windesktop\msvcstl\dyn\rt-dyn\x64

SET DEFAULT_R_HOME=%PACKAGES_ROOT%\R-4.0.5-win

REM Find R_HOME from user, or set to default.
REM Error code 203 is ENVVAR_NOT_FOUND.
REM
SET ENVVAR_NOT_FOUND=203

IF "%R_HOME%" == "" (
	IF EXIST %DEFAULT_R_HOME% (
		SET R_HOME=%DEFAULT_R_HOME%
	) ELSE (
		CALL :CHECKERROR %ENVVAR_NOT_FOUND% "Error: R_HOME variable must be set to run RExtension-test" || EXIT /b %ENVVAR_NOT_FOUND%
	)
)

:LOOP

REM Set cmake config to first arg
REM
SET CMAKE_CONFIGURATION=%1

REM *Setting CMAKE_CONFIGURATION to anything but "debug" will set MSVC_BUILD_CONFIGURATION to "release".
REM The string comparison for CMAKE_CONFIGURATION is case-insensitive.
REM
IF NOT DEFINED CMAKE_CONFIGURATION (SET CMAKE_CONFIGURATION=release)
IF /I %CMAKE_CONFIGURATION%==debug (SET MSVC_BUILD_CONFIGURATION=debug) ELSE (SET MSVC_BUILD_CONFIGURATION=release)

pushd %REXTENSIONTEST_WORKING_DIR%\%MSVC_BUILD_CONFIGURATION%
copy %REXTENSION_WORKING_DIR%\%MSVC_BUILD_CONFIGURATION%\libRExtension.dll .
copy %GTEST_LIB_PATH%\debug\gtest.dll .
SET PATH=%R_HOME%\bin\x64;%PATH%
RExtension-test.exe --gtest_output=xml:%ENL_ROOT%\out\TestReport_RExtension-test.xml
IF %ERRORLEVEL% NEQ 0 GOTO error
popd

REM Advance arg passed to build-RExtension-test.cmd
REM
SHIFT

REM Continue running using more configs until argv has been exhausted
REM
IF NOT "%~1"=="" GOTO LOOP

:error
EXIT /b %ERRORLEVEL%

:USAGE
echo.
echo Usage:
echo %0 { debug ^| release }
