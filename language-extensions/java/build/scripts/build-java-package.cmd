@ECHO off

REM Nuget packages directory and location of the JDK
SET EnlRoot=%~dp0..\..\..\..\
SET PACKAGES_ROOT=%EnlRoot%packages
SET AZUL_PACKAGE=AzulSystems.Zulu.DPG.8.33.0.1
SET JAVA_BIN=%PACKAGES_ROOT%\%AZUL_PACKAGE%\tools\bin

REM Output directory and output JAR name
SET TARGET="%EnlRoot%.build\java-extension\target"
SET TARGET_CLASSES=%TARGET%\classes
SET OUTPUT_JAR=mssql-java-lang-extension.jar

REM Create the output directory
mkdir %TARGET_CLASSES%

REM Save current location, while moving to the Java source directory
pushd "%EnlRoot%language-extensions\java\sdk\src\java\main\java"

REM Write all of the sources to file for compilation
dir /s /B *.java > %TARGET%\sources.txt

REM Compile all the Java sources
%JAVA_BIN%\javac -d %TARGET_CLASSES% @%TARGET%\sources.txt

REM Save exit code of compiler
SET EX=%ERRORLEVEL%

if "%EX%" neq "0" (
    echo "Error: Failed to compile Java source files for mssql-java-lang-extension"
	GOTO CLEANUP
)

REM Create the mssql-java-lang-extension.jar file
%JAVA_BIN%\jar cvf %TARGET%\%OUTPUT_JAR% -C %TARGET_CLASSES% .

REM Save exit code of packaging
SET EX=%ERRORLEVEL%

if "%EX%" neq "0" (
    echo "Error: Failed to create package for mssql-java-lang-extension.jar"
	GOTO CLEANUP
)

REM Save exit code of compiler
SET EX=%ERRORLEVEL%

REM Check the exit code of the compiler and exit appropriately so that build will fail.

:CLEANUP

if "%EX%" neq "0" (
    echo "Build failed"
)

REM Restore working directory
popd

REM Return the error from compilation
exit /B %EX%
