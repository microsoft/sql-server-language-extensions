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

REM Get RTools35 for mingw32-make and rtools40 for g++ v8.3.0 that works with C++17.
REM
SET RTOOLS_HOME=%PACKAGES_ROOT%\Rtools
IF NOT EXIST %RTOOLS_HOME% (
	MKDIR %RTOOLS_HOME%
	powershell -Command "Invoke-WebRequest https://cran.r-project.org/bin/windows/Rtools/Rtools35.exe -OutFile %RTOOLS_HOME%\Rtools35.exe"
	CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to download Rtools3.5." || EXIT /b %ERRORLEVEL%
	powershell -Command "Invoke-WebRequest https://cran.r-project.org/bin/windows/Rtools/rtools40-x86_64.exe -OutFile %RTOOLS_HOME%\rtools40.exe"
	CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to download Rtools4.0." || EXIT /b %ERRORLEVEL%
	REM Install RTools
	REM
	%RTOOLS_HOME%\Rtools35.exe /VERYSILENT /DIR="C:\Rtools\"
	CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to install Rtools3.5." || EXIT /b %ERRORLEVEL%
	%RTOOLS_HOME%\rtools40.exe /VERYSILENT /DIR="C:\rtools40\"
	CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to install Rtools4.0." || EXIT /b %ERRORLEVEL%
)

RMDIR /s /q %BUILD_OUTPUT%
MKDIR %BUILD_OUTPUT%
PUSHD %BUILD_OUTPUT%

REM Make sure g++, mingw32-make and R.dll are in the PATH.
REM Do not enclose the C:\Rtools\mingw_64\bin path in quotes - cmake test fails
REM
SET PATH=C:\rtools40\mingw64\bin;C:\Rtools\mingw_64\bin;%R_HOME%\bin\x64;%PATH%

CALL "%CMAKE_ROOT%\bin\cmake.exe" ^
	-G "MinGW Makefiles" ^
	-DCMAKE_MAKE_PROGRAM=mingw32-make ^
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
