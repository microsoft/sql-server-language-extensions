#!/usr/bin/env bash
set -euo pipefail

# Set non-interactive mode for package managers (Azure DevOps pipelines)
export DEBIAN_FRONTEND=noninteractive

# ============================================================
# ONNX Runtime Extension Linux Build Script
# ============================================================
# 
# SUMMARY:
# This script configures and builds the ONNX Runtime Extension for Linux
# using CMake build system. It supports multiple build configurations
# (debug/release) and handles all necessary prerequisites including
# system dependencies and directory structure setup.
#
# PURPOSE:
# - Compiles ONNX Runtime extension shared library for Linux platforms
# - Validates build environment and installs missing dependencies
# - Supports multiple build configurations in single execution
# - Creates proper directory structure matching Windows build layout
#
# USAGE:
#   ./build-onnxruntime-extension.sh [debug] [release]
#   - No arguments: defaults to release build only
#   - Single argument: builds specified configuration only
#   - Multiple arguments: builds all specified configurations
#   - Case-insensitive: 'Debug', 'debug', 'RELEASE', 'release' all valid
#
# PREREQUISITES:
#   - CMake 3.20.0 or higher (verified automatically)
#   - unixODBC development headers (installed automatically if missing)
#   - C++ compiler toolchain (gcc/clang)
#   - Previously restored packages (run restore-packages.sh first)
#
# INPUT DEPENDENCIES:
#   - Source code: ../../src/ (C++ source files)
#   - Headers: ../../include/ (header files)
#   - External packages: ../../packages/ (restored by restore-packages.sh)
#   - CMake configuration: ../../CMakeLists.txt
#
# OUTPUT ARTIFACTS:
#   - Build files: ../../build/x64/build/{Debug|Release}/
#   - Binaries: ../../build/x64/bin/{Debug|Release}/
#   - Primary output: libonnxextension.so (shared library)
#
# SYSTEM REQUIREMENTS:
#   - Linux distribution with apt-get or yum package manager
#   - CMake 3.20.0+ installed and available in PATH
#   - Internet access for installing system dependencies (if needed)
#   - Sufficient disk space for build artifacts
#
# TARGET FOLDER STRUCTURE:
# 
# INPUT STRUCTURE:
#   onnxruntime-extension/
#   ├── CMakeLists.txt               <- CMake configuration file
#   ├── src/                         <- Source code directory
#   │   ├── main.cpp
#   │   ├── OnnxSession.cpp
#   │   └── [other .cpp files]
#   ├── include/                     <- Header files directory
#   │   ├── OnnxSession.h
#   │   └── [other .h files]
#   ├── packages/                    <- External dependencies (from restore-packages.sh)
#   │   ├── Microsoft.ML.OnnxRuntime.{version}/
#   │   └── nlohmann.json.{version}/
#   └── build/
#       └── linux/                   <- Current script location
#           └── build-onnxruntime-extension.sh
#
# OUTPUT STRUCTURE:
#   onnxruntime-extension/
#   └── build/
#       └── x64/
#           ├── build/               <- CMake build files and makefiles
#           │   ├── Debug/           <- Debug build configuration
#           │   │   ├── CMakeFiles/
#           │   │   ├── Makefile
#           │   │   └── [build artifacts]
#           │   └── Release/         <- Release build configuration
#           │       ├── CMakeFiles/
#           │       ├── Makefile
#           │       └── [build artifacts]
#           └── bin/                 <- Final binary outputs
#               ├── Debug/           <- Debug binaries
#               │   └── libonnxextension.so
#               └── Release/         <- Release binaries
#                   └── libonnxextension.so
#
# BUILD PROCESS:
#   1. Validate CMake version (minimum 3.20.0 required)
#   2. Check/install unixODBC development headers
#   3. For each specified build configuration:
#      a. Clean previous build directory
#      b. Create build and binary output directories
#      c. Run CMake configuration phase
#      d. Execute parallel build using all available cores
#      e. Verify expected binary artifact exists
#   4. Return to original directory and report success
# ============================================================

# --- Check if cmake is available and meets minimum version ---
REQUIRED_CMAKE_VERSION="3.20.0"
CMAKE_VERSION_STR=$(cmake --version 2>/dev/null | head -n1 | awk '{print $3}')
if [[ -z "$CMAKE_VERSION_STR" ]]; then
    echo "[ERROR] CMake is not installed or not in PATH. Please install CMake $REQUIRED_CMAKE_VERSION or higher and ensure it is available in your PATH."
    exit 1
fi

# Compare CMake version
verlte() { [ "$1" = "$2" ] && return 0 || [  "$(printf '%s\n' "$1" "$2" | sort -V | head -n1)" = "$1" ]; }
if ! verlte "$REQUIRED_CMAKE_VERSION" "$CMAKE_VERSION_STR"; then
    echo "[ERROR] CMake version $CMAKE_VERSION_STR found, but $REQUIRED_CMAKE_VERSION or higher is required."
    exit 1
fi

echo "[INFO] CMake $CMAKE_VERSION_STR found in PATH."

