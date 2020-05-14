SET ENL_ROOT=%~dp0..\..\..\..
CALL %ENL_ROOT%\restore-packages.cmd
nuget restore %ENL_ROOT%\language-extensions\python\packages.config -PackagesDirectory %ENL_ROOT%\packages
pushd %ENL_ROOT%\packages
mkdir python

for /D %%d in (*cab_spo*) do (
	expand %%d\*.cab -F:* python >> extract.txt
)

for /D %%d in (*External-Boost*) do (
	copy %%d\Windows\lib-debug\* %%d\Windows\lib\ >> extract.txt
)

REM If building in pipeline, set the PYTHONHOME here to overwrite the existing PYTHONHOME
REM
if NOT "%BUILD_BUILDID%"=="" (
	setx PYTHONHOME %ENL_ROOT%\packages\python
)

del extract.txt

popd