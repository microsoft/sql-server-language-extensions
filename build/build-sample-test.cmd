SETLOCAL

REM Set environment variables
SET EnlRoot=%~dp0\..\
SET GTEST_HOME=%EnlRoot%test\gtest\
SET SAMPLETEST_HOME=%EnlRoot%test\sample-test\

:LOOP

REM Set cmake config to first arg
SET CMAKE_CONFIGURATION=%1

REM *Setting CMAKE_CONFIGURATION to anything but "debug" will set MSVC_BUILD_CONFIGURATION to "release".
REM     -The string comparison for CMAKE_CONFIGURATION is case-insensitive.
IF NOT DEFINED CMAKE_CONFIGURATION (SET CMAKE_CONFIGURATION=debug)
IF /I %CMAKE_CONFIGURATION%==debug (SET MSVC_BUILD_CONFIGURATION=debug) ELSE (SET MSVC_BUILD_CONFIGURATION=release)

REM VSCMD_START_DIR set the working directory to this variable after calling VsDevCmd.bat
REM otherwise, it will default to %USERPROFILE%\Source
SET VSCMD_START_DIR=%EnlRoot%

REM Do not call VsDevCmd if the environment is already set. Otherwise, it will keep appending
REM to the PATH environment variable and it will be too long for windows to handle.
if not defined DevEnvDir (
    call "F:\Program Files\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64
)

ECHO "[INFO] Building gtest project using CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%, MSVC_BUILD_CONFIGURATION=%MSVC_BUILD_CONFIGURATION%"

REM Build the project
msbuild %EnlRoot%test\sample-test\sample-test.vcxproj /m /fl1 /fl2 /fl3 /flp1:logfile=%SAMPLETEST_HOME%msbuild.err;errorsonly /flp2:logfile=%SAMPLETEST_HOME%msbuild.wrn;warningsonly /flp3:logfile=%SAMPLETEST_HOME%msbuild.log /property:Configuration=%CMAKE_CONFIGURATION% /property:Platform=x64
IF %ERRORLEVEL% NEQ 0 GOTO CLEANUP

SET EX=%ERRORLEVEL%

if "%EX%" neq "0" (
    echo "Error: Failed to build gtest.vcxproj"
	GOTO CLEANUP
)

REM Advance arg passed to build-gtest.cmd
SHIFT

REM Continue building using more configs until argv has been exhausted
IF NOT "%~1"=="" GOTO LOOP

REM Save exit code of compiler
SET EX=%ERRORLEVEL%

:CLEANUP

if "%EX%" neq "0" (
    echo "Build failed"
)

EXIT /b %ERRORLEVEL%
