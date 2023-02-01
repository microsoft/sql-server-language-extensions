# .NET Core CSharp Language Extension

## Getting Started
Language Extensions is a feature of SQL Server used for executing external code. The relational data can be used in the external code using the extensibility framework.

For more information about SQL Server Language Extensions, refer to this [documentation](https://docs.microsoft.com/en-us/sql/language-extensions/language-extensions-overview?view=sql-server-ver15).

The dotnet-core-CSharp-extension version in this repository is compatible with SQL Server 2019 CU3 onwards. It integrates .NET core in SQL Server and works with .NET 6.0 in **Windows only**.

Currently, the extension supports the following data types: SQL_C_SLONG, SQL_C_ULONG, SQL_C_SSHORT, SQL_C_USHORT, SQL_C_SBIGINT, SQL_C_UBIGINT, SQL_C_STINYINT, SQL_C_UTINYINT, SQL_C_BIT, SQL_C_FLOAT, SQL_C_DOUBLE, SQL_C_CHAR. It supports the following SQL data types: int, bigint, smallint, tinyint, real, float, bit, varchar(n).

To use this dotnet-core-CSharp-lang-extension.zip package, follow [this tutorial](./sample/regex/README.md). For any fixes or enhancements, you are welcome to modify, rebuild and use the binaries using the following instructions.

## Building

### Windows
1. Install latest [Visual Studio](https://visualstudio.microsoft.com/vs/) selecting the following required workloads: ".NET desktop development", "Desktop Development with C++" and ".NET core cross platform".

2. Run [restore-packages.cmd](./build/windows/restore-packages.cmd) which will restore the packages needed for the extension

3. Run [build-dotnet-core-CSharp-extension.cmd](./build/windows/build-dotnet-core-CSharp-extension.cmd) which will generate: \
        - PATH\TO\ENLISTMENT\build-output\dotnet-core-CSharp-extension\windows\release\nativecsharpextension.dll \
        - PATH\TO\ENLISTMENT\build-output\dotnet-core-CSharp-extension\windows\release\hostfxr.dll \
        - PATH\TO\ENLISTMENT\build-output\dotnet-core-CSharp-extension\windows\release\Microsoft.SqlServer.CSharpExtension.dll \
        - PATH\TO\ENLISTMENT\build-output\dotnet-core-CSharp-extension\windows\release\Microsoft.SqlServer.CSharpExtension.runtimeconfig.json\
        - PATH\TO\ENLISTMENT\build-output\dotnet-core-CSharp-extension\windows\release\Microsoft.SqlServer.CSharpExtension.deps.json

4. Run [create-dotnet-core-CSharp-extension-zip.cmd](./build/windows/create-dotnet-core-CSharp-extension-zip.cmd) which will generate: \
        - PATH\TO\ENLISTMENT\build-output\dotnet-core-CSharp-extension\target\debug\dotnet-core-CSharp-lang-extension.zip
        This zip can be used in CREATE EXTERNAL LANGUAGE, as detailed in the tutorial in the Usage section below.

### Linux
Not Supported.

## Testing (Optional)

### Windows
To unit test this extension,
1. Install [CMake](https://cmake.org/download/) for Windows. Set `CMAKE_ROOT` to point to the cmake installation folder.
2. Run [build-dotnet-core-CSharp-extension-test.cmd](./test/build/windows/build-dotnet-core-CSharp-extension-test.cmd) which will generate: \
PATH\TO\ENLISTMENT\build-output\dotnet-core-CSharp-extension-test\windows\release\dotnet-core-CSharp-extension-test.exe
3. Run [run-dotnet-core-CSharp-extension-test.cmd](./test/build/windows/run-dotnet-core-CSharp-extension-test.cmd) to run all the unit tests.

### Linux
Not Supported.

## Usage
After downloading or building the dotnet-core-CSharp-lang-extension.zip, use [CREATE EXTERNAL LANGUAGE](https://docs.microsoft.com/en-us/sql/t-sql/statements/create-external-language-transact-sql?view=sql-server-ver15) to register the language with SQL Server 2019 CU3+.

This [tutorial](./sample/regex/README.md) will walk you through an end to end sample using the .NET Core C# language extension.
