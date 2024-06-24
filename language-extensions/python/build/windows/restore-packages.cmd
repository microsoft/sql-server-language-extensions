SET ENL_ROOT=%~dp0..\..\..\..
CALL %ENL_ROOT%\restore-packages.cmd

SET PACKAGES_ROOT=%ENL_ROOT%\packages

REM 7zip file extraction tooling - Direct path on pipeline
SET ARCHIVE_TOOL_PATH="C:\7-Zip\7z.exe"

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
SET NUMPY_VERSION=1.22.3
SET PANDAS_VERSION=1.4.2
"%PYTHON_INSTALLATION_PATH%\python.exe" -m pip install --force-reinstall numpy==%NUMPY_VERSION% pandas==%PANDAS_VERSION%

REM Remove the Python installer which is no longer needed
REM
del "python-%PYTHON_VERSION%.exe"

REM BOOST artifact download, extract, build
REM Download the specified version of Boost from SourceForge
REM Extract the downloaded Boost zip file to the packages directory
REM Remove the Boost zip file
REM Navigate to the extracted Boost directory
REM
curl -L -o boost_%BOOST_VERSION_IN_UNDERSCORE%.7z https://archives.boost.io/release/%BOOST_VERSION%/source/boost_%BOOST_VERSION_IN_UNDERSCORE%.7z

REM -o Output directory
REM 2nd param is the file to expand
echo -- Beginning Boost ZIP extraction -- Time: %time% --
%ARCHIVE_TOOL_PATH% x -y -o"%PACKAGES_ROOT%" "boost_%BOOST_VERSION_IN_UNDERSCORE%.7z"
echo -- Finished Boost Zip extration -- Time: %time% --

REM Boost cleanup
echo Delete 7z boost archive
del boost_%BOOST_VERSION_IN_UNDERSCORE%.7z
echo go to dir with boost unarchived
echo %cd%
dir
echo %PACKAGES_ROOT%
pushd %PACKAGES_ROOT%\boost_%BOOST_VERSION_IN_UNDERSCORE%
echo %cd%
dir
echo create user config jam
REM Create a Boost user-config.jam configuration file for building Boost.Python
REM
echo using python : %PYTHON_VERSION_MAJOR_MINOR% : "%PYTHON_INSTALLATION_PATH_DOUBLE_SLASH%\\python" : "%PYTHON_INSTALLATION_PATH_DOUBLE_SLASH%\\include" : "%PYTHON_INSTALLATION_PATH_DOUBLE_SLASH%\\libs" ; > user-config.jam

REM Run Boost's bootstrap script and build Boost.Python with the created configuration
REM Explicitly set VS2019 Compiler tooling 
REM bootstrap - "vc142"
REM b2.exe "toolset=msvc-14.2"
echo -- Beginning Boost b2.exe build -- Time: %time% --
CALL bootstrap.bat vc142
echo -- Beginning Boost build using compiled b2.exe-- Time: %time% --
b2.exe -j12 --prefix=%PACKAGES_ROOT%\output --with-python --user-config="%PACKAGES_ROOT%\boost_%BOOST_VERSION_IN_UNDERSCORE%\user-config.jam" --debug-configuration -dp0 toolset=msvc-14.2
echo -- Finished Boost build -- Time: %time% --

REM If building in pipeline, set the PYTHONHOME here to overwrite the existing PYTHONHOME
REM
if NOT "%BUILD_BUILDID%"=="" (
	setx PYTHONHOME "%PYTHON_INSTALLATION_PATH%"
)

popd
