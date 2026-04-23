param (
    [Parameter(Position=0)]
    [ValidateSet("Release", "Debug", "Debug Release", "Release Debug")]
    [string]$BuildConfig = "Debug Release" # Default to Release if no argument is provided
)

function Write-Log {
    param(
        [string]$Message,
        [ValidateSet("INFO", "ERROR", "WARNING")]
        [string]$Level = "INFO"
    )

    $logMessage = "[$Level] $Message"
    Write-Host $logMessage
}

# Parse build configurations
$configs = $BuildConfig.Split(" ", [StringSplitOptions]::RemoveEmptyEntries)
Write-Log "Starting build process for configurations: $($configs -join ', ')"

# Check if rustup is installed and install it if not
Write-Log "Checking Rustup installation..."
if (-not (Get-Command rustup -ErrorAction SilentlyContinue)) {
    Write-Log "Rustup is not installed. Installing rustup..."
    try {
        Invoke-WebRequest -Uri https://sh.rustup.rs -OutFile rustup-init.exe
        Start-Process -FilePath .\rustup-init.exe -ArgumentList '-y' -Wait
        Remove-Item -Path .\rustup-init.exe -Force
        if (-not (Get-Command rustup -ErrorAction SilentlyContinue)) {
            Write-Log "Error: Failed to install rustup." "ERROR"
            exit 1
        }
    }
    catch {
        Write-Log "Error installing Rustup: $_" "ERROR"
        exit 1
    }
} else {
    Write-Log "Rustup is already installed."
}

# Add submodule and update
Write-Log "Setting up git submodules..."
try {
    git submodule update --init --recursive
    Write-Log "Submodules updated successfully."
}
catch {
    Write-Log "Error updating submodules: $_" "ERROR"
    exit 1
}

# Create build directory
Write-Log "Creating build directory..."
$buildDir = "build"
if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}
Push-Location $buildDir

# Build for each configuration
foreach ($config in $configs) {
    Write-Log "Starting build for configuration: $config"

    try {
        # Use CMake to configure and build the project
        Write-Log "Running CMake configuration..."
        cmake .. -DCMAKE_BUILD_TYPE=$config
        if ($LASTEXITCODE -ne 0) {
            Write-Log "Error: CMake configuration failed for $config." "ERROR"
            continue
        }

        Write-Log "Building project..."
        cmake --build . --config $config
        if ($LASTEXITCODE -ne 0) {
            Write-Log "Error: CMake build failed for $config." "ERROR"
            continue
        }

        Write-Log "Build completed successfully for $config"
    }
    catch {
        Write-Log "Unexpected error during $config build: $_" "ERROR"
        continue
    }
}

Pop-Location

# Return the last exit code
exit $LASTEXITCODE
