#!/usr/bin/env pwsh

$ENL_ROOT = Join-Path $PSScriptRoot "../../../.."
$DOTNET_EXTENSION_WORKING_DIR = Join-Path -Path $ENL_ROOT -ChildPath "build-output/dotnet-core-CSharp-extension/linux"

function CheckError {
    param(
        [int]$errorLevel,
        [string]$errorMessage
    )
    if ($errorLevel -ne 0) {
        Write-Error $errorMessage
        exit $errorLevel
    }
}

if ($args.Count -eq 0) {
    # Default to release
    $actualArgs = @("release")
}
else {
    $actualArgs = $args
}

# Process each build configuration
foreach ($BUILD_CONFIGURATION in $actualArgs) {
    $BUILD_CONFIGURATION = $BUILD_CONFIGURATION.ToLower()
    # Default to release if not debug
    if ($BUILD_CONFIGURATION -ne "debug") {
        $BUILD_CONFIGURATION = "release"
    }

    $BUILD_OUTPUT = Join-Path -Path $DOTNET_EXTENSION_WORKING_DIR -ChildPath $BUILD_CONFIGURATION
    New-Item -ItemType Directory -Force -Path "$BUILD_OUTPUT/packages" | Out-Null

    # Delete the ref folder so that the zip can be loaded by the SPEES
    Remove-Item -Recurse -Force -ErrorAction SilentlyContinue "$BUILD_OUTPUT/ref"

    # Define files to compress, conditionally including .pdb files if BUILD_CONFIGURATION is "debug"
    $FILES_TO_COMPRESS = @(
        "Microsoft.SqlServer.CSharpExtension.runtimeconfig.json",
        "Microsoft.SqlServer.CSharpExtension.deps.json"
    )
    $FILES_TO_COMPRESS += Get-ChildItem -Path $BUILD_OUTPUT -Filter "*.dll" | ForEach-Object { $_.Name }
    $FILES_TO_COMPRESS += Get-ChildItem -Path $BUILD_OUTPUT -Filter "*.so" | ForEach-Object { $_.Name }
    if ($BUILD_CONFIGURATION -ieq "debug") {
        $FILES_TO_COMPRESS += Get-ChildItem -Path $BUILD_OUTPUT -Filter "*.pdb" | ForEach-Object { $_.Name }
    }

    # Package the signed binaries.
    try {
        Push-Location $BUILD_OUTPUT
        & tar -czvf "$BUILD_OUTPUT/packages/dotnet-core-CSharp-lang-extension.tar.gz" $FILES_TO_COMPRESS
        CheckError $LASTEXITCODE "Error: Failed to create zip for dotnet-core-CSharp-extension for configuration=$BUILD_CONFIGURATION"
        Pop-Location
        Write-Host "Success: Compressed dotnet-core-CSharp-extension for $BUILD_CONFIGURATION configuration."
    }
    catch {
        CheckError 1 "Error: Failed to create zip for dotnet-core-CSharp-extension for configuration=$BUILD_CONFIGURATION"
    }
}
