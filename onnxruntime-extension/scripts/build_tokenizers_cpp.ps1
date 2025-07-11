param (
    [ValidateSet("Release", "Debug")]
    [string]$BuildConfig = "Release" # Default to Release if no argument is provided
)

# Check if this script is running from root by checking if script dir is there or not
if (-not (Test-Path ".\scripts")) {
    Write-Host "Error: This script must be run from the root directory of the repository."
    exit 1
}

# Check if rustup is installed and install it if not
# Rustup is the recommended toolchain installer and version manager for Rust.
# The tokenizers-cpp project depends on Rust for building its core components,
# so we need to ensure rustup (and thus Rust) is available before proceeding.
if (-not (Get-Command rustup -ErrorAction SilentlyContinue)) {
    Write-Host "Rustup is not installed. Installing rustup..."
    Invoke-WebRequest -Uri https://sh.rustup.rs -OutFile rustup-init.exe
    Start-Process -FilePath .\rustup-init.exe -ArgumentList '-y' -Wait
    Remove-Item -Path .\rustup-init.exe -Force
    if (-not (Get-Command rustup -ErrorAction SilentlyContinue)) {
        Write-Host "Error: Failed to install rustup."
        exit 1
    }
} else {
    Write-Host "Rustup is already installed."
}

# Add submodule and update
# Check if the submodule already exists before adding
$submodulePath = "external\tokenizers-cpp"
if (-not (Test-Path $submodulePath)) {
    git submodule add https://github.com/mlc-ai/tokenizers-cpp $submodulePath
} else {
    Write-Host "Submodule 'external/tokenizers-cpp' already exists. Skipping 'git submodule add'."
}
git submodule update --init --recursive

# Copy StaticExports.cc to external/tokenizers-cpp/src
$sourcePath = "scripts\StaticExports.cc"
$destinationPath = "external\tokenizers-cpp\src\StaticExports.cc"

# Copy the file
Copy-Item -Path $sourcePath -Destination $destinationPath -Force

# Add add_library(tokenizers_cpp SHARED ${TOKENIZER_CPP_SRCS} src/StaticExports.cc) to the CMakeLists.txt
$cmakeListsPath = "external\tokenizers-cpp\CMakeLists.txt"
$cmakeListsContent = Get-Content -Path $cmakeListsPath

# $cmakeListsContent contains the text of a CMakeLists.txt file.
# The -replace operator searches for the line:
#   add_library(tokenizers_cpp STATIC ${TOKENIZER_CPP_SRCS})
# and replaces it with:
#   add_library(tokenizers_cpp SHARED ${TOKENIZER_CPP_SRCS} src/StaticExports.cc)
#
# This change does two things:
# 1. Changes the library type from STATIC (a static library) to SHARED (a dynamic/shared library).
# 2. Adds 'src/StaticExports.cc' to the list of source files for the library.
$cmakeListsContent = $cmakeListsContent -replace 'add_library\(tokenizers_cpp STATIC \${TOKENIZER_CPP_SRCS}\)', 'add_library(tokenizers_cpp SHARED ${TOKENIZER_CPP_SRCS} src/StaticExports.cc)'Set-Content -Path $cmakeListsPath -Value $cmakeListsContent

# Build the project
pushd external/tokenizers-cpp
$buildDir = "build"
if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir
}
cd $buildDir


# Check if cmake is installed
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Host "CMake is not installed or not in PATH. Attempting to locate CMake via Visual Studio using vswhere.exe..."

    # Try to find vswhere.exe
    $vswherePath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswherePath)) {
        Write-Host "Error: vswhere.exe not found. Please install CMake or ensure it is in your PATH."
        exit 1
    }

    # Find the latest Visual Studio installation path
    $vsInstallPath = & $vswherePath -latest -products * -requires Microsoft.Component.MSBuild -property installationPath
    if (-not $vsInstallPath) {
        Write-Host "Error: Visual Studio installation not found."
        exit 1
    }

    # Try to find cmake.exe under the VS installation
    $cmakePath = Join-Path $vsInstallPath "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    if (-not (Test-Path $cmakePath)) {
        Write-Host "Error: cmake.exe not found in Visual Studio installation. Please install CMake or ensure it is in your PATH."
        exit 1
    }

    & $cmakePath -G "Visual Studio 17 2022" -A x64 ..
    & $cmakePath --build . --config $BuildConfig
} else {
    & cmake -G "Visual Studio 17 2022" -A x64 ..
    & cmake --build . --config $BuildConfig
}

# Check if the build was successful
if ($LASTEXITCODE -ne 0) {
    Write-Host "Error: Build failed."
    exit 1
} else {
    Write-Host "Build completed successfully."
}

popd