# --- Ensure unixODBC development headers are installed ---
# Check if sql.h is available (required for compilation)
if ! find /usr/include -name "sql.h" 2>/dev/null | head -1 | grep -q sql.h; then
    echo "[INFO] unixODBC development headers not found. Installing..."
    if command -v apt-get &> /dev/null; then
        # Use heredoc to avoid debconf issues completely
        {
            sudo DEBIAN_FRONTEND=noninteractive apt-get update -qq
            sudo DEBIAN_FRONTEND=noninteractive apt-get install -y -qq unixodbc-dev
        } >/dev/null 2>&1
        
        if [ $? -eq 0 ]; then
            echo "[INFO] unixODBC development headers installed successfully."
        else
            echo "[ERROR] Failed to install unixODBC development headers."
            exit 1
        fi
    elif command -v yum &> /dev/null; then
        sudo yum install -y unixODBC-devel >/dev/null 2>&1
        if [ $? -eq 0 ]; then
            echo "[INFO] unixODBC development headers installed successfully."
        else
            echo "[ERROR] Failed to install unixODBC development headers."
            exit 1
        fi
    else
        echo "[ERROR] Could not detect package manager to install unixODBC development headers."
        exit 1
    fi
else
    echo "[INFO] unixODBC development headers found."
fi

# --- Set up environment variables and directories ---
ENL_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
ONNXRUNTIME_EXTENSION_HOME="$ENL_ROOT/onnxruntime-extension"

# --- Define build and binary output directories ---
# These will be set per configuration below

# --- Save current directory to return later ---
pushd . > /dev/null

# If no arguments, default to release only
if [[ $# -eq 0 ]]; then
    set -- release
fi

while [[ $# -gt 0 ]]; do
    # Parse build configuration argument
    CMAKE_CONFIGURATION="$1"
    if [[ -z "$CMAKE_CONFIGURATION" ]]; then
        CMAKE_CONFIGURATION="release"
    elif [[ "${CMAKE_CONFIGURATION,,}" != "debug" && "${CMAKE_CONFIGURATION,,}" != "release" ]]; then
        echo "[ERROR] Invalid build configuration: '$CMAKE_CONFIGURATION'. Use 'debug' or 'release'."
        popd > /dev/null
        exit 1
    fi
    echo "[INFO] Using build configuration: $CMAKE_CONFIGURATION"

    # Set bin dir to match Windows naming convention (Debug/Release)
    if [[ "${CMAKE_CONFIGURATION,,}" == "debug" ]]; then
        BIN_CONFIG_NAME="Debug"
    else
        BIN_CONFIG_NAME="Release"
    fi
    BUILD_DIR="$ONNXRUNTIME_EXTENSION_HOME/build/x64/build/$BIN_CONFIG_NAME"
    BIN_DIR="$ONNXRUNTIME_EXTENSION_HOME/build/x64/bin/$BIN_CONFIG_NAME"

    # --- Clean up previous build files if they exist ---
    if [ -d "$BUILD_DIR" ]; then
        echo "[INFO] Removing existing build files in $BUILD_DIR"
        rm -rf "$BUILD_DIR"
    else
        echo "[INFO] No previous build directory found for $CMAKE_CONFIGURATION."
    fi

    # --- Create build and binary directories if they do not exist ---
    if [ ! -d "$BUILD_DIR" ]; then
        echo "[INFO] Creating build directory $BUILD_DIR"
        mkdir -p "$BUILD_DIR"
    fi
    if [ ! -d "$BIN_DIR" ]; then
        echo "[INFO] Creating binary directory $BIN_DIR"
        mkdir -p "$BIN_DIR"
    fi

    # --- Change to build directory ---
    cd "$BUILD_DIR"

    # --- Generate build files with CMake ---
    echo "[INFO] Generating build files in $BUILD_DIR..."
    cmake -S "$ONNXRUNTIME_EXTENSION_HOME" -B "$BUILD_DIR" \
        -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="$BIN_DIR" \
        -DCMAKE_LIBRARY_OUTPUT_DIRECTORY="$BIN_DIR" \
        -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="$BIN_DIR" \
        -DCMAKE_BUILD_TYPE="$CMAKE_CONFIGURATION"
    if [ $? -ne 0 ]; then
        echo "[ERROR] Error configuring the project with CMake."
        popd > /dev/null
        exit 1
    fi
    echo "[INFO] CMake configuration completed successfully."

    # --- Build the project ---
    echo "[INFO] Building the project with configuration $CMAKE_CONFIGURATION..."
    cmake --build "$BUILD_DIR" --parallel
    if [ $? -ne 0 ]; then
        echo "[ERROR] Error building the project."
        popd > /dev/null
        exit 1
    fi

    echo "[INFO] Build completed for configuration $CMAKE_CONFIGURATION."

    # --- Check if the expected binary exists ---
    if [ ! -f "$BIN_DIR/libonnxextension.so" ]; then
        echo "[ERROR] Expected binary not found: $BIN_DIR/libonnxextension.so"
        popd > /dev/null
        exit 1
    fi

    shift
done

# --- Restore original directory ---
popd > /dev/null

echo "[SUCCESS] All builds completed successfully."
exit 0