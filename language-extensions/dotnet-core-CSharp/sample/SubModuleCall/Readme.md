# SubModule Call
 This folder contains an implementation which will help in calling any functionality which has been embedded inside an external DLL. Imagine that you already have a DLL which has a lot of functionality built into it and you would like to reuse all those features in SQL Server Language Extension. This submodule example enables you to reuse your existing code. To represent the custom build, we have created a simple project, which has been placed in SubModuleProject folder. This project just has a console out statement. 
 ## Prerequisites
 * SubModuleProject\SubModule\Program.cs: This is a sample custom code with simple console log.  
 * SubModuleProject\SubModule\SubModule.csproj: This is the .NET project file which encapsulated the custom code. 
 * SubModuleProject\SubModule\SubModule.sln : Solution file for the custom project. 
 *  CallSubModule.cs : This is the actual module which has the implementation to call a external DLL. Modify this code to point to the right folder where the external DLLs are stored. 

## Implementation Steps

1. Download/clone the SubModuleProject . Lets assume that you have cloned the repo in a folder named "C:\Dev\language-extensions"
```bash

git clone https://github.com/microsoft/sql-server-language-extensions.git 

```

2. Compile the project SubModule.sln. This will generete a DLL file named SubModule.dll
```bash

dotnet build C:\Dev\language-extensions\language-extensions\dotnet-core-CSharp\sample\SubModuleCall\SubModuleProject\SubModule\SubModule.csproj

```

3. Move this DLL file to the respective folder of the SQL Server Language Extension.
```bash

copy "C:\Dev\language-extensions\language-extensions\dotnet-core-CSharp\sample\SubModuleCall\SubModuleProject\SubModule\bin\Debug\net8.0\SubModule.dll"  "C:\Program Files\Microsoft SQL Server\MSSQL16.SQLSERVER2022\MSSQL\ExternalLibraries\6\65537\1"


```
4. Grant access to the DLL once it is moved. Below are the commands to grant access. Run these in a command prompt window.

```bash

icacls "C:\Program Files\Microsoft SQL Server\MSSQL16.SQLSERVER2022\MSSQL\ExternalLibraries\6\65537\1" /grant "SQLRUsergroupSQLSERVER2022":(OI)(CI)RX /T
icacls "C:\Program Files\Microsoft SQL Server\MSSQL16.SQLSERVER2022\MSSQL\ExternalLibraries\6\65537\1"  /grant *S-1-15-2-1:(OI)(CI)RX /T

```

5) Call [CallSubModule.cs](./CallSubModule.cs) using the below sample code . 
```sql

declare @rowsCount int
declare @dllLocation varchar(200)
declare @className varchar(30)
declare @methodName varchar(30)
declare @Status varchar(30)
set @dllLocation = N'C:\Program Files\Microsoft SQL Server\MSSQL16.SQLSERVER2022\MSSQL\ExternalLibraries\6\65537\1\SubModule.dll'
set @className = N'UserExecutor.HelloWorld'
set @methodName = N'printConsole'

EXEC sp_execute_external_script
  @language = N'Dotnet'
, @script = N'UserExecutor.CallSubModule'
, @params = N'@dllLocation varchar(200),@className varchar(30),@methodName varchar(30), @Status varchar(20) OUTPUT, @rowsCount int OUTPUT'
, @Status =  @Staus OUTPUT
, @rowsCount = @rowsCount OUTPUT
with result sets (( text varchar(100)));

select @rowsCount as rowsCount, @Status as message

```

6) Once you run it you should be able to see the console output in SSMS. 

