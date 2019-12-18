REM @ECHO off
SETLOCAL

REM Set environment variables
SET ENL_ROOT=%~dp0..\..\..\..\..\
SET PYTHONEXTENSIONTEST_WORKING_DIR=%ENL_ROOT%.build\pythonextension-test\x64\
SET PACKAGES_ROOT=%ENL_ROOT%packages\
SET PYTHONHOME=%PACKAGES_ROOT%python
SET PATH=%PATH%;%PYTHONHOME%

:LOOP

IF "%~1"=="" GOTO USAGE

REM Set cmake config to first arg
SET MSVC_BUILD_CONFIGURATION=%1

pushd %PYTHONEXTENSIONTEST_WORKING_DIR%%MSVC_BUILD_CONFIGURATION%
pythonextension-test.exe --gtest_output=xml:%EnlistmentRoot%out\TestReport_pythonextension-test.xml
IF %ERRORLEVEL% NEQ 0 GOTO error
popd

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
