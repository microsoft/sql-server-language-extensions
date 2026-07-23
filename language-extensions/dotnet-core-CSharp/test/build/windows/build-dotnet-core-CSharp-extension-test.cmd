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
CALL :SETUP_VS_ENV
IF ERRORLEVEL 1 EXIT /b 1

SET BUILD_OUTPUT=%DOTNETCORE_CSHARP_EXTENSION_TEST_WORKING_DIR%\%CMAKE_CONFIGURATION%
MKDIR %BUILD_OUTPUT%
PUSHD %BUILD_OUTPUT%

ECHO "[INFO] Generating dotnet-core-CSharp-extension test project build files using CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%"

REM Call cmake
REM
CALL "%VS_CMAKE_EXE%" ^
	-G "%VS_CMAKE_GENERATOR%" ^
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
CALL "%VS_CMAKE_EXE%" --build . --config %CMAKE_CONFIGURATION% --target INSTALL
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

REM Locate a Visual Studio install (any version with the C++ toolset) via vswhere,
REM initialize its developer environment, and select a matching CMake generator plus a
REM CMake new enough to know it. Using vswhere keeps the build working across the OneBranch
REM container images regardless of the Visual Studio version they ship (e.g. VS 2022 on the
REM ltsc2022 image) instead of a hard-coded VS 2019 install path and generator.
REM
:SETUP_VS_ENV
	REM NOTE: Visual Studio install paths contain "(x86)"; the ")" breaks parenthesized IF
	REM blocks, so this routine deliberately uses single-line IF statements and GOTO labels.
	SET "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
	IF NOT EXIST "%VSWHERE%" GOTO :SETUP_VS_ENV_NOVSWHERE
	SET "VSINSTALLPATH="
	FOR /F "usebackq tokens=*" %%i IN (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) DO SET "VSINSTALLPATH=%%i"
	IF NOT DEFINED VSINSTALLPATH GOTO :SETUP_VS_ENV_NOVS
	REM Only initialize the developer environment once; VsDevCmd keeps appending to PATH.
	IF NOT DEFINED DevEnvDir CALL "%VSINSTALLPATH%\Common7\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64
	REM Map the installed VS major version to the matching CMake generator name.
	SET "VS_MAJOR="
	FOR /F "usebackq tokens=1 delims=." %%v IN (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationVersion`) DO SET "VS_MAJOR=%%v"
	IF "%VS_MAJOR%"=="17" SET "VS_CMAKE_GENERATOR=Visual Studio 17 2022"
	IF "%VS_MAJOR%"=="16" SET "VS_CMAKE_GENERATOR=Visual Studio 16 2019"
	IF NOT DEFINED VS_CMAKE_GENERATOR GOTO :SETUP_VS_ENV_BADVER
	REM Prefer the CMake bundled with Visual Studio: it knows the "Visual Studio 17 2022"
	REM generator, whereas the packaged CMake (CMAKE_ROOT) is pinned to 3.15 and only knows
	REM up to the VS 2019 generator. Fall back to CMAKE_ROOT when the VS-bundled CMake is
	REM absent (e.g. an older VS install without the C++ CMake tools component).
	SET "VS_CMAKE_EXE="
	FOR /F "usebackq tokens=*" %%c IN (`"%VSWHERE%" -latest -products * -find "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"`) DO SET "VS_CMAKE_EXE=%%c"
	IF NOT DEFINED VS_CMAKE_EXE SET "VS_CMAKE_EXE=%CMAKE_ROOT%\bin\cmake.exe"
	EXIT /b 0
:SETUP_VS_ENV_NOVSWHERE
	ECHO [Error] vswhere.exe not found. Cannot locate Visual Studio.
	EXIT /b 1
:SETUP_VS_ENV_NOVS
	ECHO [Error] No Visual Studio installation with the C++ toolset was found.
	EXIT /b 1
:SETUP_VS_ENV_BADVER
	ECHO [Error] Unsupported Visual Studio version '%VS_MAJOR%' for CMake generator selection.
	EXIT /b 1
