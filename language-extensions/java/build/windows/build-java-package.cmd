@ECHO off

REM Set cmake config to first arg
REM
SET CMAKE_CONFIGURATION=%1

REM *Setting CMAKE_CONFIGURATION to anything but "debug" will set CMAKE_CONFIGURATION to "release".
REM
IF NOT DEFINED CMAKE_CONFIGURATION (SET CMAKE_CONFIGURATION=debug) ELSE (SET CMAKE_CONFIGURATION=release)


REM Nuget packages directory and location of the JDK
REM
SET ENL_ROOT=%~dp0..\..\..\..
SET PACKAGES_ROOT=%ENL_ROOT%\packages
SET AZUL_PACKAGE=AzulSystems.Zulu.DPG.8.33.0.1
SET JAVA_BIN=%PACKAGES_ROOT%\%AZUL_PACKAGE%\tools\bin

REM Output directory and output JAR name
REM
SET TARGET=%ENL_ROOT%\.build\java-extension\target\%CMAKE_CONFIGURATION%
SET TARGET_CLASSES=%TARGET%\classes
SET OUTPUT_JAR=mssql-java-lang-extension.jar

REM Create the output directory
REM
mkdir %TARGET%
mkdir %TARGET_CLASSES%

REM Save current location, while moving to the Java source directory
REM
pushd "%ENL_ROOT%\language-extensions\java\sdk\src\java\main\java\com\microsoft\sqlserver\javalangextension\"

REM Write all of the sources to file for compilation
REM
dir /s /B *.java > %TARGET%\sources.txt

REM Compile all the Java sources
REM
%JAVA_BIN%\javac -d %TARGET_CLASSES% @%TARGET%\sources.txt

REM Save exit code of compiler
REM
SET EX=%ERRORLEVEL%

if "%EX%" neq "0" (
	echo "Error: Failed to compile Java source files for mssql-java-lang-extension"
	GOTO CLEANUP
)

REM Create the mssql-java-lang-extension.jar file
REM
%JAVA_BIN%\jar cvf %TARGET%\%OUTPUT_JAR% -C %TARGET_CLASSES% .

REM Save exit code of packaging
REM
SET EX=%ERRORLEVEL%

if "%EX%" neq "0" (
	echo "Error: Failed to create package for mssql-java-lang-extension.jar"
	GOTO CLEANUP
)

REM Save exit code of compiler
REM
SET EX=%ERRORLEVEL%

REM Check the exit code of the compiler and exit appropriately so that build will fail.
REM
:CLEANUP

if "%EX%" neq "0" (
	echo "Build failed"
)

REM Restore working directory
REM
popd

REM Return the error from compilation
REM
exit /B %EX%
