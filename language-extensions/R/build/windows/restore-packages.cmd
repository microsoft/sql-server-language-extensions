SET EnlRoot=%~dp0..\..\..\..
nuget restore %EnlRoot%\language-extensions\R\packages.config -PackagesDirectory %EnlRoot%\packages
