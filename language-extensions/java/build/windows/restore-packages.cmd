SET ENL_ROOT=%~dp0..\..\..\..
CALL %ENL_ROOT%\restore-packages.cmd

SET PACKAGES_ROOT=%ENL_ROOT%\packages
SET JAVA_VERSION=17.0.5

REM Download and install JAVA
REM
SET JAVA_DOWNLOAD_URL="https://aka.ms/download-jdk/microsoft-jdk-%JAVA_VERSION%-windows-x64.zip"
SET JAVA_INSTALLATION_PATH=%ProgramFiles%\JAVA

curl %JAVA_DOWNLOAD_URL% -L -o "java-%JAVA_VERSION%.zip"
powershell -NoProfile -ExecutionPolicy Unrestricted -Command "Expand-Archive -Force -Path 'java-%JAVA_VERSION%.zip' -DestinationPath '%PACKAGES_ROOT%'"
DEL "java-%JAVA_VERSION%.zip"
