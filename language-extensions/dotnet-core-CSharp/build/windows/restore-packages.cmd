@ECHO off
SETLOCAL

SET ENL_ROOT=%~dp0..\..\..\..
SET DOTNET_EXTENSION_HOME=%ENL_ROOT%\language-extensions\dotnet-core-CSharp
SET DOTNET_MANAGED_SRC=%DOTNET_EXTENSION_HOME%\src\managed

REM Call the root level restore-packages
REM
SET PACKAGES_ROOT=%ENL_ROOT%\packages
CALL %ENL_ROOT%\restore-packages.cmd
CALL :CHECKERROR %ERRORLEVEL% "Error: Failed to restore common nuget packages." || EXIT /b %ERRORLEVEL%

dotnet restore %DOTNET_MANAGED_SRC%\Microsoft.SqlServer.CSharpExtension.csproj

EXIT /b %ERRORLEVEL%

:CHECKERROR
	IF %1 NEQ 0 (
		ECHO %2
		EXIT /b %1
	)

	EXIT /b 0
