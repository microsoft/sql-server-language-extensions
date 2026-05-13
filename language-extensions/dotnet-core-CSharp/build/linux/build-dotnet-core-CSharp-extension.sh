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

    # Publish managed code as self-contained for linux-x64.
    # This bundles the entire .NET runtime (hostfxr, coreclr, framework DLLs)
    # into the output so the extension works without a system .NET installation.
    echo "[Info] Publishing Microsoft.SqlServer.CSharpExtension (self-contained, linux-x64)..."
    DOTNET_MANAGED_SRC="$DOTNET_EXTENSION_HOME/src/managed"
    dotnet publish \
        "$DOTNET_MANAGED_SRC/Microsoft.SqlServer.CSharpExtension.csproj" \
        -c "$BUILD_CONFIGURATION" \
        -r linux-x64 \
        --self-contained \
        -o "$BUILD_OUTPUT" \
        --no-restore

    # Post-process: transform the self-contained runtimeconfig.json for component hosting.
    # hostfxr rejects self-contained component initialization (error 0x80008093) because
    # runtimeconfig.json contains "includedFrameworks". We convert it to use "framework"
    # (framework-dependent format) and create a shared/ symlink so hostfxr can resolve
    # the bundled runtime via hostfxr_initialize_for_runtime_config.
    RUNTIMECONFIG="$BUILD_OUTPUT/Microsoft.SqlServer.CSharpExtension.runtimeconfig.json"
    if grep -q '"includedFrameworks"' "$RUNTIMECONFIG" 2>/dev/null; then
        FRAMEWORK_VERSION=$(python3 -c "
import json, sys
with open(sys.argv[1]) as f:
    cfg = json.load(f)
ro = cfg.get('runtimeOptions', {})
included = ro.get('includedFrameworks', [])
if not included:
    sys.exit(1)
ro['framework'] = included[0]
del ro['includedFrameworks']
with open(sys.argv[1], 'w') as f:
    json.dump(cfg, f, indent=2)
print(included[0]['version'])
" "$RUNTIMECONFIG")

        if [ -n "$FRAMEWORK_VERSION" ]; then
            # Create framework directory structure with file copies.
            # hostfxr resolves frameworks at <dotnet_root>/shared/<framework>/<version>/
            # which must contain the runtime DLLs and native libraries.
            #
            # We use file copies because SQL Server's internal tar extraction
            # (used by CREATE EXTERNAL LANGUAGE) does not preserve symbolic links
            # or hard links -- both are silently dropped during extraction.
            # Copying the files ensures they are always present after extraction.
            FRAMEWORK_DIR="$BUILD_OUTPUT/shared/Microsoft.NETCore.App/$FRAMEWORK_VERSION"
            mkdir -p "$FRAMEWORK_DIR"
            # Copy all runtime files from .NET shared framework.
            # The source is the published output which contains the framework files.
            # hostfxr requires Microsoft.NETCore.App.deps.json to recognize a valid framework.
            for f in "$BUILD_OUTPUT"/*.dll "$BUILD_OUTPUT"/*.so "$BUILD_OUTPUT"/*.json; do
                [ -e "$f" ] && cp "$f" "$FRAMEWORK_DIR/$(basename "$f")"
            done
            # Also copy hidden files like .version that hostfxr may need
            for f in "$BUILD_OUTPUT"/.[!.]* ; do
                [ -e "$f" ] && cp "$f" "$FRAMEWORK_DIR/$(basename "$f")"
            done
            echo "Success: Configured for component hosting (framework $FRAMEWORK_VERSION, $(ls -a "$FRAMEWORK_DIR" | wc -l) copied files)"
        fi
    fi

    echo "Success: Built dotnet-core-CSharp-extension for $BUILD_CONFIGURATION configuration."
done

exit 0
