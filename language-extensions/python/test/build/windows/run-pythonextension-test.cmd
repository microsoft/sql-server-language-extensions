@ECHO off
SETLOCAL

REM Set environment variables
REM
SET ENL_ROOT=%~dp0..\..\..\..\..
SET PACKAGES_ROOT=%ENL_ROOT%\packages
SET PYTHONEXTENSION_TEST_WORKING_DIR=%ENL_ROOT%\build-output\pythonextension-test\windows
SET PYTHONEXTENSION_WORKING_DIR=%ENL_ROOT%\build-output\pythonextension\windows
SET GTEST_HOME=%PACKAGES_ROOT%\Microsoft.googletest.v140.windesktop.msvcstl.dyn.rt-dyn.1.8.1.3
SET GTEST_LIB_PATH=%GTEST_HOME%\lib\native\v140\windesktop\msvcstl\dyn\rt-dyn\x64

:LOOP

REM Set cmake config to first arg
REM
SET CMAKE_CONFIGURATION=%1

REM *Setting CMAKE_CONFIGURATION to anything but "debug" will set CMAKE_CONFIGURATION to "release".
REM The string comparison for CMAKE_CONFIGURATION is case-insensitive.
REM
IF NOT DEFINED CMAKE_CONFIGURATION (SET CMAKE_CONFIGURATION=release)
IF /I %CMAKE_CONFIGURATION%==debug (SET CMAKE_CONFIGURATION=debug) ELSE (SET CMAKE_CONFIGURATION=release)

pushd %PYTHONEXTENSION_TEST_WORKING_DIR%\%CMAKE_CONFIGURATION%
copy %PYTHONEXTENSION_WORKING_DIR%\%CMAKE_CONFIGURATION%\pythonextension.* .
copy %GTEST_LIB_PATH%\%CMAKE_CONFIGURATION%\gtest* .

IF "%PYTHONHOME%"=="" (SET PYTHONHOME=%PACKAGES_ROOT%\python)

SET PATH=%PATH%;%PYTHONHOME%;

pythonextension-test.exe --gtest_output=xml:%ENL_ROOT%\out\TestReport_PythonExtension-test.xml
IF %ERRORLEVEL% NEQ 0 GOTO error
popd

REM Advance arg passed to build-pythonextension-test.cmd
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
