SET ENL_ROOT=%~dp0..\..\..\..
CALL %ENL_ROOT%\restore-packages.cmd
nuget restore %ENL_ROOT%\language-extensions\python\packages.config -PackagesDirectory %ENL_ROOT%\packages
pushd %ENL_ROOT%\packages
mkdir python

for /D %%d in (*cab_spo*) do (
    expand %%d\*.cab -F:* python >> extract.txt
)
del extract.txt
popd