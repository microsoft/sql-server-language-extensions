@ECHO off
SETLOCAL

REM Set environment variables
REM
SET ENL_ROOT=%~dp0..\..\..\..\..
SET DOTNETCORE_CSHARP_EXTENSION_TEST_WORKING_DIR=%ENL_ROOT%\build-output\dotnet-core-csharp-extension-test\windows
SET PACKAGES_ROOT=%ENL_ROOT%\packages
SET GTEST_HOME=%PACKAGES_ROOT%\Microsoft.googletest.v140.windesktop.msvcstl.dyn.rt-dyn.1.8.1.3
SET GTEST_LIB_PATH=%GTEST_HOME%\lib\native\v140\windesktop\msvcstl\dyn\rt-dyn\x64

:LOOP

REM Set cmake config to first arg
REM
SET CMAKE_CONFIGURATION=%1

REM CMAKE_CONFIGURATION is debug by default if not defined.
REM
IF NOT DEFINED CMAKE_CONFIGURATION (SET CMAKE_CONFIGURATION=debug)

PUSHD %DOTNETCORE_CSHARP_EXTENSION_TEST_WORKING_DIR%\%CMAKE_CONFIGURATION%
COPY %GTEST_LIB_PATH%\%CMAKE_CONFIGURATION%\gtest*.dll .
dotnet-core-CSharp-extension-test.exe --gtest_output=xml:%ENL_ROOT%\out\TestReport_dotnet-core-csharp-extension-test.xml
IF %ERRORLEVEL% NEQ 0 GOTO error
POPD

REM Advance arg passed to run-dotnet-core-csharp-extension-test.cmd
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
