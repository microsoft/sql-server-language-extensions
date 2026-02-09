# .NET Core CSharp Language Extension

## Getting Started
Language Extensions is a feature of SQL Server used for executing external code. The relational data can be used in the external code using the extensibility framework.

For more information about SQL Server Language Extensions, refer to this [documentation](https://docs.microsoft.com/en-us/sql/language-extensions/language-extensions-overview?view=sql-server-ver15).

The dotnet-core-CSharp-extension version in this repository is compatible with SQL Server 2019 CU3 onwards. It integrates .NET core in SQL Server and works with .NET 6.0 in **Windows only**.

Currently, the extension supports the following data types: SQL_C_SLONG, SQL_C_ULONG, SQL_C_SSHORT, SQL_C_USHORT, SQL_C_SBIGINT, SQL_C_UBIGINT, SQL_C_STINYINT, SQL_C_UTINYINT, SQL_C_BIT, SQL_C_FLOAT, SQL_C_DOUBLE, SQL_C_CHAR, and SQL_C_WCHAR. It supports the following SQL data types: int, bigint, smallint, tinyint, real, float, bit, char(n), varchar(n), nchar(n), and nvarchar(n).

To use this dotnet-core-CSharp-lang-extension.zip package, follow [this tutorial](./sample/regex/README.md). For any fixes or enhancements, you are welcome to modify, rebuild and use the binaries using the following instructions.

## Building

### Windows
1. Install latest [Visual Studio](https://visualstudio.microsoft.com/vs/) selecting the following required workloads: ".NET desktop development", "Desktop Development with C++" and ".NET core cross platform".

2. Run [restore-packages.cmd](./build/windows/restore-packages.cmd) which will restore the packages needed for the extension

3. Run 'dotnet restore <PATH\TO\ENLISTMENT\language-extensions\dotnet-core-CSharp\src\managed\Microsoft.SqlServer.CSharpExtension.csproj\Microsoft.SqlServer.CSharpExtension.csproj>'

4. Run [build-dotnet-core-CSharp-extension.cmd](./build/windows/build-dotnet-core-CSharp-extension.cmd) which will generate: \
        - PATH\TO\ENLISTMENT\build-output\dotnet-core-CSharp-extension\windows\release\nativecsharpextension.dll \
        - PATH\TO\ENLISTMENT\build-output\dotnet-core-CSharp-extension\windows\release\hostfxr.dll \
        - PATH\TO\ENLISTMENT\build-output\dotnet-core-CSharp-extension\windows\release\Microsoft.SqlServer.CSharpExtension.dll \
        - PATH\TO\ENLISTMENT\build-output\dotnet-core-CSharp-extension\windows\release\Microsoft.SqlServer.CSharpExtension.runtimeconfig.json\
        - PATH\TO\ENLISTMENT\build-output\dotnet-core-CSharp-extension\windows\release\Microsoft.SqlServer.CSharpExtension.deps.json

5. Run [create-dotnet-core-CSharp-extension-zip.cmd](./build/windows/create-dotnet-core-CSharp-extension-zip.cmd) which will generate: \
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

## Output Schema Support

By default, output column types are inferred from the .NET DataFrame column types. For string columns, you can explicitly specify the SQL data type using the `OutputColumnDataTypes` property.

### Specifying Output Column Types

Use `OutputColumnDataTypes` to specify the SQL data type for output columns by name:

```csharp
using Microsoft.SqlServer.CSharpExtension.SDK;
using Microsoft.Data.Analysis;
using static Microsoft.SqlServer.CSharpExtension.Sql;

public class MyExecutor : AbstractSqlServerExtensionExecutor
{
    public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams)
    {
        // Specify NVARCHAR (UTF-16) output for a string column
        OutputColumnDataTypes["unicode_column"] = SqlDataType.DotNetWChar;
        
        // Process and return data
        return resultDataFrame;
    }
}
```

### Supported String Types

| SqlDataType | SQL Type | Encoding | Description |
|-------------|----------|----------|-------------|
| `SqlDataType.DotNetChar` | VARCHAR | UTF-8 | Default for string columns |
| `SqlDataType.DotNetWChar` | NVARCHAR | UTF-16 | Use for Unicode data |

### Example: Mixed VARCHAR and NVARCHAR Output

```csharp
public class MixedOutputExecutor : AbstractSqlServerExtensionExecutor
{
    public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams)
    {
        // "ascii_col" will default to VARCHAR (no configuration needed)
        
        // "unicode_col" should be NVARCHAR
        OutputColumnDataTypes["unicode_col"] = SqlDataType.DotNetWChar;
        
        return input;
    }
}
```

### Default Behavior

If no explicit type is specified for a string column:
- String columns default to `DotNetChar` (VARCHAR/UTF-8)
- Numeric and other types are automatically mapped from their .NET types
