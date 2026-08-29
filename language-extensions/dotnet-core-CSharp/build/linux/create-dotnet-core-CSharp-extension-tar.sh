#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ENL_ROOT="$(cd "$SCRIPT_DIR/../../../.." && pwd)"
DOTNET_EXTENSION_WORKING_DIR="$ENL_ROOT/build-output/dotnet-core-CSharp-extension/linux"

check_error() {
    local error_level=$1
    local error_message=$2
    if [ "$error_level" -ne 0 ]; then
        echo "Error: $error_message" >&2
        exit "$error_level"
    fi
}

# Default to release if no arguments provided
if [ $# -eq 0 ]; then
    set -- "release"
fi

# Process each build configuration
for BUILD_CONFIGURATION in "$@"; do
    BUILD_CONFIGURATION="$(echo "$BUILD_CONFIGURATION" | tr '[:upper:]' '[:lower:]')"

    # Default to release if not debug
    if [ "$BUILD_CONFIGURATION" != "debug" ]; then
        BUILD_CONFIGURATION="release"
    fi

    BUILD_OUTPUT="$DOTNET_EXTENSION_WORKING_DIR/$BUILD_CONFIGURATION"
    mkdir -p "$BUILD_OUTPUT/packages"

    # Delete the ref folder so that the tarball can be loaded by the SPEES
    rm -rf "$BUILD_OUTPUT/ref"

    # Collect files to compress
    FILES_TO_COMPRESS=()
    FILES_TO_COMPRESS+=("Microsoft.SqlServer.CSharpExtension.runtimeconfig.json")
    FILES_TO_COMPRESS+=("Microsoft.SqlServer.CSharpExtension.deps.json")

    # Add all .dll and .so files
    for f in "$BUILD_OUTPUT"/*.dll "$BUILD_OUTPUT"/*.so; do
        [ -e "$f" ] && FILES_TO_COMPRESS+=("$(basename "$f")")
    done

    # Include .pdb files for debug builds
    if [ "$BUILD_CONFIGURATION" = "debug" ]; then
        for f in "$BUILD_OUTPUT"/*.pdb; do
            [ -e "$f" ] && FILES_TO_COMPRESS+=("$(basename "$f")")
        done
    fi

    # Include the shared/ directory (framework file copies for component hosting).
    # The build script creates shared/Microsoft.NETCore.App/<version>/ with copies
    # of the root DLLs/SOs so hostfxr can resolve the bundled .NET runtime.
    # File copies are used because SQL Server's archive extraction does not preserve
    # symlinks or hard links.
    if [ -d "$BUILD_OUTPUT/shared" ]; then
        FILES_TO_COMPRESS+=("shared")
    fi

    # Package the binaries.
    #
    # IMPORTANT: SQL Server on Linux extracts CREATE EXTERNAL LANGUAGE archives
    # using its built-in zip code (the same path R/Python/Java use). Tar.gz is
    # NOT supported on Linux - exthost would silently fail to find the .so and
    # exit with status 9 immediately after launch. Use zip + .zip extension to
    # match the format expected by the satellite/launchpad.
    #
    # Both .tar.gz and .zip outputs are produced for backward compatibility:
    # - dotnet-core-CSharp-lang-extension-linux.zip   (NEW - used by Linux launchpad)
    # - dotnet-core-CSharp-lang-extension.tar.gz      (legacy/Windows-style, kept for compat)
    pushd "$BUILD_OUTPUT" > /dev/null

    # Primary: zip archive (Linux SQL Server expects this format)
    rm -f "$BUILD_OUTPUT/packages/dotnet-core-CSharp-lang-extension-linux.zip"
    zip -r "$BUILD_OUTPUT/packages/dotnet-core-CSharp-lang-extension-linux.zip" "${FILES_TO_COMPRESS[@]}" > /dev/null
    check_error $? "Failed to create zip for dotnet-core-CSharp-extension for configuration=$BUILD_CONFIGURATION"

    # Legacy: tar.gz (kept for any callers still referencing the old name)
    tar -czf "$BUILD_OUTPUT/packages/dotnet-core-CSharp-lang-extension.tar.gz" "${FILES_TO_COMPRESS[@]}"
    check_error $? "Failed to create tarball for dotnet-core-CSharp-extension for configuration=$BUILD_CONFIGURATION"
    popd > /dev/null

    echo "Success: Compressed dotnet-core-CSharp-extension for $BUILD_CONFIGURATION configuration."
done
