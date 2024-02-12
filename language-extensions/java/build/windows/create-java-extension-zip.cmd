@ECHO off
SETLOCAL

SET ENL_ROOT=%~dp0..\..\..\..

:LOOP

REM Set cmake config to first arg
REM
SET BUILD_CONFIGURATION=%1

REM Setting BUILD_CONFIGURATION to anything but "debug" will set BUILD_CONFIGURATION to "release".
REM The string comparison for BUILD_CONFIGURATION is case-insensitive.
REM
IF NOT DEFINED BUILD_CONFIGURATION (SET BUILD_CONFIGURATION=release)
IF /I NOT %BUILD_CONFIGURATION%==debug (SET BUILD_CONFIGURATION=release)

SET BUILD_OUTPUT=%ENL_ROOT%\build-output\java-extension\windows\%BUILD_CONFIGURATION%

mkdir %BUILD_OUTPUT%\packages

REM Set common files to be included in the zip
SET INCLUDE_FILES=%BUILD_OUTPUT%\javaextension.dll

REM Check if BUILD_CONFIGURATION is debug, then include javaextension.pdb in the zip
IF /I "%BUILD_CONFIGURATION%"=="debug" (
    SET INCLUDE_FILES=%INCLUDE_FILES%, %BUILD_OUTPUT%\javaextension.pdb
)

powershell -NoProfile -ExecutionPolicy Unrestricted -Command "Compress-Archive -Path %INCLUDE_FILES% -DestinationPath %BUILD_OUTPUT%\packages\java-lang-extension.zip -Force"
CALL :CHECK_BUILD_ERROR %ERRORLEVEL% %BUILD_CONFIGURATION%

REM Advance arg passed to create-java-extension.cmd
REM
SHIFT

REM Continue building using more configs until argv has been exhausted
REM
IF NOT "%~1"=="" GOTO LOOP

EXIT /b %ERRORLEVEL%

:CHECK_BUILD_ERROR
    IF %1 == 0 (
        ECHO Success: Created zip for %2 config
    ) ELSE (
        ECHO Error: Failed to create zip for %2 config
        EXIT /b %1
    )
