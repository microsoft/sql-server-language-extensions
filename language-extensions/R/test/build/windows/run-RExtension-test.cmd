@ECHO off
SETLOCAL

REM Set environment variables
SET ENL_ROOT=%~dp0..\..\..\..\..
SET REXTENSIONTEST_WORKING_DIR=%ENL_ROOT%\.build\RExtension-test\windows
SET REXTENSION_WORKING_DIR=%ENL_ROOT%\.build\RExtension\windows
SET PACKAGES_ROOT=%ENL_ROOT%\packages
SET GTEST_HOME=%PACKAGES_ROOT%\Microsoft.googletest.v140.windesktop.msvcstl.dyn.rt-dyn.1.8.0
SET GTEST_LIB_PATH=%GTEST_HOME%\lib\native\v140\windesktop\msvcstl\dyn\rt-dyn\x64

REM Set cmake config to first arg
SET CMAKE_CONFIGURATION=%1

REM *Setting CMAKE_CONFIGURATION to anything but "debug" will set MSVC_BUILD_CONFIGURATION to "release".
REM The string comparison for CMAKE_CONFIGURATION is case-insensitive.
IF NOT DEFINED CMAKE_CONFIGURATION (SET CMAKE_CONFIGURATION=debug)
IF /I %CMAKE_CONFIGURATION%==debug (SET MSVC_BUILD_CONFIGURATION=debug) ELSE (SET MSVC_BUILD_CONFIGURATION=release)

pushd %REXTENSIONTEST_WORKING_DIR%\%MSVC_BUILD_CONFIGURATION%
copy %REXTENSION_WORKING_DIR%\%MSVC_BUILD_CONFIGURATION%\libRExtension.dll .
copy %GTEST_LIB_PATH%\debug\gtest.dll .
SET R_HOME=%PACKAGES_ROOT%\External-R.MRO-3.5.2.R.3.5.2.229\Windows
SET PATH=%R_HOME%\bin\x64;%PATH%
RExtension-test.exe --gtest_output=xml:%ENL_ROOT%\out\TestReport_RExtension-test.xml
IF %ERRORLEVEL% NEQ 0 GOTO error
popd

:error
EXIT /b %ERRORLEVEL%

:USAGE
echo.
echo Usage:
echo %0 { debug ^| release }
