# SubModule Call
 This folder contains an implementation which will help in Calling any functionality which has been embeded inside an external DLL. Imagine that you already have a DLL which has a lot of functionalities built into it and you would like to reuse all those features in SQL Server Language Extension. This particualar example tries to address this exact requirements. To represent the custom build, we have create a simple project , which has been placed in SubModuleProject folder.This project just has a console out statement. 
 
 ## Prerequisites
 * SubModuleProject\SubModule\Program.cs: This is a sample custom code with simple console log.  
 * SubModuleProject\SubModule\SubModule.csproj: This is the .NET project file which encapsulated the custom code. 
 * SubModuleProject\SubModule\SubModule.sln : Solution file for the custom project. 
 *  CallSubModule.cs : This is the actual module which has the implementation to call a external DLL. Modify this code to point to the right folder where the external DLLs are stored. 

## Implementation Steps

1) Download/clone the [SubModuleProject](./SubModuleProject/SubModule/) 
2) Compile the project [SubModule.sln](./SubModuleProject/SubModule/SubModule.sln). This will generete a DLL file named SubModule.dll
3) Move this DLL file to respective folder of SQL server Langugage extension.
4) Grant access to the DLL once it is moved.Below are the commands to grant access. Run these in a command prompt window. 

  * icacls "C:\Program Files\Microsoft SQL Server\MSSQL16.SQLSERVER2022\MSSQL\ExternalLibraries\6\65537\1" /grant "SQLRUsergroupSQLSERVER2022":(OI)(CI)RX /T  
    * icacls "C:\Program Files\Microsoft SQL Server\MSSQL16.SQLSERVER2022\MSSQL\ExternalLibraries\6\65537\1"  /grant *S-1-15-2-1:(OI)(CI)RX /T 
5) Modify the code in [CallSubModule.cs](./CallSubModule.cs). You will have to just update the folder name here along with DLL.
6) Follow the standard method to outlined in documentation to moved the DLL code into Language extension folder.
7) Execute the code through a stored procedure call. 
8) Once your run it you should be able to see the console output in SSMS. 

