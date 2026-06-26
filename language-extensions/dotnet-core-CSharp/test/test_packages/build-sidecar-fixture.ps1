# Builds testpackageL-SIDECAR.zip -- a regression fixture for PR #85.
#
# Layout:
#
#   foo.deps.json            root-level sidecar (matches "{libName}." prefix
#                            but is NOT a loadable DLL)
#   foo.runtimeconfig.json   root-level sidecar (same shape)
#   lib/net8.0/foo.dll       the actual DLL, NESTED -- not at root, so
#                            DllUtils.CreateDllList (top-level only) cannot
#                            find it without an alias
#
# Regression scenario: with libName="foo", the loader needs a root-level
# "foo.dll". Pre-fix, DetermineAliasSource matched any root file starting
# with "foo." (StartsWith), so the sidecars caused alias creation to be
# suppressed and the install ended up with no loadable DLL at the root.
# Post-fix, DetermineAliasSource only suppresses on EXACT match against
# "foo.dll" (the alias filename) -- so the sidecars no longer suppress, and
# the nested "lib/net8.0/foo.dll" is cloned to the root as the alias.
#
# The companion test (AliasCreatedWhenOnlySidecarsAtRootTest) asserts that
# after install:
#   - SQL_SUCCESS,
#   - root-level "foo.dll" alias EXISTS,
#   - alias is a byte-for-byte copy of "lib/net8.0/foo.dll",
#   - both sidecars were extracted to the root,
#   - the nested "lib/net8.0/foo.dll" was extracted intact.
#
# Run once from the test_packages directory; checks the fixture in.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem

$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$outPath = Join-Path $here 'testpackageL-SIDECAR.zip'

if (Test-Path $outPath) { Remove-Item $outPath -Force }

# Helper: write a single string entry into the archive at $relPath (forward
# slashes; .NET ZipArchive normalizes correctly).
function Add-TextEntry {
    param(
        [System.IO.Compression.ZipArchive]$Archive,
        [string]$RelPath,
        [string]$Text
    )
    $entry = $Archive.CreateEntry($RelPath)
    $writer = New-Object System.IO.StreamWriter($entry.Open())
    try { $writer.Write($Text) } finally { $writer.Dispose() }
}

$stream = [System.IO.File]::Create($outPath)
try {
    $archive = New-Object System.IO.Compression.ZipArchive(
        $stream, [System.IO.Compression.ZipArchiveMode]::Create)
    try {
        # Two root-level sidecars. Both match "foo." prefix; neither is a
        # loadable DLL. The pre-fix StartsWith check would treat either of
        # them as proof the library is "already discoverable" and skip
        # alias creation -- producing an install with no root-level DLL.
        Add-TextEntry -Archive $archive -RelPath 'foo.deps.json' `
            -Text '{"runtimeTarget":{"name":".NETCoreApp,Version=v8.0"}}'
        Add-TextEntry -Archive $archive -RelPath 'foo.runtimeconfig.json' `
            -Text '{"runtimeOptions":{"tfm":"net8.0"}}'

        # The actual DLL, NESTED. DllUtils.CreateDllList only walks the top
        # level, so without an alias clone at the root this DLL is
        # undiscoverable to the loader.
        Add-TextEntry -Archive $archive -RelPath 'lib/net8.0/foo.dll' `
            -Text 'fake DLL content for foo'
    } finally {
        $archive.Dispose()
    }
} finally {
    $stream.Dispose()
}

Write-Host "Wrote: $outPath ($((Get-Item $outPath).Length) bytes)"
