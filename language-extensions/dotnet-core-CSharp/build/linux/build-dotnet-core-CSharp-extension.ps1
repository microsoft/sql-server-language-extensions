#!/usr/bin/env pwsh

# Set root and working directories
$ENL_ROOT = Join-Path $PSScriptRoot "../../../.."
$DOTNET_EXTENSION_HOME = Join-Path $ENL_ROOT "language-extensions/dotnet-core-CSharp"
$DOTNET_EXTENSION_WORKING_DIR = Join-Path $ENL_ROOT "build-output/dotnet-core-CSharp-extension/linux"

# Clean and create working directory
if (Test-Path $DOTNET_EXTENSION_WORKING_DIR) {
    Remove-Item $DOTNET_EXTENSION_WORKING_DIR -Recurse -Force
}
New-Item -ItemType Directory -Path $DOTNET_EXTENSION_WORKING_DIR | Out-Null

$i = 0

# Process each build configuration
while($true) {
    $BUILD_CONFIGURATION = ""
    if ($i -lt $args.Count) {
        $BUILD_CONFIGURATION = $args[$i]
    }
    # Default to release if not debug
    if ($BUILD_CONFIGURATION -ne "debug") {
        $BUILD_CONFIGURATION = "release"
    }

    # Set target directory
    $TARGET = Join-Path $ENL_ROOT "build-output/dotnet-core-CSharp-extension/target/$BUILD_CONFIGURATION"

    # Clean and create target directory
    if (Test-Path $TARGET) {
        Remove-Item $TARGET -Recurse -Force
    }
    New-Item -ItemType Directory -Path $TARGET | Out-Null

    Write-Host "[Info] Building dotnet-core-CSharp-extension nativecsharpextension dll..."

    # Set build output directory
    $BUILD_OUTPUT = Join-Path $DOTNET_EXTENSION_WORKING_DIR $BUILD_CONFIGURATION
    if (Test-Path $BUILD_OUTPUT) {
        Remove-Item $BUILD_OUTPUT -Recurse -Force
    }
    New-Item -ItemType Directory -Path $BUILD_OUTPUT | Out-Null
    Push-Location $BUILD_OUTPUT

    # Set source and include paths
    $DOTNET_NATIVE_SRC = Join-Path $DOTNET_EXTENSION_HOME "src/native"
    $DOTNET_NATIVE_INCLUDE = Join-Path $DOTNET_EXTENSION_HOME "include"
    $EXTENSION_HOST_INCLUDE = Join-Path $ENL_ROOT "extension-host/include"
    $DOTNET_NATIVE_LIB = Join-Path $DOTNET_EXTENSION_HOME "lib"

    # Build native code
    $ccArgs = @(
        "-shared",
        "-fPIC",
        "-o", "libnativecsharpextension.so",
        "-I", $DOTNET_NATIVE_INCLUDE,
        "-I", $EXTENSION_HOST_INCLUDE,
        "-D", "LINUX"
    )
    $ccArgs += Get-ChildItem -Path $DOTNET_NATIVE_SRC -Filter "*.cpp" | ForEach-Object { $_.FullName }
    if ($BUILD_CONFIGURATION -eq "debug") {
        $ccArgs += "-D", "DEBUG"
    }

    if ($null -ne $env:CXX) {
        $cxx = $env:CXX
    } else {
        $cxx = "c++"
    }

    $proc = Start-Process -FilePath $cxx -ArgumentList $ccArgs -NoNewWindow -Wait -PassThru
    if ($proc.ExitCode -ne 0) {
        Write-Host "Error: Failed to build nativecsharpextension for configuration=$BUILD_CONFIGURATION"
        exit $proc.ExitCode
    }

    Write-Host "[Info] Copying dependent libraries..."
    Copy-Item (Join-Path $DOTNET_NATIVE_LIB "libhostfxr.so") $BUILD_OUTPUT -Force

    # Build managed code
    Write-Host "[Info] Building Microsoft.SqlServer.CSharpExtension dll..."
    $DOTNET_MANAGED_SRC = Join-Path $DOTNET_EXTENSION_HOME "src/managed"
    $dotnetProc = Start-Process -FilePath "dotnet" -ArgumentList @(
        "build",
        (Join-Path $DOTNET_MANAGED_SRC "Microsoft.SqlServer.CSharpExtension.csproj"),
        "-m",
        "-c", $BUILD_CONFIGURATION,
        "-o", $BUILD_OUTPUT,
        "--no-dependencies"
    ) -NoNewWindow -Wait -PassThru

    if ($dotnetProc.ExitCode -ne 0) {
        Write-Host "Error: Failed to build for Microsoft.SqlServer.CSharpExtension.dll for configuration=$BUILD_CONFIGURATION"
        exit $dotnetProc.ExitCode
    }

    Write-Host "Success: Built dotnet-core-CSharp-extension for $BUILD_CONFIGURATION configuration."

    $i++
    if ($i -ge $args.Count) {
        break
    }
}

exit 0
