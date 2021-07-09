# .NET Core C# Language Extension

## Getting Started
Language Extensions is a feature of SQL Server used for executing external code. The relational data can be used in the external code using the extensibility framework.

For more information about SQL Server Language Extensions, refer to this [documentation](https://docs.microsoft.com/en-us/sql/language-extensions/language-extensions-overview?view=sql-server-ver15).

The dotnet-core-C#-extension version in this repository is compatible with SQL Server 2019 CU3 onwards. It integrates .NET core in SQL Server and works with .NET Core >= v3.1.

To use this dotnet-core-C#-lang-extension.zip package, follow [this tutorial](https://docs.microsoft.com/en-us/sql/machine-learning/install/custom-runtime-r?view=sql-server-ver15). For any fixes or enhancements, you are welcome to modify, rebuild and use the binaries using the following instructions.

## Building

### Windows
1. Install latest Visual Studio (https://visualstudio.microsoft.com/vs/) selecting the following required workloads: ".NET desktop development", "Desktop Development with C++" and ".NET core cross platform".

2. Run [build-dotnet-core-C#-extension.cmd debug release](./build-dotnet-core-C#-extension.cmd) which will generate: \
        - PATH\TO\ENLISTMENT\build-output\dotnet-core-C#-extension\windows\debug\nativecsharpextension.dll \
        - PATH\TO\ENLISTMENT\build-output\dotnet-core-C#-extension\windows\debug\hostfxr.dll \
        - PATH\TO\ENLISTMENT\build-output\dotnet-core-C#-extension\windows\debug\Microsoft.SqlServer.CSharpExtension.dll \
        - PATH\TO\ENLISTMENT\build-output\dotnet-core-C#-extension\windows\debug\Microsoft.SqlServer.CSharpExtension.runtimeconfig.json 

3. Run [create-dotnet-core-CSharp-extension-zip.cmd](./create-dotnet-core-CSharp-extension-zip.cmd) which will generate: \
        - PATH\TO\ENLISTMENT\build-output\dotnet-core-C#-extension\target\debug\dotnet-core-C#-lang-extension.zip

### Linux
TODO

## Testing (Optional)

TODO

## Usage
After downloading or building the dotnet-core-C#-lang-extension.zip, use [CREATE EXTERNAL LANGUAGE](https://docs.microsoft.com/en-us/sql/t-sql/statements/create-external-language-transact-sql?view=sql-server-ver15) to register the language with SQL Server 2019 CU3+.

This [tutorial](https://docs.microsoft.com/en-us/sql/machine-learning/install/custom-runtime-r?view=sql-server-ver15) will walk you through an end to end sample using the R language extension.
