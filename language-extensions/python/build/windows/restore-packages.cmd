SET ENL_ROOT=%~dp0..\..\..\..
CALL %ENL_ROOT%\restore-packages.cmd
nuget restore %ENL_ROOT%\language-extensions\python\packages.config -PackagesDirectory %ENL_ROOT%\packages

set PYTHON_VERSION=3.10.2
set PYTHON_VERSION_NO_DOT=310

REM Download and install python
REM
SET PYTHON_DOWNLOAD_URL="https://www.python.org/ftp/python/%PYTHON_VERSION%/python-%PYTHON_VERSION%-amd64.exe"
SET PYTHON_INSTALLATION_PATH=%ProgramFiles%\Python%PYTHON_VERSION_NO_DOT%

curl %PYTHON_DOWNLOAD_URL% -o "python-%PYTHON_VERSION%.exe"

"python-%PYTHON_VERSION%.exe" /quiet InstallAllUsers=1 PrependPath=1

REM Set the PYTHONHOME and PYTHONPATH for the build session
REM
set PYTHONHOME=%PYTHON_INSTALLATION_PATH%
set PYTHONPATH=%PYTHON_INSTALLATION_PATH%

REM Download and install pip
REM
curl -sS https://bootstrap.pypa.io/get-pip.py |"%PYTHON_INSTALLATION_PATH%\python.exe"

REM Install numpy and pandas
REM
"%PYTHON_INSTALLATION_PATH%\python.exe" -m pip install pandas numpy

del "python-%PYTHON_VERSION%.exe"

pushd %ENL_ROOT%\packages

for /D %%d in (*External-Boost*) do (
	copy %%d\Windows\lib-debug\* %%d\Windows\lib\ >> extract.txt
)

REM If building in pipeline, set the PYTHONHOME here to overwrite the existing PYTHONHOME
REM
if NOT "%BUILD_BUILDID%"=="" (
	setx PYTHONHOME "%PYTHON_INSTALLATION_PATH%"
)

del extract.txt

popd