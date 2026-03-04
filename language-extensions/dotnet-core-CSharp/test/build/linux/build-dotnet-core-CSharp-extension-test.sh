#!/usr/bin/env bash
set -euo pipefail

# Set environment variables
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ENL_ROOT="$(cd "$SCRIPT_DIR/../../../../.." && pwd)"
DOTNETCORE_CSHARP_EXTENSION_TEST_HOME="$ENL_ROOT/language-extensions/dotnet-core-CSharp/test"
DOTNETCORE_CSHARP_EXTENSION_TEST_WORKING_DIR="$ENL_ROOT/build-output/dotnet-core-CSharp-extension-test/linux"

# Clean and create build working directory
rm -rf "$DOTNETCORE_CSHARP_EXTENSION_TEST_WORKING_DIR"
mkdir -p "$DOTNETCORE_CSHARP_EXTENSION_TEST_WORKING_DIR"

# Default to release if no arguments
if [ $# -eq 0 ]; then
    set -- "release"
fi

for CMAKE_CONFIGURATION in "$@"; do
    CMAKE_CONFIGURATION="$(echo "$CMAKE_CONFIGURATION" | tr '[:upper:]' '[:lower:]')"
    if [ "$CMAKE_CONFIGURATION" != "debug" ]; then
        CMAKE_CONFIGURATION="release"
    fi

    BUILD_OUTPUT="$DOTNETCORE_CSHARP_EXTENSION_TEST_WORKING_DIR/$CMAKE_CONFIGURATION"
    mkdir -p "$BUILD_OUTPUT"
    pushd "$BUILD_OUTPUT" > /dev/null

    echo "[INFO] Generating dotnet-core-CSharp-extension test project build files using CMAKE_CONFIGURATION=$CMAKE_CONFIGURATION"

    # Call cmake to generate makefiles
    cmake \
        -DCMAKE_INSTALL_PREFIX:PATH="$DOTNETCORE_CSHARP_EXTENSION_TEST_WORKING_DIR/$CMAKE_CONFIGURATION" \
        -DENL_ROOT="$ENL_ROOT" \
        -DCMAKE_CONFIGURATION="$CMAKE_CONFIGURATION" \
        -DPLATFORM=linux \
        "$DOTNETCORE_CSHARP_EXTENSION_TEST_HOME/src/native"

    echo "[INFO] Building dotnet-core-CSharp-extension test project using CMAKE_CONFIGURATION=$CMAKE_CONFIGURATION"

    # Build managed test executor
    dotnet build \
        "$DOTNETCORE_CSHARP_EXTENSION_TEST_HOME/src/managed/Microsoft.SqlServer.CSharpExtensionTest.csproj" \
        -m \
        -c "$CMAKE_CONFIGURATION" \
        -o "$BUILD_OUTPUT" \
        --no-dependencies

    # Delete Microsoft.SqlServer.CSharpExtension.dll to avoid test executor referencing it
    # instead of the extension itself
    rm -f "$BUILD_OUTPUT/Microsoft.SqlServer.CSharpExtension.dll"

    # Build native test executable
    cmake --build . --config "$CMAKE_CONFIGURATION" --target install

    popd > /dev/null

    echo "Success: Built dotnet-core-CSharp-extension-test for $CMAKE_CONFIGURATION configuration."
done

exit 0
