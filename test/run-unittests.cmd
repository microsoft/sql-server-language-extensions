@ECHO off
SETLOCAL

:LOOP

IF "%~1"=="" GOTO USAGE

REM Set config to first arg
REM
SET MSVC_BUILD_CONFIGURATION=%1

IF NOT DEFINED CDP_USER_SOURCE_FOLDER_CONTAINER_PATH (SET ENL_ROOT=%~dp0..) ELSE (SET ENL_ROOT=%CDP_USER_SOURCE_FOLDER_CONTAINER_PATH%)

pushd %ENL_ROOT%\build-output\sample-test\x64\%MSVC_BUILD_CONFIGURATION%
sample-test.exe --gtest_output=xml:%ENL_ROOT%\out\TestReport_sample-test.xml
IF %ERRORLEVEL% NEQ 0 GOTO error
popd

REM Advance arg passed to this script
REM
SHIFT

REM Continue running using more configs until args have been exhausted
REM
IF NOT "%~1"=="" GOTO LOOP

:error
EXIT /b %ERRORLEVEL%

:USAGE
echo.
echo Usage:
echo %0 { debug ^| release }
