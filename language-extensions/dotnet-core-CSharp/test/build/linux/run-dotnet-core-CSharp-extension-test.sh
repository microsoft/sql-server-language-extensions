#!/usr/bin/env bash
set -euo pipefail

# Set environment variables
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ENL_ROOT="$(cd "$SCRIPT_DIR/../../../../.." && pwd)"
DOTNETCORE_CSHARP_EXTENSION_TEST_WORKING_DIR="$ENL_ROOT/build-output/dotnet-core-CSharp-extension-test/linux"

# Default to release if no arguments
if [ $# -eq 0 ]; then
    set -- "release"
fi

for CMAKE_CONFIGURATION in "$@"; do
    CMAKE_CONFIGURATION="$(echo "$CMAKE_CONFIGURATION" | tr '[:upper:]' '[:lower:]')"
    if [ "$CMAKE_CONFIGURATION" != "debug" ]; then
        CMAKE_CONFIGURATION="release"
    fi

    TEST_DIR="$DOTNETCORE_CSHARP_EXTENSION_TEST_WORKING_DIR/$CMAKE_CONFIGURATION"
    pushd "$TEST_DIR" > /dev/null

    echo "[INFO] Running dotnet-core-CSharp-extension tests for $CMAKE_CONFIGURATION configuration"

    ./dotnet-core-CSharp-extension-test \
        --gtest_output=xml:"$ENL_ROOT/out/TestReport_dotnet-core-csharp-extension-test.xml"

    popd > /dev/null
done

exit 0
