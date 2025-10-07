#!/bin/bash
set -euo pipefail

# ============================================
# ONNX Runtime Extension Linux Package Restore Script
# ============================================
# 
# SUMMARY:
# This script restores external dependencies required for building the ONNX Runtime
# extension on Linux systems. It mirrors the functionality of the Windows 
# restore-packages.cmd script, downloading packages from GitHub releases instead
# of using NuGet.
#
# PURPOSE:
# - Downloads and extracts required external dependencies for Linux builds
# - Creates proper package directory structure for build system integration
# - Ensures consistent dependency versions across Windows/Linux platforms
# - Validates successful package restoration with comprehensive error handling
#
# USAGE:
#   ./restore-packages.sh
#   - No arguments required - reads configuration from packages.config
#   - Must be run before building the ONNX Runtime extension
#   - Automatically creates packages/ directory structure
#
# INPUT:
#   - Configuration file: ../../packages.config (XML format)
#   - Supported packages: Microsoft.ML.OnnxRuntime, nlohmann.json
#
# OUTPUT:
#   - Package directory: ../../packages/
#   - ONNX Runtime: Microsoft.ML.OnnxRuntime.{version}/
#   - nlohmann JSON: nlohmann.json.{version}/include/nlohmann/json.hpp
#
# DOWNLOAD SOURCES:
#   - ONNX Runtime: https://github.com/microsoft/onnxruntime/releases/
#   - nlohmann JSON: https://github.com/nlohmann/json/releases/
#
# ERROR HANDLING:
#   - Validates packages.config existence and XML structure
#   - Implements retry logic with 3 attempts per download
#   - Verifies package extraction and directory structure
#   - Exits with error if any package restoration fails
#
# DEPENDENCIES:
#   - curl (for downloading packages)
#   - tar (for extracting .tgz archives)
#   - python3 (for XML parsing of packages.config)
#   - mkdir, ls, awk (standard Linux utilities)
#
# TARGET FOLDER STRUCTURE:
# 
# INPUT STRUCTURE:
#   onnxruntime-extension/
#   ├── packages.config               <- SOURCE: Package configuration (XML)
#   └── build/
#       └── linux/                   <- Current script location
#           └── restore-packages.sh
#
# OUTPUT STRUCTURE:
#   onnxruntime-extension/
#   └── packages/                    <- TARGET: All restored packages
#       ├── Microsoft.ML.OnnxRuntime.{version}/
#       │   ├── include/             <- ONNX Runtime headers
#       │   ├── lib/                 <- ONNX Runtime libraries
#       │   │   └── libonnxruntime.so.{version}
#       │   └── [other ONNX Runtime files]
#       └── nlohmann.json.{version}/
#           └── include/
#               └── nlohmann/
#                   └── json.hpp     <- Single header JSON library
#
# RESTORE PROCESS:
#   1. Parse packages.config using python3 XML parser
#   2. For each package, download from respective GitHub releases
#   3. Extract/organize files into expected directory structure
#   4. Verify successful restoration and required files existence
#   5. Exit with success/error status for build system integration
#
# ============================================

# These will be extracted from package.config during the restore loop
ONNX_VERSION=""
NLOHMANN_VERSION=""

# ONNX Runtime Extension Package Restore Script for Linux
# This script mirrors the Windows restore-packages.cmd exactly
# It reads packages.config and downloads packages from https://github.com/microsoft/onnxruntime/releases to create the expected structure

# Set the root directory (same as Windows script)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ENL_ROOT="$(realpath "$SCRIPT_DIR/../../..")"
ONNXRUNTIME_EXTENSION_HOME="$ENL_ROOT/onnxruntime-extension"

echo "[INFO] Script directory: $SCRIPT_DIR"
echo "[INFO] ENL_ROOT: $ENL_ROOT"
echo "[INFO] ONNXRUNTIME_EXTENSION_HOME: $ONNXRUNTIME_EXTENSION_HOME"

