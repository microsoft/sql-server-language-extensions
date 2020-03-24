@ECHO off
SETLOCAL

SET ENL_ROOT=%~dp0..\..\..\..

SET MSVC_BUILD_CONFIGURATION=debug
SET BUILD_OUTPUT=%ENL_ROOT%\build-output\RExtension\windows\%MSVC_BUILD_CONFIGURATION%

mkdir %BUILD_OUTPUT%\packages
powershell -NoProfile -ExecutionPolicy Unrestricted -Command "Compress-Archive -Path %BUILD_OUTPUT%\libRExtension.dll -DestinationPath %ENL_ROOT%\build\RExtension\windows\%MSVC_BUILD_CONFIGURATION%\packages\R-lang-extension.zip -Force"

SET MSVC_BUILD_CONFIGURATION=release
SET BUILD_OUTPUT=%ENL_ROOT%\build-output\RExtension\windows\%MSVC_BUILD_CONFIGURATION%

mkdir %BUILD_OUTPUT%\packages
powershell -NoProfile -ExecutionPolicy Unrestricted -Command "Compress-Archive -Path %BUILD_OUTPUT%\libRExtension.dll -DestinationPath %BUILD_OUTPUT%\packages\R-lang-extension.zip -Force"

EXIT /b %ERRORLEVEL%
