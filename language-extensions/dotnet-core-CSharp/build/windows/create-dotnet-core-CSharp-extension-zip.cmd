@ECHO off
SETLOCAL

SET ENL_ROOT=%~dp0..\..\..\..
SET DOTNET_EXTENSION_WORKING_DIR=%ENL_ROOT%\build-output\dotnet-core-CSharp-extension\windows

:LOOP

REM Set build config to first arg
REM
SET BUILD_CONFIGURATION=%1

REM Setting BUILD_CONFIGURATION to anything but "debug" will set BUILD_CONFIGURATION to "release".
REM The string comparison for BUILD_CONFIGURATION is case-insensitive.
REM
IF NOT DEFINED BUILD_CONFIGURATION (SET BUILD_CONFIGURATION=release)
IF /I NOT %BUILD_CONFIGURATION%==debug (SET BUILD_CONFIGURATION=release)

SET BUILD_OUTPUT=%DOTNET_EXTENSION_WORKING_DIR%\%BUILD_CONFIGURATION%

MKDIR %BUILD_OUTPUT%\packages

REM Delete the ref folder so that the zip can be loaded by the SPEES
REM
RD /S /Q %BUILD_OUTPUT%\ref
POPD

REM Define files to compress, conditionally including .pdb files if BUILD_CONFIGURATION is "debug"
REM
SET FILES_TO_COMPRESS=%BUILD_OUTPUT%\*.dll, %BUILD_OUTPUT%\Microsoft.SqlServer.CSharpExtension.runtimeconfig.json, %BUILD_OUTPUT%\Microsoft.SqlServer.CSharpExtension.deps.json
IF /I "%BUILD_CONFIGURATION%"=="debug" (SET FILES_TO_COMPRESS=%FILES_TO_COMPRESS%, %BUILD_OUTPUT%\*.pdb)

REM Package the signed binaries.
REM
powershell -NoProfile -ExecutionPolicy Unrestricted ^
 -Command "Compress-Archive -Force -Path %FILES_TO_COMPRESS%" ^
 "-DestinationPath %BUILD_OUTPUT%\packages\dotnet-core-CSharp-lang-extension.zip"
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to create zip for dotnet-core-CSharp-extension for configuration=%BUILD_CONFIGURATION%" || EXIT /b %ERRORLEVEL%

ECHO "Success: Compressed dotnet-core-CSharp-extension for %BUILD_CONFIGURATION% configuration."

REM Advance arg passed to create-dotnet-core-CSharp-extension-zip.cmd
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
