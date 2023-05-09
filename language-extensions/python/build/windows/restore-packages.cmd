SET ENL_ROOT=%~dp0..\..\..\..
CALL %ENL_ROOT%\restore-packages.cmd

SET PACKAGES_ROOT=%ENL_ROOT%\packages

REM Specify the Python version to be downloaded and installed
REM
SET PYTHON_VERSION=3.10.2
SET PYTHON_VERSION_MAJOR_MINOR=3.10
SET PYTHON_VERSION_NO_DOT=310

REM Specify the Boost version to be downloaded and installed
REM
SET BOOST_VERSION=1.79.0
SET BOOST_VERSION_IN_UNDERSCORE=1_79_0

REM Download and install Python from the official Python website
REM
SET PYTHON_DOWNLOAD_URL="https://www.python.org/ftp/python/%PYTHON_VERSION%/python-%PYTHON_VERSION%-amd64.exe"
SET PYTHON_INSTALLATION_PATH=C:\Python%PYTHON_VERSION_NO_DOT%

SET "PYTHON_INSTALLATION_PATH_DOUBLE_SLASH=%PYTHON_INSTALLATION_PATH:\=\\%"

REM Download the Python installer using curl
REM
curl %PYTHON_DOWNLOAD_URL% -o "python-%PYTHON_VERSION%.exe"

REM Run the installer in quiet mode, install for all users, prepend Python to PATH, and specify installation directory
REM
"python-%PYTHON_VERSION%.exe" /quiet InstallAllUsers=1 PrependPath=1 TargetDir="%PYTHON_INSTALLATION_PATH%"

REM Set the PYTHONHOME and PYTHONPATH for the build session
REM
SET PYTHONHOME=%PYTHON_INSTALLATION_PATH%
SET PYTHONPATH=%PYTHON_INSTALLATION_PATH%

REM Download and install pip
REM
curl -sS https://bootstrap.pypa.io/get-pip.py |"%PYTHON_INSTALLATION_PATH%\python.exe"

REM Install numpy and pandas
REM
"%PYTHON_INSTALLATION_PATH%\python.exe" -m pip install pandas numpy

REM Remove the Python installer which is no longer needed
REM
del "python-%PYTHON_VERSION%.exe"

REM Download the specified version of Boost from SourceForge
REM Extract the downloaded Boost zip file to the packages directory
REM Remove the Boost zip file
REM Navigate to the extracted Boost directory
REM
curl -L -o boost_%BOOST_VERSION_IN_UNDERSCORE%.zip https://sourceforge.net/projects/boost/files/boost/%BOOST_VERSION%/boost_%BOOST_VERSION_IN_UNDERSCORE%.zip/download
powershell -NoProfile -ExecutionPolicy Unrestricted -Command "Expand-Archive -Force -Path 'boost_%BOOST_VERSION_IN_UNDERSCORE%.zip' -DestinationPath '%PACKAGES_ROOT%'"
del boost_%BOOST_VERSION_IN_UNDERSCORE%.zip
pushd %PACKAGES_ROOT%\boost_%BOOST_VERSION_IN_UNDERSCORE%

REM Create a Boost user-config.jam configuration file for building Boost.Python
REM
echo using python : %PYTHON_VERSION_MAJOR_MINOR% : "%PYTHON_INSTALLATION_PATH_DOUBLE_SLASH%\\python" : "%PYTHON_INSTALLATION_PATH_DOUBLE_SLASH%\\include" : "%PYTHON_INSTALLATION_PATH_DOUBLE_SLASH%\\libs" ; > user-config.jam

REM Run Boost's bootstrap script and build Boost.Python with the created configuration
REM
CALL bootstrap.bat
b2.exe -j12 --with-python --user-config="%PACKAGES_ROOT%\boost_%BOOST_VERSION_IN_UNDERSCORE%\user-config.jam" --debug-configuration -d0

REM If building in pipeline, set the PYTHONHOME here to overwrite the existing PYTHONHOME
REM
if NOT "%BUILD_BUILDID%"=="" (
	setx PYTHONHOME "%PYTHON_INSTALLATION_PATH%"
)

popd
