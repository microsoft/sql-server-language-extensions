$ErrorActionPreference = 'Stop'
if ($PSVersionTable.PSEdition -ne 'Desktop') {
    throw 'Run this test with Windows PowerShell 5.1 (powershell.exe).'
}

$restoreScript = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..\..\build\windows\restore-packages.cmd'))
$testRoot = Join-Path ([IO.Path]::GetTempPath()) ('python-restore-tests-' + [Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $testRoot | Out-Null
$environmentNames = @(
    'PATH', 'BOOST_SOURCE_DIR', 'BUILD_BUILDID', 'DevEnvDir', 'MOCK_LOG',
    'MOCK_RESTORE_EXIT', 'MOCK_BOOTSTRAP_EXIT', 'MOCK_PYTHON_EXIT',
    'MOCK_B2_EXIT', 'MOCK_CURL_EXIT', 'MOCK_SETX_EXIT'
)
$savedEnvironment = @{}
foreach ($name in $environmentNames) {
    $savedEnvironment[$name] = [Environment]::GetEnvironmentVariable($name, 'Process')
}

$stubSource = @'
using System;
using System.IO;
class RestoreFixture {
    static int Main(string[] args) {
        string name = Path.GetFileNameWithoutExtension(System.Reflection.Assembly.GetExecutingAssembly().Location);
        File.AppendAllText(Environment.GetEnvironmentVariable("MOCK_LOG"),
            name + "|" + Directory.GetCurrentDirectory() + "|" + string.Join(" ", args) + "\n");
        int code = int.Parse(Environment.GetEnvironmentVariable("MOCK_" + name.ToUpperInvariant() + "_EXIT") ?? "0");
        if (name == "b2" && code == 0) {
            Directory.CreateDirectory("stage\\lib");
            foreach (string component in new[] { "python310", "numpy310" })
                foreach (string suffix in new[] { "vc143-mt-x64-1_90", "vc143-mt-gd-x64-1_90" })
                    File.WriteAllText("stage\\lib\\libboost_" + component + "-" + suffix + ".lib", "");
        }
        return code;
    }
}
'@

function Invoke-RestoreCase {
    param(
        [string]$Name,
        [switch]$DefaultSource,
        [int]$ExpectedExit = 0,
        [int]$CachedLibraries = 0,
        [string]$MissingMarker,
        [switch]$WrongVersion,
        [string]$FailureComponent,
        [int]$FailureExit = 0,
        [switch]$CiCleanup
    )

    $root = Join-Path $testRoot $Name
    $scriptDirectory = Join-Path $root 'language-extensions\python\build\windows'
    $source = if ($DefaultSource) {
        Join-Path $root 'packages\boost_1_90_0'
    } else {
        Join-Path $root 'local boost sources'
    }
    $python = Join-Path $root 'python with spaces'
    $bin = Join-Path $root 'bin'
    foreach ($path in $scriptDirectory, $python, $bin, (Join-Path $source 'boost')) {
        New-Item -ItemType Directory -Force -Path $path | Out-Null
    }
    foreach ($library in 'python', 'numpy', 'headers', 'unused-fixture') {
        New-Item -ItemType Directory -Path (Join-Path $source "libs\$library") | Out-Null
    }
    Copy-Item -LiteralPath $restoreScript -Destination $scriptDirectory
    Copy-Item -LiteralPath $stub -Destination (Join-Path $python 'python.exe')
    Copy-Item -LiteralPath $stub -Destination (Join-Path $source 'b2.exe')
    Copy-Item -LiteralPath $stub -Destination (Join-Path $bin 'curl.exe')
    Copy-Item -LiteralPath $stub -Destination (Join-Path $bin 'setx.exe')
    Set-Content -LiteralPath (Join-Path $root 'restore-packages.cmd') -Encoding ASCII -Value @'
@echo restore>>"%MOCK_LOG%"
@exit /b %MOCK_RESTORE_EXIT%
'@
    Set-Content -LiteralPath (Join-Path $source 'bootstrap.bat') -Encoding ASCII -Value @'
@echo bootstrap^|%*>>"%MOCK_LOG%"
@echo bootstrap>project-config.jam
@mkdir bootstrap-child
@cd bootstrap-child
@exit /b %MOCK_BOOTSTRAP_EXIT%
'@
    Set-Content -LiteralPath (Join-Path $source 'Jamroot') -Encoding ASCII -Value 'fixture'
    $version = if ($WrongVersion) { '108900' } else { '109000' }
    Set-Content -LiteralPath (Join-Path $source 'boost\version.hpp') -Encoding ASCII -Value "#define BOOST_VERSION $version"
    if ($MissingMarker) {
        Remove-Item -LiteralPath (Join-Path $source $MissingMarker)
    }
    $libraries = @(
        'libboost_python310-vc143-mt-x64-1_90.lib',
        'libboost_numpy310-vc143-mt-x64-1_90.lib',
        'libboost_python310-vc143-mt-gd-x64-1_90.lib',
        'libboost_numpy310-vc143-mt-gd-x64-1_90.lib'
    )
    if ($CachedLibraries -gt 0) {
        $libDirectory = Join-Path $source 'stage\lib'
        New-Item -ItemType Directory -Path $libDirectory -Force | Out-Null
        $libraries | Select-Object -First $CachedLibraries | ForEach-Object {
            Set-Content -LiteralPath (Join-Path $libDirectory $_) -Value ''
        }
    }

    $env:MOCK_LOG = Join-Path $root 'commands.log'
    foreach ($component in 'RESTORE', 'BOOTSTRAP', 'PYTHON', 'B2', 'SETX') {
        [Environment]::SetEnvironmentVariable("MOCK_${component}_EXIT", '0', 'Process')
    }
    $env:MOCK_CURL_EXIT = '41'
    if ($FailureComponent) {
        [Environment]::SetEnvironmentVariable("MOCK_$($FailureComponent.ToUpperInvariant())_EXIT", "$FailureExit", 'Process')
    }
    $env:BOOST_SOURCE_DIR = if ($DefaultSource) { $null } else { $source }
    $env:BUILD_BUILDID = if ($CiCleanup) { 'fixture' } else { $null }
    $env:DevEnvDir = $root
    $env:PATH = "$bin;$($savedEnvironment['PATH'])"
    $command = 'call "' + (Join-Path $scriptDirectory 'restore-packages.cmd') + '" "' + $python + '"'
    $output = & $env:ComSpec /d /c $command 2>&1
    $actualExit = $LASTEXITCODE
    if ($actualExit -ne $ExpectedExit) {
        throw "$Name returned $actualExit; expected $ExpectedExit.`n$($output -join [Environment]::NewLine)"
    }
    $log = if (Test-Path -LiteralPath $env:MOCK_LOG) {
        Get-Content -LiteralPath $env:MOCK_LOG -Raw
    } else {
        ''
    }
    if ($MissingMarker -or $WrongVersion) {
        if ($log) { throw "$Name attempted dependency restoration before rejecting invalid sources" }
        if (($output -join "`n") -notmatch 'Error: BOOST_SOURCE_DIR') {
            throw "$Name did not explain the invalid source input"
        }
    }
    if (-not $DefaultSource -and $log -match 'curl\|') {
        throw "$Name attempted a public download for supplied sources"
    }
    if ($DefaultSource -and $ExpectedExit -eq 41 -and $log -notmatch 'curl\|.*archives.boost.io/release/1.90.0/source/boost_1_90_0.7z') {
        throw "$Name did not preserve the default Boost download"
    }
    if ($ExpectedExit -eq 0) {
        if ($log -notmatch 'python\|.*-m pip install --force-reinstall numpy==1.22.3 pandas==1.4.2') {
            throw "$Name did not preserve Python dependency restoration"
        }
        if ($CachedLibraries -eq 4) {
            if ($log -match 'bootstrap\||b2\||curl\|') { throw "$Name did not reuse the complete cache" }
        } else {
            if ($log -notmatch 'bootstrap\|vc143' -or $log -notmatch 'b2\|.*variant=debug,release') {
                throw "$Name did not bootstrap and build both Boost configurations"
            }
            $buildLine = ($log -split "`n" | Where-Object { $_ -like 'b2|*' }) -split '\|'
            if ($buildLine[1] -ne $source) { throw "$Name built outside the selected Boost root" }
            foreach ($library in $libraries) {
                if (-not (Test-Path -LiteralPath (Join-Path $source "stage\lib\$library"))) {
                    throw "$Name did not produce $library"
                }
            }
            if ((Get-Content -LiteralPath (Join-Path $source 'project-config.jam') -Raw) -notmatch 'using python : 3.10 :') {
                throw "$Name did not configure Boost.Python after bootstrap"
            }
        }
        $unusedExists = Test-Path -LiteralPath (Join-Path $source 'libs\unused-fixture')
        if ($CiCleanup -eq $unusedExists) { throw "$Name did not preserve the CI-only cleanup behavior" }
        foreach ($library in 'python', 'numpy', 'headers') {
            if (-not (Test-Path -LiteralPath (Join-Path $source "libs\$library"))) {
                throw "$Name removed a required library source"
            }
        }
    }
    Write-Host "PASS: $Name"
}

try {
    $stub = Join-Path $testRoot 'fixture.exe'
    Add-Type -TypeDefinition $stubSource -OutputAssembly $stub -OutputType ConsoleApplication
    Invoke-RestoreCase -Name 'local-sources-with-spaces'
    Invoke-RestoreCase -Name 'local-complete-cache' -CachedLibraries 4
    Invoke-RestoreCase -Name 'local-partial-cache' -CachedLibraries 3
    Invoke-RestoreCase -Name 'local-ci-cleanup' -CiCleanup
    Invoke-RestoreCase -Name 'missing-bootstrap' -MissingMarker 'bootstrap.bat' -ExpectedExit 1
    Invoke-RestoreCase -Name 'missing-jamroot' -MissingMarker 'Jamroot' -ExpectedExit 1
    Invoke-RestoreCase -Name 'missing-version-header' -MissingMarker 'boost\version.hpp' -ExpectedExit 1
    Invoke-RestoreCase -Name 'wrong-boost-version' -WrongVersion -ExpectedExit 1
    Invoke-RestoreCase -Name 'invalid-source-with-complete-cache' -WrongVersion -CachedLibraries 4 -ExpectedExit 1
    Invoke-RestoreCase -Name 'default-complete-cache' -DefaultSource -CachedLibraries 4
    Invoke-RestoreCase -Name 'default-download' -DefaultSource -ExpectedExit 41
    Invoke-RestoreCase -Name 'default-partial-cache' -DefaultSource -CachedLibraries 3 -ExpectedExit 41
    Invoke-RestoreCase -Name 'root-restore-failure' -FailureComponent 'restore' -FailureExit 42 -ExpectedExit 42
    Invoke-RestoreCase -Name 'pip-failure' -FailureComponent 'python' -FailureExit 43 -ExpectedExit 43
    Invoke-RestoreCase -Name 'bootstrap-failure' -FailureComponent 'bootstrap' -FailureExit 44 -ExpectedExit 44
    Invoke-RestoreCase -Name 'boost-build-failure' -FailureComponent 'b2' -FailureExit 45 -ExpectedExit 45
} finally {
    foreach ($name in $environmentNames) {
        [Environment]::SetEnvironmentVariable($name, $savedEnvironment[$name], 'Process')
    }
    Remove-Item -LiteralPath $testRoot -Recurse -Force
}
