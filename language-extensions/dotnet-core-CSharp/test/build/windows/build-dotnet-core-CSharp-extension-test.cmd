@ECHO off
SETLOCAL

REM Set environment variables
REM
SET ENL_ROOT=%~dp0..\..\..\..\..
SET DOTNETCORE_CSHARP_EXTENSION_TEST_HOME=%ENL_ROOT%\language-extensions\dotnet-core-CSharp\test
SET DOTNETCORE_CSHARP_EXTENSION_TEST_LIB=%DOTNETCORE_CSHARP_EXTENSION_TEST_HOME%\lib
SET DOTNETCORE_CSHARP_EXTENSION_TEST_WORKING_DIR=%ENL_ROOT%\build-output\dotnet-core-CSharp-extension-test\windows
SET PACKAGES_ROOT=%ENL_ROOT%\packages
SET DEFAULT_CMAKE_ROOT=%PACKAGES_ROOT%\CMake-win64.3.15.5

REM Find CMAKE_ROOT from user, or set to default.
REM Error code 203 is ENVVAR_NOT_FOUND.
REM
SET ENVVAR_NOT_FOUND=203

IF "%CMAKE_ROOT%" == "" (
	IF EXIST %DEFAULT_CMAKE_ROOT% (
		SET CMAKE_ROOT=%DEFAULT_CMAKE_ROOT%
	) ELSE (
		CALL :CHECKERROR %ENVVAR_NOT_FOUND% "Error: CMAKE_ROOT variable must be set to build dotnet-core-CSharp-extension-test" || EXIT /b %ENVVAR_NOT_FOUND%
	)
)

REM Create build working directory
REM
RMDIR /s /q %DOTNETCORE_CSHARP_EXTENSION_TEST_WORKING_DIR%
MKDIR %DOTNETCORE_CSHARP_EXTENSION_TEST_WORKING_DIR%

:LOOP

REM Set cmake config to first arg
REM
SET CMAKE_CONFIGURATION=%1

REM This string comparison for CMAKE_CONFIGURATION is case-insensitive.
REM
IF NOT DEFINED CMAKE_CONFIGURATION (SET CMAKE_CONFIGURATION=release)
IF /I NOT %CMAKE_CONFIGURATION%==debug (SET CMAKE_CONFIGURATION=release)

REM VSCMD_START_DIR set the working directory to this variable after calling VsDevCmd.bat
REM otherwise, it will default to %USERPROFILE%\Source
REM
SET VSCMD_START_DIR=%DOTNETCORE_CSHARP_EXTENSION_TEST_WORKING_DIR%

REM Do not call VsDevCmd if the environment is already set. Otherwise, it will keep appending
REM to the PATH environment variable and it will be too long for windows to handle.
REM
IF NOT DEFINED DevEnvDir (
	CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64
)

SET BUILD_OUTPUT=%DOTNETCORE_CSHARP_EXTENSION_TEST_WORKING_DIR%\%CMAKE_CONFIGURATION%
MKDIR %BUILD_OUTPUT%
PUSHD %BUILD_OUTPUT%

ECHO "[INFO] Generating dotnet-core-CSharp-extension test project build files using CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%"

REM Call cmake
REM
CALL "%CMAKE_ROOT%\bin\cmake.exe" ^
	-G "Visual Studio 16 2019" ^
	-DCMAKE_INSTALL_PREFIX:PATH="%DOTNETCORE_CSHARP_EXTENSION_TEST_WORKING_DIR%\\%CMAKE_CONFIGURATION%" ^
	-DENL_ROOT="%ENL_ROOT%" ^
	-DCMAKE_CONFIGURATION=%CMAKE_CONFIGURATION% ^
	-DPLATFORM=windows ^
	"%DOTNETCORE_CSHARP_EXTENSION_TEST_HOME%\src\native"
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to generate make files for CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%" || EXIT /b %ERRORLEVEL%

ECHO "[INFO] Building dotnet-core-CSharp-extension test project using CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%"

REM Call dotnet build
REM
dotnet build %DOTNETCORE_CSHARP_EXTENSION_TEST_HOME%\src\managed\Microsoft.SqlServer.CSharpExtensionTest.csproj /m -c %CMAKE_CONFIGURATION% -o %BUILD_OUTPUT% --no-dependencies

REM Delete Microsoft.SqlServer.CSharpExtension.dll to avoid test executor referencing it instead of the extension itself
REM
DEL /S /Q %BUILD_OUTPUT%\Microsoft.SqlServer.CSharpExtension.dll


REM Call cmake build
REM
CALL "%CMAKE_ROOT%\bin\cmake.exe" --build . --config %CMAKE_CONFIGURATION% --target INSTALL
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to build dotnet-core-CSharp-extension-test for CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%" || EXIT /b %ERRORLEVEL%

REM Advance arg passed to build-dotnet-core-CSharp-extension-test.cmd
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
