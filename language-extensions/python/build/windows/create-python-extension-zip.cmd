@ECHO off
SETLOCAL

SET EnlRoot=%~dp0\..\..\..\..

SET MSVC_BUILD_CONFIGURATION=debug
SET BUILD_OUTPUT=%EnlRoot%\.build\python-extension\windows\%MSVC_BUILD_CONFIGURATION%

mkdir %BUILD_OUTPUT%\packages
powershell -NoProfile -ExecutionPolicy Unrestricted -Command "Compress-Archive -Path %BUILD_OUTPUT%\pythonextension.dll -DestinationPath %BUILD_OUTPUT%\packages\python-lang-extension.zip"

SET MSVC_BUILD_CONFIGURATION=release
SET BUILD_OUTPUT=%EnlRoot%\.build\python-extension\windows\%MSVC_BUILD_CONFIGURATION%

mkdir %BUILD_OUTPUT%\packages
powershell -NoProfile -ExecutionPolicy Unrestricted -Command "Compress-Archive -Path %BUILD_OUTPUT%\pythonextension.dll -DestinationPath %BUILD_OUTPUT%\packages\python-lang-extension.zip"

EXIT /b %ERRORLEVEL%
