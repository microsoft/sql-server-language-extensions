# Builds testpackageK-SYMLINK.zip -- a regression fixture for PR #85.
#
# Layout: outer zip wrapping a single inner zip ("inner.zip" at root). The
# inner zip contains:
#
#   legitfile.dll       regular file, plain content
#   evil-symlink.dll    Unix mode 0o120755 (symbolic link), payload "/etc/passwd"
#
# The Unix-mode bits live in the central directory entry's external
# attributes. Today's .NET ZipFile.ExtractToDirectory ignores those bits on
# every platform and writes evil-symlink.dll as a regular file containing
# the literal text "/etc/passwd". The test that uses this fixture
# (InnerZipFutureSymlinkRejectedTest) asserts a future-proofing invariant:
# regardless of how a future .NET runtime decides to materialize that entry,
# the installed library directory must contain NO reparse points.
#
# Run once from the test_packages directory; checks the fixture in.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem

$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$outerPath = Join-Path $here 'testpackageK-SYMLINK.zip'
$innerTempPath = [System.IO.Path]::Combine(
    [System.IO.Path]::GetTempPath(),
    [System.Guid]::NewGuid().ToString() + '.zip')

# Unix mode 0o120755 (symbolic link, owner rwx, group rx, other rx) packed
# into the high 16 bits of the 32-bit external attributes field. The .NET
# ExternalAttributes property is a signed Int32. We parse the hex value as
# Int32 directly so PowerShell's "literal must fit in target type" rule is
# never triggered: 0xA1ED0000 has its sign bit set, so it parses to the
# negative value -1578237952, which is the correct two's-complement
# representation in the central directory.
[int]$externalAttrsSymlink = [int]::Parse(
    'A1ED0000',
    [System.Globalization.NumberStyles]::HexNumber)

# --- Build the inner zip ---
$innerStream = [System.IO.File]::Create($innerTempPath)
try {
    $innerArchive = New-Object System.IO.Compression.ZipArchive(
        $innerStream, [System.IO.Compression.ZipArchiveMode]::Create)
    try {
        # Regular file -- proves install completed (it must end up in installDir).
        $regular = $innerArchive.CreateEntry('legitfile.dll')
        $regWriter = New-Object System.IO.StreamWriter($regular.Open())
        try { $regWriter.Write('regular DLL content') } finally { $regWriter.Dispose() }

        # Unix-symlink-mode entry. Today's .NET writes this as a regular
        # file; a future .NET that honors the mode bits would write a real
        # symlink. The IsReparsePoint guard in CopyDirectory must skip it
        # in the latter case.
        $symlink = $innerArchive.CreateEntry('evil-symlink.dll')
        $symlink.ExternalAttributes = $externalAttrsSymlink
        $symWriter = New-Object System.IO.StreamWriter($symlink.Open())
        try { $symWriter.Write('/etc/passwd') } finally { $symWriter.Dispose() }
    } finally {
        $innerArchive.Dispose()
    }
} finally {
    $innerStream.Dispose()
}

# --- Build the outer zip wrapping inner.zip ---
if (Test-Path $outerPath) { Remove-Item $outerPath -Force }
$outerStream = [System.IO.File]::Create($outerPath)
try {
    $outerArchive = New-Object System.IO.Compression.ZipArchive(
        $outerStream, [System.IO.Compression.ZipArchiveMode]::Create)
    try {
        $entry = $outerArchive.CreateEntry('inner.zip')
        $entryStream = $entry.Open()
        try {
            $innerBytes = [System.IO.File]::ReadAllBytes($innerTempPath)
            $entryStream.Write($innerBytes, 0, $innerBytes.Length)
        } finally {
            $entryStream.Dispose()
        }
    } finally {
        $outerArchive.Dispose()
    }
} finally {
    $outerStream.Dispose()
}

Remove-Item $innerTempPath -Force

Write-Host "Wrote: $outerPath ($((Get-Item $outerPath).Length) bytes)"
