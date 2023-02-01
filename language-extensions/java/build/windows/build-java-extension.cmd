@ECHO off
SETLOCAL

REM Nuget packages directory and location of the JDK
REM
SET ENL_ROOT=%~dp0..\..\..\..
SET JAVAEXTENSION_WORKING_DIR=%ENL_ROOT%\build-output\java-extension\windows
SET JAVAEXTENSION_HOME=%ENL_ROOT%\language-extensions\java

SET DEFAULT_CMAKE_ROOT=%ENL_ROOT%\packages\CMake-win64.3.15.5
SET DEFAULT_JAVA_HOME=%ENL_ROOT%\packages\jdk-17.0.5+8

REM Find JAVA_HOME and CMAKE_ROOT from user, or set to default for tests.
REM Error code 203 is ENVVAR_NOT_FOUND.
REM
SET ENVVAR_NOT_FOUND=203

IF "%JAVA_HOME%" == "" (
	IF EXIST %DEFAULT_JAVA_HOME% (
		SET JAVA_HOME=%DEFAULT_JAVA_HOME%
	) ELSE (
		CALL :CHECKERROR %ENVVAR_NOT_FOUND% "Error: JAVA_HOME variable must be set to build the java extension" || EXIT /b %ENVVAR_NOT_FOUND%
	)
)

IF "%CMAKE_ROOT%" == "" (
	IF EXIST %DEFAULT_CMAKE_ROOT% (
		SET CMAKE_ROOT=%DEFAULT_CMAKE_ROOT%
	) ELSE (
		CALL :CHECKERROR %ENVVAR_NOT_FOUND% "Error: CMAKE_ROOT variable must be set to build the java extension" || EXIT /b %ENVVAR_NOT_FOUND%
	)
)

SET JAVA_HOME="%JAVA_HOME%"
SET JAVA_BIN=%JAVA_HOME%\bin

IF EXIST %JAVAEXTENSION_WORKING_DIR% (RMDIR /s /q %JAVAEXTENSION_WORKING_DIR%)
MKDIR %JAVAEXTENSION_WORKING_DIR%

:LOOP

REM Set cmake config to first arg
REM
SET CMAKE_CONFIGURATION=%1

REM Setting CMAKE_CONFIGURATION to anything but "debug" will set CMAKE_CONFIGURATION to "release".
REM The string comparison for CMAKE_CONFIGURATION is case-insensitive.
REM
IF NOT DEFINED CMAKE_CONFIGURATION (SET CMAKE_CONFIGURATION=release)
IF /I NOT %CMAKE_CONFIGURATION%==debug (SET CMAKE_CONFIGURATION=release)

REM Output directory and output JAR name
REM
SET TARGET=%ENL_ROOT%\build-output\java-extension\target\%CMAKE_CONFIGURATION%
SET TARGET_CLASSES=%TARGET%\classes
SET OUTPUT_JAR=%TARGET%\mssql-java-lang-extension.jar

SET JAVA_PATH="%ENL_ROOT%\language-extensions\java\sdk\src\java\main\java\com\microsoft\sqlserver\javalangextension\"

REM Remove existing output files
REM
IF EXIST %TARGET% (RMDIR /s /q %TARGET%)
IF EXIST %TARGET_CLASSES% (RMDIR /s /q %TARGET_CLASSES%)
IF EXIST %OUTPUT_JAR% (RM /s /q %OUTPUT_JAR%)

REM Create the output directories
REM
mkdir %TARGET%
mkdir %TARGET_CLASSES%

IF NOT EXIST %JAVA_BIN% ECHO %JAVA_BIN% does not exist.
IF NOT EXIST %TARGET% ECHO %TARGET% does not exist.
IF NOT EXIST %TARGET_CLASSES% ECHO %TARGET_CLASSES% does not exist.
IF NOT EXIST %JAVA_PATH% ECHO %JAVA_PATH% does not exist.

REM Save current location, while moving to the Java source directory
REM
pushd %JAVA_PATH%

REM Write all of the sources to file for compilation
REM
dir /s /B *.java > %TARGET%\sources.txt

REM Compile all the Java sources
REM
ECHO %JAVA_HOME% is used for compilation
%JAVA_BIN%\javac -d %TARGET_CLASSES% @%TARGET%\sources.txt

REM Restore the working directory
REM
popd

CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to compile Java source files for mssql-java-lang-extension" || EXIT /b %ERRORLEVEL%

REM Create the mssql-java-lang-extension.jar file
REM
%JAVA_BIN%\jar cvf %OUTPUT_JAR% -C %TARGET_CLASSES% .

CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to create package for mssql-java-lang-extension.jar" || EXIT /b %ERRORLEVEL%

REM VSCMD_START_DIR set the working directory to this variable after calling VsDevCmd.bat
REM otherwise, it will default to %USERPROFILE%\Source
REM
SET VSCMD_START_DIR=%ENL_ROOT%

REM Do not call VsDevCmd if the environment is already set. Otherwise, it will keep appending
REM to the PATH environment variable and it will be too long for windows to handle.
REM
IF NOT DEFINED DevEnvDir (
	call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64
)

ECHO "[INFO] Generating Java extension project build files using CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%"

SET BUILD_OUTPUT=%JAVAEXTENSION_WORKING_DIR%\%CMAKE_CONFIGURATION%
MKDIR %BUILD_OUTPUT%
PUSHD %BUILD_OUTPUT%

REM Call cmake
REM
CALL "%CMAKE_ROOT%\bin\cmake.exe" ^
	-G "Visual Studio 16 2019" ^
	-DCMAKE_BUILD_TYPE=%CMAKE_CONFIGURATION% ^
	-DCMAKE_INSTALL_PREFIX:PATH="%JAVAEXTENSION_WORKING_DIR%" ^
	-DENL_ROOT=%ENL_ROOT% ^
	-DJAVA_HOME=%JAVA_HOME% ^
	-DPLATFORM=Windows ^
	%JAVAEXTENSION_HOME%\src

CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to generate make files for CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%" || EXIT /b %ERRORLEVEL%

ECHO "[INFO] Building Java extension project using CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%"

REM Call cmake build
REM
CALL "%CMAKE_ROOT%\bin\cmake.exe" --build . --config %CMAKE_CONFIGURATION% --target INSTALL

CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to build Java extension for CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%" || EXIT /b %ERRORLEVEL%

REM Copy DLL, LIB, etc files out of debug/debug and release/release into the build output folder
REM
copy %BUILD_OUTPUT%\%CMAKE_CONFIGURATION%\* %BUILD_OUTPUT%\

REM This will create the Java extension package with unsigned binaries, this is used for local development and non-release builds. release
REM builds will call create-java-extension-zip.cmd after the binaries have been signed and this will be included in the zip
REM
IF /I %CMAKE_CONFIGURATION%==debug (
	powershell -NoProfile -ExecutionPolicy Unrestricted -Command "Compress-Archive -Force -Path %BUILD_OUTPUT%\javaextension.dll, %BUILD_OUTPUT%\javaextension.pdb -DestinationPath %TARGET%\java-lang-extension.zip"
) ELSE (
	powershell -NoProfile -ExecutionPolicy Unrestricted -Command "Compress-Archive -Force -Path %BUILD_OUTPUT%\javaextension.dll -DestinationPath %TARGET%\java-lang-extension.zip"
)

CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to create zip for Java extension for CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%" || EXIT /b %ERRORLEVEL%

REM Advance arg passed to build-java-extension.cmd
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
