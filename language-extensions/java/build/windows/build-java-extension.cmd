@ECHO off
SETLOCAL

REM Nuget packages directory and location of the JDK
SET ENL_ROOT=%~dp0..\..\..\..
SET PACKAGES_ROOT=%ENL_ROOT%\packages
SET AZUL_PACKAGE=AzulSystems.Zulu.DPG.8.33.0.1
SET JAVA_HOME=%PACKAGES_ROOT%\%AZUL_PACKAGE%\tools
SET JAVA_BIN=%PACKAGES_ROOT%\%AZUL_PACKAGE%\tools\bin

SET JAVAEXTENSION_WORKING_DIR=%ENL_ROOT%\.build\java-extension\windows
SET JAVAEXTENSION_HOME=%ENL_ROOT%\language-extensions\java
SET CMAKE_ROOT=%ENL_ROOT%\packages\CMake-win64.3.15.5
IF EXIST %JAVAEXTENSION_WORKING_DIR% (RMDIR /s /q %JAVAEXTENSION_WORKING_DIR%)
MKDIR %JAVAEXTENSION_WORKING_DIR%
	
:LOOP

REM Set cmake config to first arg
SET CMAKE_CONFIGURATION=%1

REM *Setting CMAKE_CONFIGURATION to anything but "debug" will set CMAKE_CONFIGURATION to "release".
REM The string comparison for CMAKE_CONFIGURATION is case-insensitive.
IF NOT DEFINED CMAKE_CONFIGURATION (SET CMAKE_CONFIGURATION=debug)
IF /I NOT %CMAKE_CONFIGURATION%==debug (SET CMAKE_CONFIGURATION=release)

REM Output directory and output JAR name
SET TARGET=%ENL_ROOT%\.build\java-extension\target\%CMAKE_CONFIGURATION%
SET TARGET_CLASSES=%TARGET%\classes
SET OUTPUT_JAR=%TARGET%\mssql-java-lang-extension.jar

SET JAVA_PATH="%ENL_ROOT%\language-extensions\java\sdk\src\java\main\java\com\microsoft\sqlserver\javalangextension\"

REM Remove existing output files
IF EXIST %TARGET% (RMDIR /s /q %TARGET%)
IF EXIST %TARGET_CLASSES% (RMDIR /s /q %TARGET_CLASSES%)
IF EXIST %OUTPUT_JAR% (RM /s /q %OUTPUT_JAR%)

REM Create the output directories
mkdir %TARGET%
mkdir %TARGET_CLASSES%

IF NOT EXIST %JAVA_BIN% ECHO %JAVA_BIN% does not exist.
IF NOT EXIST %TARGET% ECHO %TARGET% does not exist.
IF NOT EXIST %TARGET_CLASSES% ECHO %TARGET_CLASSES% does not exist.
IF NOT EXIST %JAVA_PATH% ECHO %JAVA_PATH% does not exist.

REM Save current location, while moving to the Java source directory
pushd %JAVA_PATH%

REM Write all of the sources to file for compilation
dir /s /B *.java > %TARGET%\sources.txt

REM Compile all the Java sources
%JAVA_BIN%\javac -d %TARGET_CLASSES% @%TARGET%\sources.txt

REM Restore the working directory
popd

CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to compile Java source files for mssql-java-lang-extension" || EXIT /b %ERRORLEVEL%

REM Create the mssql-java-lang-extension.jar file
%JAVA_BIN%\jar cvf %OUTPUT_JAR% -C %TARGET_CLASSES% .

CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to create package for mssql-java-lang-extension.jar" || EXIT /b %ERRORLEVEL%

REM VSCMD_START_DIR set the working directory to this variable after calling VsDevCmd.bat
REM otherwise, it will default to %USERPROFILE%\Source
SET VSCMD_START_DIR=%ENL_ROOT%

REM Do not call VsDevCmd if the environment is already set. Otherwise, it will keep appending
REM to the PATH environment variable and it will be too long for windows to handle.
if not defined DevEnvDir (
	call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64
)


ECHO "[INFO] Generating Java extension project build files using CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%"

SET BUILD_OUTPUT=%JAVAEXTENSION_WORKING_DIR%\%CMAKE_CONFIGURATION%
MKDIR %BUILD_OUTPUT%
PUSHD %BUILD_OUTPUT%

REM Call cmake
CALL "%CMAKE_ROOT%\bin\cmake.exe" ^
	-G "Visual Studio 15 2017 Win64" ^
	-DAZUL_PACKAGE=%AZUL_PACKAGE% ^
	-DCMAKE_BUILD_TYPE=%CMAKE_CONFIGURATION% ^
	-DCMAKE_INSTALL_PREFIX:PATH="%JAVAEXTENSION_WORKING_DIR%" ^
	-DENL_ROOT=%ENL_ROOT% ^
	-DJAVA_HOME=%JAVA_HOME% ^
	-DPLATFORM=Windows ^
	%JAVAEXTENSION_HOME%\src
	
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to generate make files for CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%" || EXIT /b %ERRORLEVEL%

ECHO "[INFO] Building Java extension project using CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%"

REM Call cmake build
CALL "%CMAKE_ROOT%\bin\cmake.exe" --build . --config %CMAKE_CONFIGURATION% --target INSTALL
				
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to build Java extension for CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%" || EXIT /b %ERRORLEVEL%

REM This will create the Java extension package with unsigned binaries, this is used for local development and non-release builds. release
REM builds will call create-java-extension-zip.cmd after the binaries have been signed and this will be included in the zip
IF /I %CMAKE_CONFIGURATION%==debug (
	powershell -NoProfile -ExecutionPolicy Unrestricted -Command "Compress-Archive -Force -Path %BUILD_OUTPUT%\%CMAKE_CONFIGURATION%\javaextension.dll, %BUILD_OUTPUT%\%CMAKE_CONFIGURATION%\javaextension.pdb -DestinationPath %TARGET%\java-lang-extension.zip"
) ELSE (
	powershell -NoProfile -ExecutionPolicy Unrestricted -Command "Compress-Archive -Force -Path %BUILD_OUTPUT%\%CMAKE_CONFIGURATION%\javaextension.dll -DestinationPath %TARGET%\java-lang-extension.zip"
)

CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to create zip for Java extension for CMAKE_CONFIGURATION=%CMAKE_CONFIGURATION%" || EXIT /b %ERRORLEVEL%

REM Advance arg passed to build-java-extension.cmd
SHIFT

REM Continue building using more configs until argv has been exhausted
IF NOT "%~1"=="" GOTO LOOP

EXIT /b %ERRORLEVEL%

:CHECKERROR
	IF %1 NEQ 0 (
		ECHO %2
		EXIT /b %1
	)

	EXIT /b 0