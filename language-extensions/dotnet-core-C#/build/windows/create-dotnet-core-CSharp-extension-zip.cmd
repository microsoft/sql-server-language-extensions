@ECHO off
SETLOCAL

SET ENL_ROOT=%~dp0..\..\..\..
SET DOTNET_EXTENSION_HOME=%ENL_ROOT%\language-extensions\dotnet-core-C#
SET DOTNET_EXTENSION_WORKING_DIR=%ENL_ROOT%\build-output\dotnet-core-C#-extension\windows

REM Set build config to first arg
REM
SET BUILD_CONFIGURATION=%1

REM BUILD_CONFIGURATION is debug by default.
REM
IF NOT DEFINED BUILD_CONFIGURATION (SET BUILD_CONFIGURATION=debug)
IF /I NOT %BUILD_CONFIGURATION%==debug (SET BUILD_CONFIGURATION=release)

REM Output directory and output dll name
REM
SET TARGET="%ENL_ROOT%\build-output\dotnet-core-C#-extension\target\%BUILD_CONFIGURATION%"

SET BUILD_OUTPUT=%DOTNET_EXTENSION_WORKING_DIR%\%BUILD_CONFIGURATION%

REM Delete the ref folder so that the zip can be loaded by the SPEES
RD /S /Q %BUILD_OUTPUT%\ref
POPD

REM This will create the dotnet-core-C#-extension package with unsigned binaries, this is used for local development and non-release builds.
REM Release builds will call create-dotnet-core-C#-extension-zip.cmd after the binaries have been signed and those will be included in the zip.
REM
powershell -NoProfile -ExecutionPolicy Unrestricted -Command "Compress-Archive -Force -Path %BUILD_OUTPUT%\* -DestinationPath %TARGET%\dotnet-core-C#-lang-extension.zip"
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to create zip for dotnet-core-C#-extension for configuration=%BUILD_CONFIGURATION%" || EXIT /b %ERRORLEVEL%

ECHO "Success: Compressed dotnet-core-C#-extension for %BUILD_CONFIGURATION% configuration."

REM REM Advance arg passed to create-dotnet-core-CSharp-extension-zip.cmd
REM
SHIFT
REM Continue building using more configs until argv has been exhausted
REM
IF NOT "%~1"=="" GOTO LOOP

EXIT /b %ERRORLEVEL%

:CHECKERROR
	IF %1 NEQ 0 (
		ECHO %2
		EXIT /b %1
	)
	EXIT /b 0
