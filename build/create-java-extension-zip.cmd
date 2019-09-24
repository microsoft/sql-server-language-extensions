@ECHO off
SETLOCAL

SET EnlRoot=%~dp0..\

SET MSVC_BUILD_CONFIGURATION=debug
SET BUILD_OUTPUT=%EnlRoot%\language-extensions\java\build\windows\x64\%MSVC_BUILD_CONFIGURATION%

SET TARGET="%~dp0..\.build\java-extension\target\%MSVC_BUILD_CONFIGURATION%"
SET OUTPUT_JAR=%TARGET%\mssql-java-lang-extension.jar

mkdir %BUILD_OUTPUT%\packages
powershell -NoProfile -ExecutionPolicy Unrestricted -Command "Compress-Archive -Path %BUILD_OUTPUT%\javaextension.dll, %OUTPUT_JAR% -DestinationPath %BUILD_OUTPUT%\packages\java-lang-extension.zip"

SET MSVC_BUILD_CONFIGURATION=release
SET BUILD_OUTPUT=%EnlRoot%\language-extensions\java\build\windows\x64\%MSVC_BUILD_CONFIGURATION%

SET TARGET="%~dp0..\.build\java-extension\target\%MSVC_BUILD_CONFIGURATION%"
SET OUTPUT_JAR=%TARGET%\mssql-java-lang-extension.jar

mkdir %BUILD_OUTPUT%\packages
powershell -NoProfile -ExecutionPolicy Unrestricted -Command "Compress-Archive -Path %BUILD_OUTPUT%\javaextension.dll, %OUTPUT_JAR% -DestinationPath %BUILD_OUTPUT%\packages\java-lang-extension.zip"

EXIT /b %ERRORLEVEL%