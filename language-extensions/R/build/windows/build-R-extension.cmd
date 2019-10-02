@ECHO off
SETLOCAL

REM Nuget packages directory and location of R libs
SET EnlRoot=%~dp0..\..\..\..\
SET REXTENSION_HOME=%EnlRoot%language-extensions\R\

:LOOP

REM Set cmake config to first arg
SET CMAKE_CONFIGURATION=%1

REM *Setting CMAKE_CONFIGURATION to anything but "debug" will set MSVC_BUILD_CONFIGURATION to "relwithdebinfo".
REM The string comparison for CMAKE_CONFIGURATION is case-insensitive.
IF NOT DEFINED CMAKE_CONFIGURATION (SET CMAKE_CONFIGURATION=debug)
IF /I %CMAKE_CONFIGURATION%==debug (SET MSVC_BUILD_CONFIGURATION=debug) ELSE (SET MSVC_BUILD_CONFIGURATION=relwithdebinfo)

REM Output directory and output dll name
SET TARGET="%EnlRoot%.build\R-extension\target\%MSVC_BUILD_CONFIGURATION%"

REM Create the output directories
mkdir %TARGET%

REM VSCMD_START_DIR set the working directory to this variable after calling VsDevCmd.bat
REM otherwise, it will default to %USERPROFILE%\Source
SET VSCMD_START_DIR=%EnlRoot%

REM Do not call VsDevCmd if the environment is already set. Otherwise, it will keep appending
REM to the PATH environment variable and it will be too long for windows to handle.
if not defined DevEnvDir (
    call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64
)

REM Build the project
msbuild %REXTENSION_HOME%build\windows\Rextension.vcxproj /m /property:Configuration=%MSVC_BUILD_CONFIGURATION% /property:Platform=x64

REM Save exit code of compiler
SET EX=%ERRORLEVEL%

REM Check the exit code of the compiler and exit appropriately so that build will fail.
IF %EX% NEQ 0 (
    echo "Error: Failed to build R extension"
    GOTO CLEANUP
)

SET BUILD_OUTPUT=%EnlRoot%.build\R-extension\windows\%MSVC_BUILD_CONFIGURATION%

REM This will create the R extension package with unsigned binaries, this is used for local development and non-release builds. Release
REM builds will call create-R-extension-zip.cmd after the binaries have been signed and this will be included in the zip
powershell -NoProfile -ExecutionPolicy Unrestricted -Command "Compress-Archive -Force -Path %BUILD_OUTPUT%\Rextension.dll, %BUILD_OUTPUT%\Rextension.pdb -DestinationPath %TARGET%\R-lang-extension.zip"

SET EX=%ERRORLEVEL%

if "%EX%" neq "0" (
    echo "Error: Failed to create zip for R extension"
    GOTO CLEANUP
)

REM Advance arg passed to build-R-extension.cmd
SHIFT

REM Continue building using more configs until argv has been exhausted
IF NOT "%~1"=="" GOTO LOOP

:CLEANUP

if "%EX%" neq "0" (
    echo "Build failed"
)

EXIT /b %ERRORLEVEL%
