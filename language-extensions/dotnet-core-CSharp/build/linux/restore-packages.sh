#!/bin/bash

function check_exit_code {
    EXIT_CODE=$?
    if [ ${EXIT_CODE} -eq 0 ]; then
        echo $1
    else
        echo $2
        exit ${EXIT_CODE}
    fi
}

# Install .NET SDK 8.0 for building the C# extension managed code
# and for obtaining libnethost.a (used by the native host to discover hostfxr).
#

# Check if dotnet is already installed and is version 8.x
if command -v dotnet &>/dev/null && dotnet --version 2>/dev/null | grep -q "^8\."; then
    echo "Info: .NET SDK 8.x already installed ($(dotnet --version))"
else
    echo "Info: Installing .NET SDK 8.0..."
    apt-get update
    apt-get install -y --no-install-recommends wget apt-transport-https
    wget https://dot.net/v1/dotnet-install.sh -O /tmp/dotnet-install.sh
    chmod +x /tmp/dotnet-install.sh
    /tmp/dotnet-install.sh --channel 8.0 --install-dir /usr/share/dotnet
    ln -sf /usr/share/dotnet/dotnet /usr/bin/dotnet
    check_exit_code "Success: Installed .NET SDK 8.0" "Error: Failed to install .NET SDK 8.0"
fi

# Copy libnethost.a from the .NET SDK runtime packs into the extension's lib directory.
# The SDK ships this as part of the AppHost pack.
#
SCRIPTDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
ENL_ROOT=${SCRIPTDIR}/../../../..
DOTNET_EXTENSION_HOME=${ENL_ROOT}/language-extensions/dotnet-core-CSharp

NETHOST_DIR=$(find /usr/share/dotnet -path "*/runtimes/linux-x64/native/libnethost.a" -print -quit 2>/dev/null)
if [ -z "$NETHOST_DIR" ]; then
    echo "Error: libnethost.a not found in .NET SDK. Ensure .NET SDK 8.0 is installed."
    exit 1
fi

echo "Info: Found libnethost.a at: $NETHOST_DIR"
mkdir -p "${DOTNET_EXTENSION_HOME}/lib"
cp "$NETHOST_DIR" "${DOTNET_EXTENSION_HOME}/lib/libnethost.a"
check_exit_code "Success: Copied libnethost.a to extension lib directory" "Error: Failed to copy libnethost.a"

# Pre-restore NuGet packages while network is still available.
# OneBranch enables network isolation after package-restore phases,
# so dotnet restore must happen here rather than during the build step.
#
MANAGED_PROJ="${DOTNET_EXTENSION_HOME}/src/managed/Microsoft.SqlServer.CSharpExtension.csproj"
echo "Info: Restoring NuGet packages for Microsoft.SqlServer.CSharpExtension..."
dotnet restore "$MANAGED_PROJ"
check_exit_code "Success: NuGet packages restored" "Error: Failed to restore NuGet packages"

exit $?
