@ECHO off
SETLOCAL

SET ENL_ROOT=%~dp0..\..\..\..

SET MSVC_BUILD_CONFIGURATION=debug
SET BUILD_OUTPUT=%ENL_ROOT%\.build\java-extension\windows\%MSVC_BUILD_CONFIGURATION%\%MSVC_BUILD_CONFIGURATION%
SET OUTPUT_JAR="%ENL_ROOT%\.build\java-extension\target\%MSVC_BUILD_CONFIGURATION%\mssql-java-lang-extension.jar"

mkdir %BUILD_OUTPUT%\packages
powershell -NoProfile -ExecutionPolicy Unrestricted -Command "Compress-Archive -Path %BUILD_OUTPUT%\javaextension.dll, %OUTPUT_JAR% -DestinationPath %BUILD_OUTPUT%\packages\java-lang-extension.zip"

SET MSVC_BUILD_CONFIGURATION=release
SET BUILD_OUTPUT=%ENL_ROOT%\.build\java-extension\windows\%MSVC_BUILD_CONFIGURATION%\%MSVC_BUILD_CONFIGURATION%
SET OUTPUT_JAR="%ENL_ROOT%\.build\java-extension\target\%MSVC_BUILD_CONFIGURATION%\mssql-java-lang-extension.jar"

mkdir %BUILD_OUTPUT%\packages
powershell -NoProfile -ExecutionPolicy Unrestricted -Command "Compress-Archive -Path %BUILD_OUTPUT%\javaextension.dll, %OUTPUT_JAR% -DestinationPath %BUILD_OUTPUT%\packages\java-lang-extension.zip"

EXIT /b %ERRORLEVEL%