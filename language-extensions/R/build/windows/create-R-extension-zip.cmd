@ECHO off
SETLOCAL

SET EnlRoot=%~dp0..\..\..\..

SET MSVC_BUILD_CONFIGURATION=debug
SET BUILD_OUTPUT=%EnlRoot%\.build\R-extension\windows\%MSVC_BUILD_CONFIGURATION%

mkdir %BUILD_OUTPUT%\packages
powershell -NoProfile -ExecutionPolicy Unrestricted -Command "Compress-Archive -Path %BUILD_OUTPUT%\Rextension.dll -DestinationPath %BUILD_OUTPUT%\packages\R-lang-extension.zip -Force"

SET MSVC_BUILD_CONFIGURATION=release
SET BUILD_OUTPUT=%EnlRoot%\.build\R-extension\windows\%MSVC_BUILD_CONFIGURATION%

mkdir %BUILD_OUTPUT%\packages
powershell -NoProfile -ExecutionPolicy Unrestricted -Command "Compress-Archive -Path %BUILD_OUTPUT%\Rextension.dll -DestinationPath %BUILD_OUTPUT%\packages\R-lang-extension.zip -Force"

EXIT /b %ERRORLEVEL%
