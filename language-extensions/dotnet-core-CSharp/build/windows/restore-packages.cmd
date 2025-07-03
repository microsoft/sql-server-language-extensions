@ECHO off
SETLOCAL

SET ENL_ROOT=%~dp0..\..\..\..

REM Call the root level restore-packages
REM
SET PACKAGES_ROOT=%ENL_ROOT%\packages
CALL %ENL_ROOT%\restore-packages.cmd
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to restore common nuget packages." || EXIT /b %ERRORLEVEL%

EXIT /b %ERRORLEVEL%

:CHECKERROR
	IF %1 NEQ 0 (
		ECHO %2
		EXIT /b %1
	)

	EXIT /b 0
