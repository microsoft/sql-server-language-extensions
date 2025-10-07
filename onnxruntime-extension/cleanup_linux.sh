# -----------------------------------------------------------------------------
# Cleanup Script for ONNX Runtime Extension Build Artifacts (Linux)
#
# This script restores the original .gitmodules file and removes the
# external/tokenizers-cpp submodule and its directory. It also deletes the
# external directory if present.
#
# Usage: Run from the root of the repository after build completion.
# Requirements: Git must be installed and the working directory should be clean.
# 

#!/bin/bash
set -euo pipefail

# Cleanup script for build artifacts (Linux)
echo "Cleaning up build artifacts..."

# Restore original .gitmodules file as build modifies it
git restore ../.gitmodules || {
    echo "[Error] Failed to restore ../.gitmodules. Please restore it manually."
    exit 1
}

# Remove the external/tokenizers-cpp submodule
if [ -d external/tokenizers-cpp ]; then
    echo "Removing external/tokenizers-cpp submodule..."
    git submodule deinit -f external/tokenizers-cpp || {
        echo "[Error] Failed to deinit submodule external/tokenizers-cpp."
        exit 1
    }
    git rm -f external/tokenizers-cpp || {
        echo "[Error] Failed to remove submodule external/tokenizers-cpp."
        exit 1
    }
    git config -f .gitmodules --remove-section submodule.external/tokenizers-cpp || {
        echo "[Error] Failed to update .gitmodules."
        exit 1
    }
    git add .gitmodules || {
        echo "[Error] Failed to add .gitmodules."
        exit 1
    }
    rm -rf external/tokenizers-cpp || {
        echo "[Error] Failed to remove directory external/tokenizers-cpp."
        exit 1
    }
else
    echo "external/tokenizers-cpp folder not found. Skipping..."
fi

# Remove external directory as well
if [ -d external ]; then
    echo "Removing external directory..."
    rm -rf external
else
    echo "External directory not found. Skipping..."
fi

echo "Cleanup completed."