# Check if packages.config exists (same as Windows script)
if [ ! -f "$ONNXRUNTIME_EXTENSION_HOME/packages.config" ]; then
    echo "[ERROR] packages.config not found at \"$ONNXRUNTIME_EXTENSION_HOME/packages.config\"."
    exit 1
fi

echo "[INFO] Found packages.config, proceeding with package restore..."

# Parse packages.config and restore each package
PKG_DIR="$ONNXRUNTIME_EXTENSION_HOME/packages"
mkdir -p "$PKG_DIR"

# Function to download and extract package from GitHub releases
restore_package_from_github() {
    local package_id="$1"
    local package_version="$2"
    
    echo "[INFO] Restoring $package_id version $package_version from GitHub..."
    
    if [ "$package_id" = "Microsoft.ML.OnnxRuntime" ]; then
        # Download ONNX Runtime from GitHub releases
        local release_url="https://github.com/microsoft/onnxruntime/releases/download/v${package_version}/onnxruntime-linux-x64-${package_version}.tgz"
        local package_file="$PKG_DIR/onnxruntime-linux-x64-${package_version}.tgz"
        local package_dir="$PKG_DIR/${package_id}.${package_version}"
        
        # Download package with retry logic (3 attempts)
        max_retries=3
        attempt=1
        success=0
        while [ $attempt -le $max_retries ]; do
            if curl -L --retry 3 --retry-delay 2 -sS -o "$package_file" "$release_url"; then
                echo "[INFO] ✓ Downloaded $package_id successfully (attempt $attempt)"
                echo "[INFO] File size: $(ls -lh "$package_file" | awk '{print $5}')"
                success=1
                break
            else
                echo "[WARNING] Attempt $attempt to download $package_id failed. Retrying..."
                attempt=$((attempt+1))
                sleep 2
            fi
        done
        if [ $success -ne 1 ]; then
            echo "[ERROR] ✗ Failed to download $package_id from $release_url after $max_retries attempts."
            return 1
        fi
        
        # Extract package
        mkdir -p "$package_dir"
        if tar -xzf "$package_file" -C "$package_dir" --strip-components=1; then
            echo "[INFO] ✓ Extracted $package_id successfully"
            rm "$package_file"
        else
            echo "[ERROR] ✗ Failed to extract $package_id"
            return 1
        fi
        
        # Check if the expected package directory exists after extraction
        EXPECTED_PACKAGE_DIR="$PKG_DIR/${package_id}.${package_version}"
        if [ ! -d "$EXPECTED_PACKAGE_DIR" ]; then
            echo "[ERROR] Expected package directory not found after extraction: $EXPECTED_PACKAGE_DIR"
            return 1
        else
            echo "[INFO] Package directory found: $EXPECTED_PACKAGE_DIR"
        fi

    elif [ "$package_id" = "nlohmann.json" ]; then
        # Download nlohmann.json from GitHub releases
        local release_url="https://github.com/nlohmann/json/releases/download/v${package_version}/json.hpp"
        local package_dir="$PKG_DIR/${package_id}.${package_version}"
        local header_file="$package_dir/include/nlohmann/json.hpp"

        mkdir -p "$package_dir/include/nlohmann"

        # Download header file with retry logic (3 attempts)
        max_retries=3
        attempt=1
        success=0
        while [ $attempt -le $max_retries ]; do
            if curl -L --retry 3 --retry-delay 2 -sS -o "$header_file" "$release_url"; then
                echo "[INFO] ✓ Downloaded $package_id successfully (attempt $attempt)"
                echo "[INFO] File size: $(ls -lh "$header_file" | awk '{print $5}')"
                success=1
                break
            else
                echo "[WARNING] Attempt $attempt to download $package_id failed. Retrying..."
                attempt=$((attempt+1))
                sleep 2
            fi
        done
        if [ $success -ne 1 ]; then
            echo "[ERROR] ✗ Failed to download $package_id from $release_url after $max_retries attempts."
            return 1
        fi

        # Check if the expected package directory exists after download
        EXPECTED_PACKAGE_DIR="$PKG_DIR/${package_id}.${package_version}"
        if [ ! -d "$EXPECTED_PACKAGE_DIR" ]; then
            echo "[ERROR] Expected package directory not found after download: $EXPECTED_PACKAGE_DIR"
            return 1
        else
            echo "[INFO] Package directory found: $EXPECTED_PACKAGE_DIR"
        fi
    else
        echo "[ERROR] ✗ Unknown package: $package_id"
        return 1
    fi
    
    return 0
}

