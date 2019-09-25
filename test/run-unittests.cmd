@ECHO off
SETLOCAL

:LOOP

IF "%~1"=="" GOTO USAGE

REM Set cmake config to first arg
SET MSVC_BUILD_CONFIGURATION=%1

IF NOT DEFINED CDP_USER_SOURCE_FOLDER_CONTAINER_PATH (SET EnlistmentRoot=%~dp0..\) ELSE (SET EnlistmentRoot=%CDP_USER_SOURCE_FOLDER_CONTAINER_PATH%\)

pushd %EnlistmentRoot%.build\sample-test\x64\%MSVC_BUILD_CONFIGURATION%
sample-test.exe --gtest_output=xml:%EnlistmentRoot%out\TestReport_sample-test.xml
popd
IF %ERRORLEVEL% NEQ 0 GOTO error

REM Advance arg passed to build_all.cmd
SHIFT

REM Continue building using more configs until argv has been exhausted
IF NOT "%~1"=="" GOTO LOOP

:error
EXIT /b %ERRORLEVEL%

:USAGE
echo.
echo Usage:
echo %0 { debug ^| release }
