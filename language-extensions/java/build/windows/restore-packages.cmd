SET ENL_ROOT=%~dp0..\..\..\..
CALL %ENL_ROOT%\restore-packages.cmd
nuget restore %ENL_ROOT%\language-extensions\java\packages.config -PackagesDirectory %ENL_ROOT%\packages
