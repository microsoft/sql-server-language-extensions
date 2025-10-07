#!/bin/bash
set -euo pipefail

# ============================================
# ONNX Runtime Extension Linux Packaging Script
# ============================================
# 
# SUMMARY:
# This script packages compiled ONNX Runtime extension binaries into 
# distributable ZIP files for Linux deployment. It supports both debug 
# and release build configurations.
#
# PURPOSE:
# - Creates deployment-ready ZIP packages from compiled binaries
# - Supports multiple build configurations (debug/release)
# - Ensures consistent packaging structure for CI/CD pipelines
# - Validates successful package creation with error handling
#
#
# INPUT:
#   - Source binaries from: ../x64/bin/
#   - Build configurations: debug, release (case-insensitive)
#
# OUTPUT:
#   - ZIP files created at: ../build-output/onnxextension/linux/{config}/packages/
#   - Package name: onnxruntime-extension.zip
#
# DEPENDENCIES:
#   - zip utility (for archive creation)
#   - bash 4.0+ (for array support)
#
#
# OUTPUT STRUCTURE (Target):
#   onnxruntime-extension/
#   └── build-output/
#       └── onnxextension/
#           └── linux/
#               ├── debug/            <- Debug configuration output
#               │   └── packages/
#               │       └── onnxruntime-extension.zip  <- Debug ZIP package
#               └── release/          <- Release configuration output
#                   └── packages/
#                       └── onnxruntime-extension.zip  <- Release ZIP package
#
# PACKAGING PROCESS:
#   1. Script reads from: ../x64/bin/ (all contents)
#   2. Creates structure: ../build-output/onnxextension/linux/{config}/packages/
#   3. Archives contents into: onnxruntime-extension.zip
#   4. Each configuration gets its own ZIP in separate directory
#
# ============================================
# Define paths for output and zip directories
# ============================================
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_DIR="$SCRIPT_DIR/../x64/bin"
ZIP_DIR="$SCRIPT_DIR/../build-output/onnxextension/linux"
ZIP_NAME="onnxruntime-extension.zip"

echo "[INFO] OUTPUT_DIR: $OUTPUT_DIR"
echo "[INFO] ZIP_DIR: $ZIP_DIR"
echo "[INFO] ZIP_NAME: $ZIP_NAME"

# ============================================
# Parse arguments: default to both debug and release if none specified
# ============================================
if [ $# -eq 0 ]; then
    BUILD_CONFIGS=(debug release)
    echo "[INFO] No build config specified, defaulting to debug and release."
else
    BUILD_CONFIGS=()
    for arg in "$@"; do
        case "${arg,,}" in
            debug)
                BUILD_CONFIGS+=(debug)
                ;;
            release)
                BUILD_CONFIGS+=(release)
                ;;
            *)
                echo "[ERROR] Invalid build configuration: '$arg'"
                echo "Usage: $(basename "$0") [debug] [release]"
                exit 1
                ;;
        esac
    done
fi

found=0

# ============================================
# Process each specified build configuration
# ============================================
for build_config in "${BUILD_CONFIGS[@]}"; do
    echo "[INFO] Processing build configuration: $build_config"
    CONFIG_OUTPUT_DIR="$OUTPUT_DIR"
    CONFIG_ZIP_DIR="$ZIP_DIR/$build_config/packages"
    CONFIG_ZIP_PATH="$CONFIG_ZIP_DIR/$ZIP_NAME"

    if [ ! -d "$CONFIG_OUTPUT_DIR" ]; then
        echo "[ERROR] Output directory not found: $CONFIG_OUTPUT_DIR"
        continue
    fi

    found=1

    # Create output directories if they do not exist
    mkdir -p "$CONFIG_ZIP_DIR"

    # Clean up any existing zip file before creating a new one
    rm -f "$CONFIG_ZIP_PATH"
    # Zip the build output using zip
    echo "[INFO] Creating $build_config zip at $CONFIG_ZIP_PATH..."
    (cd "$CONFIG_OUTPUT_DIR" && zip -r "$CONFIG_ZIP_PATH" .)

    if [ $? -ne 0 ]; then
        echo "[ERROR] Failed to create zip for $build_config."
        continue
    fi

    # Check if the zip file was actually created
    if [ ! -f "$CONFIG_ZIP_PATH" ]; then
        echo "[ERROR] Zip file not found after creation: $CONFIG_ZIP_PATH"
        continue
    else
        echo "[INFO] Successfully created zip for $build_config."
    fi

done

if [ "$found" -eq 0 ]; then
    echo "[ERROR] No valid build configuration specified."
    echo "Usage: $(basename "$0") [debug] [release]"
    exit 1
fi

echo "[INFO] ONNX extension packaging complete."
exit 0