@ECHO off
SETLOCAL

SET ENL_ROOT=%~dp0..\..\..\..

REM Call the root level restore-packages
REM
SET PACKAGES_ROOT=%ENL_ROOT%\packages
CALL %ENL_ROOT%\restore-packages.cmd
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to restore common nuget packages." || EXIT /b %ERRORLEVEL%

REM Set DEFAULT_R_HOME
REM
SET R_VERSION=4.0.2
SET R_INSTALLER=R-%R_VERSION%-win.exe
SET DEFAULT_R_HOME=%PACKAGES_ROOT%\R-%R_VERSION%-win

IF "%R_HOME%" == "" (
	REM If R_HOME not defined, download and install R-%R_VERSION%
	REM
	SET R_HOME=%DEFAULT_R_HOME%
	MKDIR %DEFAULT_R_HOME%
	powershell -Command "Invoke-WebRequest https://cran.r-project.org/bin/windows/base/%R_INSTALLER%  -OutFile %DEFAULT_R_HOME%\%R_INSTALLER%"
	CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to download R-%R_VERSION%." || EXIT /b %ERRORLEVEL%
	%DEFAULT_R_HOME%\%R_INSTALLER% /VERYSILENT /DIR=%DEFAULT_R_HOME%
	CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to install R-%R_VERSION%." || EXIT /b %ERRORLEVEL%
) ELSE (
	ECHO Using the already defined R_HOME=%R_HOME%
)

SET R_BIN_PATH=%R_HOME%\bin

SETLOCAL enabledelayedexpansion
SET R_LIBRARY_PATH=%R_HOME%\library

REM Install Rcpp and RInside
REM
"%R_BIN_PATH%\R" -e "install.packages('Rcpp', lib = '!R_LIBRARY_PATH:\=/!', repos = 'https://ftp.osuosl.org/pub/cran/')"
"%R_BIN_PATH%\R" -e "stopifnot(require(Rcpp))"
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to install Rcpp package" || EXIT /b %ERRORLEVEL%

"%R_BIN_PATH%\R" -e "install.packages('RInside', lib = '!R_LIBRARY_PATH:\=/!', repos = 'https://ftp.osuosl.org/pub/cran/')"
"%R_BIN_PATH%\R" -e "stopifnot(require(RInside))"
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to install RInside package" || EXIT /b %ERRORLEVEL%
SETLOCAL disabledelayedexpansion

REM Get RTools35 for mingw32-make and rtools40 for g++ v8.3.0 that works with C++17.
REM
SET RTOOLS_HOME=%PACKAGES_ROOT%\Rtools
MKDIR %RTOOLS_HOME%
powershell -Command "Invoke-WebRequest https://cran.r-project.org/bin/windows/Rtools/Rtools35.exe -OutFile %RTOOLS_HOME%\Rtools35.exe"
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to download Rtools3.5." || EXIT /b %ERRORLEVEL%
powershell -Command "Invoke-WebRequest https://cran.r-project.org/bin/windows/Rtools/rtools40-x86_64.exe -OutFile %RTOOLS_HOME%\rtools40.exe"
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to download Rtools4.0." || EXIT /b %ERRORLEVEL%

REM Install RTools
REM
"%RTOOLS_HOME%\Rtools35.exe" /VERYSILENT /DIR="C:\Rtools\"
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to install Rtools3.5." || EXIT /b %ERRORLEVEL%
"%RTOOLS_HOME%\rtools40.exe" /VERYSILENT /DIR="C:\rtools40\"
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to install Rtools4.0." || EXIT /b %ERRORLEVEL%

EXIT /b %ERRORLEVEL%

:CHECKERROR
	IF %1 NEQ 0 (
		ECHO %2
		EXIT /b %1
	)

	EXIT /b 0
