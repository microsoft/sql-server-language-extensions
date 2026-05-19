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

    # Determine C++ compiler.
    # Prefer g++-11 (Ubuntu 22.04 toolchain) to match the SQL Server runtime
    # container's glibc 2.35 / libstdc++ GLIBCXX_3.4.30 ABI. Newer compilers
    # (Ubuntu 24.04 / gcc 13) generate references to GLIBC_2.38 symbols
    # (e.g. __isoc23_strtoul) that don't exist in the runtime, causing
    # dlopen() to fail silently and the extension to never load.
    if [ -z "${CXX:-}" ]; then
        if command -v g++-11 &>/dev/null; then
            CXX="g++-11"
        else
            CXX="c++"
        fi
    fi
    echo "[Info] Using C++ compiler: $CXX ($($CXX --version | head -1))"

    # Build compiler arguments
    CC_ARGS=(
        -shared
        -fPIC
        -fshort-wchar
        -std=c++17
        -o libnativecsharpextension.so
        "-I$DOTNET_NATIVE_INCLUDE"
        "-I$EXTENSION_HOST_INCLUDE"
        # Statically link libstdc++ and libgcc so the resulting .so does NOT
        # depend on the build host's libstdc++.so.6 version.
        # Required because the build hosts (Ubuntu 24.04 / GCC 13+) introduce
        # symbols like _ZSt21ios_base_library_initv@GLIBCXX_3.4.32 that don't
        # exist on the runtime hosts (SQL Server containers may ship older
        # libstdc++.so.6). Without these flags, dlopen() fails with:
        #   "version `GLIBCXX_3.4.32' not found"
        # and the extension never loads.
        -static-libstdc++
        -static-libgcc
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
            # Copy DLLs and SOs from the self-contained publish output to the framework dir.
            for f in "$BUILD_OUTPUT"/*.dll "$BUILD_OUTPUT"/*.so; do
                [ -e "$f" ] && cp "$f" "$FRAMEWORK_DIR/$(basename "$f")"
            done
            # hostfxr requires Microsoft.NETCore.App.deps.json to recognize a valid
            # framework version directory. This file is NOT in the self-contained publish
            # output -- it only exists in the .NET SDK's shared framework directory.
            # Find and copy it from the SDK installation.
            DOTNET_SDK_ROOT=$(dirname "$(dirname "$(command -v dotnet)")")
            SDK_FX_DIR="$DOTNET_SDK_ROOT/shared/Microsoft.NETCore.App/$FRAMEWORK_VERSION"
            if [ ! -d "$SDK_FX_DIR" ]; then
                # Try the standard /usr/share/dotnet location
                SDK_FX_DIR="/usr/share/dotnet/shared/Microsoft.NETCore.App/$FRAMEWORK_VERSION"
            fi
            if [ -f "$SDK_FX_DIR/Microsoft.NETCore.App.deps.json" ]; then
                cp "$SDK_FX_DIR/Microsoft.NETCore.App.deps.json" "$FRAMEWORK_DIR/"
                echo "  Copied Microsoft.NETCore.App.deps.json from SDK"
            else
                echo "WARNING: Microsoft.NETCore.App.deps.json not found in SDK at $SDK_FX_DIR"
                # As a fallback, search for it under dotnet root
                FOUND_DEPS=$(find /usr -name "Microsoft.NETCore.App.deps.json" -path "*/$FRAMEWORK_VERSION/*" 2>/dev/null | head -1)
                if [ -n "$FOUND_DEPS" ]; then
                    cp "$FOUND_DEPS" "$FRAMEWORK_DIR/"
                    echo "  Found and copied deps.json from $FOUND_DEPS"
                fi
            fi
            # Also copy .version if available
            if [ -f "$SDK_FX_DIR/.version" ]; then
                cp "$SDK_FX_DIR/.version" "$FRAMEWORK_DIR/"
            fi
            echo "Success: Configured for component hosting (framework $FRAMEWORK_VERSION, $(ls -a "$FRAMEWORK_DIR" | wc -l) files)"
        fi
    fi

    echo "Success: Built dotnet-core-CSharp-extension for $BUILD_CONFIGURATION configuration."
done

exit 0
