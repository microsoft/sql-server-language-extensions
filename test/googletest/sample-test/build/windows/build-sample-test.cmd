@ECHO off
SETLOCAL

REM Set environment variables
REM
SET EnlRoot=%~dp0..\..\..\..\..\
SET SAMPLETEST_HOME=%EnlRoot%test\gtest\sample-test\
SET SAMPLETEST_BUILD=%EnlRoot%test\gtest\sample-test\build\

:LOOP

REM Set cmake config to first arg
REM
SET CONFIGURATION=%1

REM *Setting CONFIGURATION to anything but "debug" will set MSVC_BUILD_CONFIGURATION to "release".
REM
IF NOT DEFINED CONFIGURATION (SET CONFIGURATION=release)
IF /I %CONFIGURATION%==debug (SET MSVC_BUILD_CONFIGURATION=debug) ELSE (SET MSVC_BUILD_CONFIGURATION=release)

REM VSCMD_START_DIR set the working directory to this variable after calling VsDevCmd.bat
REM otherwise, it will default to %USERPROFILE%\Source
REM
SET VSCMD_START_DIR=%EnlRoot%

REM Do not call VsDevCmd if the environment is already set. Otherwise, it will keep appending
REM to the PATH environment variable and it will be too long for windows to handle.
REM
if not defined DevEnvDir (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64
)

ECHO "[INFO] Building sample test project using MSVC_BUILD_CONFIGURATION=%MSVC_BUILD_CONFIGURATION%"

REM Build the project
REM
msbuild %EnlRoot%test\googletest\sample-test\build\windows\sample-test.vcxproj /m /fl1 /fl2 /fl3 ^
  /flp1:logfile=%SAMPLETEST_BUILD%msbuild.err;errorsonly /flp2:logfile=%SAMPLETEST_BUILD%msbuild.wrn;warningsonly ^
  /flp3:logfile=%SAMPLETEST_BUILD%msbuild.log /property:Configuration=%MSVC_BUILD_CONFIGURATION% /property:Platform=x64
IF %ERRORLEVEL% NEQ 0 GOTO CLEANUP

SET EX=%ERRORLEVEL%

if "%EX%" neq "0" (
    echo "Error: Failed to build gtest.vcxproj"
	GOTO CLEANUP
)

REM Advance arg passed to build-gtest.cmd
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
