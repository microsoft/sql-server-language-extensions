#!/usr/bin/env bash
set -euo pipefail

# Set root and working directories
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ENL_ROOT="$(cd "$SCRIPT_DIR/../../../.." && pwd)"
DOTNET_EXTENSION_HOME="$ENL_ROOT/language-extensions/dotnet-core-CSharp"
DOTNET_EXTENSION_WORKING_DIR="$ENL_ROOT/build-output/dotnet-core-CSharp-extension/linux"

# Clean and create working directory
rm -rf "$DOTNET_EXTENSION_WORKING_DIR"
mkdir -p "$DOTNET_EXTENSION_WORKING_DIR"

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

    echo "[Info] Building dotnet-core-CSharp-extension libnativecsharpextension.so..."

    # Set build output directory
    BUILD_OUTPUT="$DOTNET_EXTENSION_WORKING_DIR/$BUILD_CONFIGURATION"
    rm -rf "$BUILD_OUTPUT"
    mkdir -p "$BUILD_OUTPUT"
    pushd "$BUILD_OUTPUT" > /dev/null

    # Set source and include paths
    DOTNET_NATIVE_SRC="$DOTNET_EXTENSION_HOME/src/native"
    DOTNET_NATIVE_INCLUDE="$DOTNET_EXTENSION_HOME/include"
    EXTENSION_HOST_INCLUDE="$ENL_ROOT/extension-host/include"
    DOTNET_NATIVE_LIB="$DOTNET_EXTENSION_HOME/lib"

    # Determine C++ compiler
    CXX="${CXX:-c++}"

    # Build compiler arguments
    CC_ARGS=(
        -shared
        -fPIC
        -fshort-wchar
        -std=c++17
        -o libnativecsharpextension.so
        "-I$DOTNET_NATIVE_INCLUDE"
        "-I$EXTENSION_HOST_INCLUDE"
    )

    if [ "$BUILD_CONFIGURATION" = "debug" ]; then
        CC_ARGS+=(-DDEBUG -g)
    fi

    # Add all .cpp source files
    for src in "$DOTNET_NATIVE_SRC"/*.cpp; do
        CC_ARGS+=("$src")
    done

    # Link with static nethost library (must run restore-packages.sh first)
    if [ ! -f "$DOTNET_NATIVE_LIB/libnethost.a" ]; then
        echo "Error: libnethost.a not found at $DOTNET_NATIVE_LIB/libnethost.a" >&2
        echo "       Run restore-packages.sh first to install the .NET SDK and copy libnethost.a." >&2
        exit 1
    fi
    CC_ARGS+=("$DOTNET_NATIVE_LIB/libnethost.a")
    # Link with libdl for dlopen/dlsym
    CC_ARGS+=(-ldl)

    echo "[Info] Compiling with: $CXX ${CC_ARGS[*]}"
    "$CXX" "${CC_ARGS[@]}"

    popd > /dev/null

    # Build managed code
    echo "[Info] Building Microsoft.SqlServer.CSharpExtension dll..."
    DOTNET_MANAGED_SRC="$DOTNET_EXTENSION_HOME/src/managed"
    dotnet build \
        "$DOTNET_MANAGED_SRC/Microsoft.SqlServer.CSharpExtension.csproj" \
        -m \
        -c "$BUILD_CONFIGURATION" \
        -o "$BUILD_OUTPUT" \
        --no-restore \
        --no-dependencies

    echo "Success: Built dotnet-core-CSharp-extension for $BUILD_CONFIGURATION configuration."
done

exit 0
