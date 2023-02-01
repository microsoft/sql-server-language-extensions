@ECHO off
SETLOCAL

REM Nuget packages directory and location of R libs
REM
SET ENL_ROOT=%~dp0..\..\..\..
SET REXTENSION_HOME=%ENL_ROOT%\language-extensions\R
SET REXTENSION_WORKING_DIR=%ENL_ROOT%\build-output\RExtension\windows
SET PACKAGES_ROOT=%ENL_ROOT%\packages
SET DEFAULT_R_HOME=%PACKAGES_ROOT%\R-4.0.5-win
SET DEFAULT_CMAKE_ROOT=%PACKAGES_ROOT%\CMake-win64.3.15.5
SET DEFAULT_RTOOLS40_HOME=C:\rtools40
SET DEFAULT_RTOOLS35_HOME=C:\rtools

REM Find R_HOME, RTOOLS40_HOME, RTOOLS35_HOME, CMAKE_ROOT from user or set to default.
REM Error code 203 is ENVVAR_NOT_FOUND.
REM
SET ENVVAR_NOT_FOUND=203

IF "%R_HOME%" == "" (
	IF EXIST %DEFAULT_R_HOME% (
		SET R_HOME=%DEFAULT_R_HOME%
	) ELSE (
		CALL :CHECKERROR %ENVVAR_NOT_FOUND% "Error: R_HOME variable must be set to build the RExtension" || EXIT /b %ENVVAR_NOT_FOUND%
	)
)

IF "%RTOOLS40_HOME%" == "" (
	IF EXIST %DEFAULT_RTOOLS40_HOME% (
		SET RTOOLS40_HOME=%DEFAULT_RTOOLS40_HOME%
	) ELSE (
		CALL :CHECKERROR %ENVVAR_NOT_FOUND% "Error: RTOOLS40_HOME variable must be set to build the RExtension" || EXIT /b %ENVVAR_NOT_FOUND%
	)
)

IF "%RTOOLS35_HOME%" == "" (
	IF EXIST %DEFAULT_RTOOLS35_HOME% (
		SET RTOOLS35_HOME=%DEFAULT_RTOOLS35_HOME%
	) ELSE (
		CALL :CHECKERROR %ENVVAR_NOT_FOUND% "Error: RTOOLS35_HOME variable must be set to build the RExtension" || EXIT /b %ENVVAR_NOT_FOUND%
	)
)

IF "%CMAKE_ROOT%" == "" (
	IF EXIST %DEFAULT_CMAKE_ROOT% (
		SET CMAKE_ROOT=%DEFAULT_CMAKE_ROOT%
	) ELSE (
		CALL :CHECKERROR %ENVVAR_NOT_FOUND% "Error: CMAKE_ROOT variable must be set to build the RExtension" || EXIT /b %ENVVAR_NOT_FOUND%
	)
)

IF EXIST %REXTENSION_WORKING_DIR% (RMDIR /s /q %REXTENSION_WORKING_DIR%)
MKDIR %REXTENSION_WORKING_DIR%

:LOOP

REM Set cmake config to first arg
REM
SET CMAKE_CONFIGURATION=%1

REM CMAKE_CONFIGURATION is release by default.
REM
IF NOT DEFINED CMAKE_CONFIGURATION (SET CMAKE_CONFIGURATION=release)
IF /I NOT %CMAKE_CONFIGURATION%==debug (SET CMAKE_CONFIGURATION=release)

REM Output directory and output dll name
REM
SET TARGET="%ENL_ROOT%\build-output\RExtension\target\%CMAKE_CONFIGURATION%"

REM Delete the output directory if exists
REM
IF EXIST %TARGET% (RMDIR /s /q %TARGET%)

REM Create the output directory
REM
MKDIR %TARGET%

ECHO "[INFO] Generating RExtension project build files using CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%"

SET BUILD_OUTPUT=%REXTENSION_WORKING_DIR%\%CMAKE_CONFIGURATION%
MKDIR %BUILD_OUTPUT%
PUSHD %BUILD_OUTPUT%

REM Make sure g++, mingw32-make and R.dll are in the PATH.
REM Do not enclose the C:\Rtools\mingw_64\bin path in quotes - cmake test fails
REM
SET PATH=%RTOOLS40_HOME%\mingw64\bin;%RTOOLS35_HOME%\mingw_64\bin;%R_HOME%\bin\x64;%PATH%

REM Call cmake
REM
CALL "%CMAKE_ROOT%\bin\cmake.exe" ^
	-G "MinGW Makefiles" ^
	-DCMAKE_INSTALL_PREFIX:PATH="%REXTENSION_WORKING_DIR%\%CMAKE_CONFIGURATION%" ^
	-DR_HOME="%R_HOME%" ^
	-DENL_ROOT="%ENL_ROOT%" ^
	-DCMAKE_MAKE_PROGRAM=mingw32-make ^
	-DCMAKE_CONFIGURATION=%CMAKE_CONFIGURATION% ^
	-DPLATFORM=windows ^
	"%REXTENSION_HOME%\src"
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to generate make files for CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%" || EXIT /b %ERRORLEVEL%

ECHO "[INFO] Building RExtension project using CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%"
REM Call cmake build
REM
CALL "mingw32-make.exe" -j all
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to build RExtension for CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%" || EXIT /b %ERRORLEVEL%

REM This will create the RExtension package with unsigned binaries, this is used for local development and non-release builds. Release
REM builds will call create-RExtension-zip.cmd after the binaries have been signed and this will be included in the zip
REM
powershell -NoProfile -ExecutionPolicy Unrestricted -Command "Compress-Archive -Force -Path %BUILD_OUTPUT%\libRExtension.dll, %BUILD_OUTPUT%\libRExtension.dll.a -DestinationPath %TARGET%\R-lang-extension.zip"
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to create zip for RExtension for CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%" || EXIT /b %ERRORLEVEL%

REM Advance arg passed to build-RExtension.cmd
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
