@ECHO off
SETLOCAL

REM
REM
SET ENL_ROOT=%~dp0..\..\..\..
SET DOTNET_EXTENSION_HOME=%ENL_ROOT%\language-extensions\dotnet-core-CSharp
SET DOTNET_EXTENSION_WORKING_DIR=%ENL_ROOT%\build-output\dotnet-core-CSharp-extension\windows

IF EXIST %DOTNET_EXTENSION_WORKING_DIR% (RMDIR /s /q %DOTNET_EXTENSION_WORKING_DIR%)
MKDIR %DOTNET_EXTENSION_WORKING_DIR%

:LOOP

REM Set build config to first arg
REM
SET BUILD_CONFIGURATION=%1

REM Setting BUILD_CONFIGURATION to anything but "debug" will set BUILD_CONFIGURATION to "release".
REM The string comparison for BUILD_CONFIGURATION is case-insensitive.
REM
IF NOT DEFINED BUILD_CONFIGURATION (SET BUILD_CONFIGURATION=release)
IF /I NOT %BUILD_CONFIGURATION%==debug (SET BUILD_CONFIGURATION=release)

REM Output directory and output dll name
REM
SET TARGET="%ENL_ROOT%\build-output\dotnet-core-CSharp-extension\target\%BUILD_CONFIGURATION%"

REM Delete the output directory if exists
REM
IF EXIST %TARGET% (RMDIR /s /q %TARGET%)

REM Create the output directory
REM
MKDIR %TARGET%

REM Do not call VsDevCmd if the environment is already set. Otherwise, it will keep appending
REM to the PATH environment variable and it will be too long for windows to handle.
REM
IF NOT DEFINED DevEnvDir (
	CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64
)

REM VSCMD_START_DIR set the working directory to this variable after calling VsDevCmd.bat
REM otherwise, it will default to %USERPROFILE%\Source
REM
SET VSCMD_START_DIR=%ENL_ROOT%

ECHO "[Info] Building dotnet-core-CSharp-extension nativecsharpextension dll..."

SET BUILD_OUTPUT=%DOTNET_EXTENSION_WORKING_DIR%\%BUILD_CONFIGURATION%
RMDIR /S %BUILD_OUTPUT% /q
MKDIR %BUILD_OUTPUT%
PUSHD %BUILD_OUTPUT%

SET DOTNET_NATIVE_SRC=%DOTNET_EXTENSION_HOME%\src\native
SET DOTNET_NATIVE_INCLUDE=%DOTNET_EXTENSION_HOME%\include
SET EXTENSION_HOST_INCLUDE=%ENL_ROOT%\extension-host\include
SET DOTNET_NATIVE_LIB=%DOTNET_EXTENSION_HOME%\lib

IF /I %BUILD_CONFIGURATION%==debug (
	cl.exe /LD %DOTNET_NATIVE_SRC%\nativecsharpextension.cpp %DOTNET_NATIVE_SRC%\*.cpp /I %DOTNET_NATIVE_INCLUDE% /I %EXTENSION_HOST_INCLUDE% /D WINDOWS /D DEBUG /EHsc /Zi
) ELSE (
	cl.exe /LD %DOTNET_NATIVE_SRC%\nativecsharpextension.cpp %DOTNET_NATIVE_SRC%\*.cpp /I %DOTNET_NATIVE_INCLUDE% /I %EXTENSION_HOST_INCLUDE% /D WINDOWS /EHsc /Zi
)

CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to build nativecsharpextension for configuration=%BUILD_CONFIGURATION%" || EXIT /b %ERRORLEVEL%

ECHO "[Info] Copying dependent libraries..."
XCOPY %DOTNET_NATIVE_LIB%\hostfxr.dll %BUILD_OUTPUT%

ECHO "[Info] Building Microsoft.SqlServer.CSharpExtension dll..."
SET DOTNET_MANAGED_SRC=%DOTNET_EXTENSION_HOME%\src\managed
dotnet build %DOTNET_MANAGED_SRC%\Microsoft.SqlServer.CSharpExtension.csproj /m -c %BUILD_CONFIGURATION% -o %BUILD_OUTPUT% --no-dependencies
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to build for Microsoft.SqlServer.CSharpExtension.dll for configuration=%BUILD_CONFIGURATION%" || EXIT /b %ERRORLEVEL%

ECHO "Success: Built dotnet-core-CSharp-extension for %BUILD_CONFIGURATION% configuration."

REM REM Advance arg passed to build-dotnet-core-CSharp-extension.cmd
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
