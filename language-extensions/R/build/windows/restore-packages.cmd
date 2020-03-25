@ECHO off
SETLOCAL

SET ENL_ROOT=%~dp0..\..\..\..

REM Call the root level restore-packages
REM
CALL %ENL_ROOT%\restore-packages.cmd
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to restore common nuget packages." || EXIT /b %ERRORLEVEL%

REM Get the MRO nuget package
REM
SET PACKAGES_ROOT=%ENL_ROOT%\packages
nuget restore %ENL_ROOT%\language-extensions\R\packages.config -PackagesDirectory %PACKAGES_ROOT%
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to restore nuget packages required for RExtension." || EXIT /b %ERRORLEVEL%

REM Set R_HOME
REM
SET DEFAULT_R_HOME=%PACKAGES_ROOT%\External-R.MRO-3.5.2.R.3.5.2.229\Windows

IF "%R_HOME%" == "" (
	IF EXIST %DEFAULT_R_HOME% (
		SET R_HOME=%DEFAULT_R_HOME%
	) ELSE (
		CALL :CHECKERROR %ENVVAR_NOT_FOUND% "Error: R_HOME variable must be set to restore RInside and Rcpp" || EXIT /b %ENVVAR_NOT_FOUND%
	)
)

SET R_BIN_PATH=%R_HOME%\bin

SETLOCAL enabledelayedexpansion
SET R_LIBRARY_PATH=%R_HOME%\library

REM Install Rcpp and RInside
REM
%R_BIN_PATH%\R -e "install.packages('Rcpp', lib = '!R_LIBRARY_PATH:\=/!', repos = 'https://ftp.osuosl.org/pub/cran/')"
%R_BIN_PATH%\R -e "stopifnot(require(Rcpp))"
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to install Rcpp package" || EXIT /b %ERRORLEVEL%

%R_BIN_PATH%\R -e "install.packages('RInside', lib = '!R_LIBRARY_PATH:\=/!', repos = 'https://ftp.osuosl.org/pub/cran/')"
%R_BIN_PATH%\R -e "stopifnot(require(RInside))"
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to install RInside package" || EXIT /b %ERRORLEVEL%
SETLOCAL disabledelayedexpansion

REM Get RTools for mingw32
REM
SET RTOOLS_HOME=%ENL_ROOT%\packages\Rtools
MKDIR %RTOOLS_HOME%
powershell -Command "Invoke-WebRequest https://cran.r-project.org/bin/windows/Rtools/Rtools35.exe -OutFile %RTOOLS_HOME%\Rtools35.exe"
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to download Rtools." || EXIT /b %ERRORLEVEL%

REM Install RTools
REM
%RTOOLS_HOME%\Rtools35.exe /VERYSILENT /DIR="C:\Rtools\"
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to install Rtools." || EXIT /b %ERRORLEVEL%

EXIT /b %ERRORLEVEL%

:CHECKERROR
	IF %1 NEQ 0 (
		ECHO %2
		EXIT /b %1
	)

	EXIT /b 0