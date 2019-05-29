@ECHO off
SETLOCAL

REM Nuget packages directory and location of the JDK
SET EnlRoot=%~dp0
SET PACKAGES_ROOT="%~dp0packages"
SET AZUL_PACKAGE=AzulSystems.Zulu.DPG.8.33.0.1
SET JAVA_HOME=%PACKAGES_ROOT%\%AZUL_PACKAGE%\tools
SET JAVA_BIN=%PACKAGES_ROOT%\%AZUL_PACKAGE%\tools\bin

:LOOP

REM Set cmake config to first arg
SET CONFIGURATION=%1

REM *Setting CONFIGURATION to anything but "debug" will set MSVC_BUILD_CONFIGURATION to "release".
IF NOT DEFINED CONFIGURATION (SET CONFIGURATION=debug)
IF /I %CONFIGURATION%==debug (SET MSVC_BUILD_CONFIGURATION=debug) ELSE (SET MSVC_BUILD_CONFIGURATION=release)

REM Output directory and output JAR name
SET TARGET="%~dp0.build\java-extension\target\%MSVC_BUILD_CONFIGURATION%"
SET TARGET_CLASSES=%TARGET%\classes
SET OUTPUT_JAR=%TARGET%\mssql-java-lang-extension.jar

REM Create the output directories
mkdir %TARGET%
mkdir %TARGET_CLASSES%

REM Save current location, while moving to the Java source directory
pushd "%~dp0language-extensions\java\sdk\src\java\main\java"

REM Write all of the sources to file for compilation
dir /s /B *.java > %TARGET%\sources.txt

REM Compile all the Java sources
%JAVA_BIN%\javac -d %TARGET_CLASSES% @%TARGET%\sources.txt

REM Restore the working directory
popd

REM Save exit code of compiler
SET EX=%ERRORLEVEL%

if "%EX%" neq "0" (
    echo "Error: Failed to compile Java source files for mssql-java-lang-extension"
	GOTO CLEANUP
)

REM Create the mssql-java-lang-extension.jar file
%JAVA_BIN%\jar cvf %OUTPUT_JAR% -C %TARGET_CLASSES% .

REM Save exit code of packaging
SET EX=%ERRORLEVEL%

if "%EX%" neq "0" (
    echo "Error: Failed to create package for mssql-java-lang-extension.jar"
	GOTO CLEANUP
)

REM VSCMD_START_DIR set the working directory to this variable after calling VsDevCmd.bat
REM otherwise, it will default to %USERPROFILE%\Source
SET VSCMD_START_DIR=%EnlRoot%

REM Do not call VsDevCmd if the environment is already set. Otherwise, it will keep appending
REM to the PATH environment variable and it will be too long for windows to handle.
if not defined DevEnvDir (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64
)

REM Build the project
msbuild %EnlRoot%language-extensions\java\build\windows\javaextension.vcxproj /m /p:JAVA_HOME=%JAVA_HOME% /property:Configuration=%MSVC_BUILD_CONFIGURATION% /property:Platform=x64 
IF %ERRORLEVEL% NEQ 0 GOTO CLEANUP

SET BUILD_OUTPUT=%EnlRoot%language-extensions\java\build\windows\x64\%MSVC_BUILD_CONFIGURATION%

REM This will create the Java extension package with unsigned binaries, this is used for local development and non-release builds. Release
REM builds will call create-java-extension-zip.cmd after the binaries have been signed and this will be included in the zip
powershell -NoProfile -ExecutionPolicy Unrestricted -Command "Compress-Archive -Force -Path %BUILD_OUTPUT%\javaextension.dll, %BUILD_OUTPUT%\javaextension.pdb, %OUTPUT_JAR% -DestinationPath %TARGET%\java-lang-extension.zip"

SET EX=%ERRORLEVEL%

if "%EX%" neq "0" (
    echo "Error: Failed to create zip for java extension"
	GOTO CLEANUP
)

REM Advance arg passed to build_all.cmd
SHIFT

REM Continue building using more configs until argv has been exhausted
IF NOT "%~1"=="" GOTO LOOP

REM Save exit code of compiler
SET EX=%ERRORLEVEL%

REM Check the exit code of the compiler and exit appropriately so that build will fail.

:CLEANUP

if "%EX%" neq "0" (
    echo "Build failed"
)

EXIT /b %ERRORLEVEL%