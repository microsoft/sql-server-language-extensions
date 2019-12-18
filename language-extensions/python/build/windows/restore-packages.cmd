SET EnlRoot=%~dp0..\..\..\..
nuget restore %EnlRoot%\language-extensions\python\packages.config -PackagesDirectory %EnlRoot%\packages
pushd %EnlRoot%\packages
mkdir python

for /D %%d in (*cab_spo*) do (
    expand %%d\*.cab -F:* python >> extract.txt
)
del extract.txt
popd