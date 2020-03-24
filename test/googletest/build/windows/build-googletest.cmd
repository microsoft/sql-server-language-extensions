@ECHO off
SETLOCAL

SET ENL_ROOT=%~dp0..\..\..\..
SET PACKAGES_ROOT=%ENL_ROOT%\packages
SET CMAKE_ROOT=%PACKAGES_ROOT%\CMake-win64.3.15.5
SET GTEST_HOME=%ENL_ROOT%\test\googletest
SET BUILD_OUTPUT=%ENL_ROOT%\build-output\googletest\windows

REM Call the root level restore-packages
REM
CALL %ENL_ROOT%\restore-packages.cmd

REM Get RTools for mingw32
REM
SET RTOOLS_HOME=%PACKAGES_ROOT%\Rtools
IF NOT EXIST %RTOOLS_HOME% (
	MKDIR %RTOOLS_HOME%
	powershell -Command "Invoke-WebRequest https://cran.r-project.org/bin/windows/Rtools/Rtools35.exe -OutFile %RTOOLS_HOME%\Rtools35.exe"
	REM Install RTools
	REM
	%RTOOLS_HOME%\Rtools35.exe /VERYSILENT /DIR="C:\Rtools\"
)

RMDIR /s /q %BUILD_OUTPUT%
MKDIR %BUILD_OUTPUT%
PUSHD %BUILD_OUTPUT%

REM Make sure g++ is in the PATH.
REM Do not enclose the C:\Rtools\mingw_64\bin path in quotes - cmake test fails
REM
SET PATH=C:\Rtools\bin;C:\Rtools\mingw_64\bin;%PATH%

CALL "%CMAKE_ROOT%\bin\cmake.exe" ^
	-G "MinGW Makefiles" ^
	-DCMAKE_MAKE_PROGRAM=mingw32-make ^
	-DCMAKE_CXX_FLAGS="-std=c++11" ^
	-DCMAKE_INSTALL_PREFIX:PATH=%BUILD_OUTPUT% ^
	-DPLATFORM=windows ^
	%GTEST_HOME%\src
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to configure googletest-framework || EXIT /b %ERRORLEVEL%

REM Call build
REM
CALL "mingw32-make.exe" all
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to build googletest-framework." || EXIT /b %ERRORLEVEL%

REM Continue building using more configs until argv has been exhausted
REM
IF NOT "%~1"=="" GOTO LOOP

EXIT /b %ERRORLEVEL%

:CHECKERROR
	IF %1 NEQ 0 (
		ECHO %2
		EXIT /b %1
	)

	EXIT /b 0