# Parse packages.config using python3 for robust XML parsing
if ! command -v python3 &> /dev/null; then
    echo "[ERROR] python3 is required but not installed. Please install python3."
    exit 1
fi

echo "[INFO] Parsing packages.config with python3..."

# Use python3 to extract package id and version pairs
while IFS="|" read -r package_id package_version; do
    if [ -n "$package_id" ] && [ -n "$package_version" ]; then
        echo "[INFO] Found package: $package_id version $package_version"
        restore_package_from_github "$package_id" "$package_version"
        if [ $? -ne 0 ]; then
            echo "[ERROR] Failed to restore $package_id"
            exit 1
        fi
        # Store versions for verification
        if [ "$package_id" = "Microsoft.ML.OnnxRuntime" ]; then
            ONNX_VERSION="$package_version"
        elif [ "$package_id" = "nlohmann.json" ]; then
            NLOHMANN_VERSION="$package_version"
        fi
    fi
done < <(python3 -c "import xml.etree.ElementTree as ET; tree = ET.parse('$ONNXRUNTIME_EXTENSION_HOME/packages.config'); [print(f'{pkg.attrib[\"id\"]}|{pkg.attrib[\"version\"]}') for pkg in tree.findall('.//package')]")



# ONNX_VERSION and NLOHMANN_VERSION are set during the restore loop above

echo "[INFO] Verifying package restoration..."
PKG_DIR="$ONNXRUNTIME_EXTENSION_HOME/packages"
echo "[INFO] Packages directory: $PKG_DIR"

# Check Microsoft.ML.OnnxRuntime
ONNX_DIR="$PKG_DIR/Microsoft.ML.OnnxRuntime.$ONNX_VERSION"
if [ -d "$ONNX_DIR" ]; then
    echo "[INFO] ✓ Microsoft.ML.OnnxRuntime.$ONNX_VERSION restored successfully"
    # Verify structure for GitHub releases (different from NuGet structure)
    if [ -d "$ONNX_DIR/include" ]; then
        echo "[INFO] ✓ ONNX Runtime headers found"
    fi
    if [ -d "$ONNX_DIR/lib" ]; then
        echo "[INFO] ✓ ONNX Runtime library files found"
    fi
    if [ -f "$ONNX_DIR/lib/libonnxruntime.so.$ONNX_VERSION" ]; then
        echo "[INFO] ✓ ONNX Runtime Linux library found"
    fi
else
    echo "[ERROR] ✗ Microsoft.ML.OnnxRuntime.$ONNX_VERSION not found - build will fail"
    exit 1
fi

# Check nlohmann.json
NLOHMANN_DIR="$PKG_DIR/nlohmann.json.$NLOHMANN_VERSION"
if [ -d "$NLOHMANN_DIR" ]; then
    echo "[INFO] ✓ nlohmann.json.$NLOHMANN_VERSION restored successfully"
    # Verify headers for GitHub releases
    if [ -f "$NLOHMANN_DIR/include/nlohmann/json.hpp" ]; then
        echo "[INFO] ✓ nlohmann.json headers found"
    fi
else
    echo "[ERROR] ✗ nlohmann.json.$NLOHMANN_VERSION not found - build will fail"
    exit 1
fi

echo "[INFO] Package restore completed successfully using GitHub releases."

# Exit successfully
exit 0
