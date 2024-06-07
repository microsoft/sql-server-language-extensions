//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CallSubModule.cs
//
// Purpose:
//  Sample C# code which can dynamically load a class in SQL Server Language extension. 
//  This can be used to wrap around different functionalities  to be extended on top of SQL server.
//*********************************************************************
using Microsoft.Data.Analysis;
using Microsoft.SqlServer.CSharpExtension.SDK;
using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;

namespace UserExecutor
{
    /// <summary>
    /// This class extends the AbstractSqlServerExtensionExecutor.
    /// This can be used to call any custom DLL to achive any functionality in Langauge Extension.
    /// </summary>
    public class CallSubModule: AbstractSqlServerExtensionExecutor
    {
        /// <summary>
        /// This method overrides the Execute method from AbstractSqlServerExtensionExecutor.
        /// </summary>
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams)
        {
            // Below is an example to call any c# program/Method from an external DLL. 
            // Access needs to be granted to respective folder before you execute these commands. 
            // To grant access perform below commands in windows command prompt. 
            // Note: The folder name may change in your environment. So please specify the right folder name.
            // Command1: 
            //          icacls "C:\Program Files\Microsoft SQL Server\MSSQL16.SQLSERVER2022\MSSQL\ExternalLibraries\6\65537\1" /grant "SQLRUsergroupSQLSERVER2022":(OI)(CI)RX /T  
            // Change the server name according to your naming convension .
            // Command2:
            //          icacls "C:\Program Files\Microsoft SQL Server\MSSQL16.SQLSERVER2022\MSSQL\ExternalLibraries\6\65537\1"  /grant *S-1-15-2-1:(OI)(CI)RX /T
            // After executing above commands, SQL server will have access to local folder which has been specified in the commands. 
            // All these commands are available in  OneTimeSetupCommands.cmd file in this solution as well
            // The required DLL needs to be stored in the location where SQL server has access. 
            // These DLLs are dynamically loaded during runtime and any method inside of these DLLs can be executed on the fly. 
            // Below is an example of how we can do that with a sample Hello World Program.
            // Example: var DLL = Assembly.LoadFile(@"C:\Program Files\Microsoft SQL Server\MSSQL16.SQLSERVER2022\MSSQL\ExternalLibraries\6\65537\1\helloworlddll.dll");
            var DLL = Assembly.LoadFile(@"<Path to DLL>");
            //Example : Type type = DLL.GetType("UserExecutor.HelloWorld");
            Type type = DLL.GetType("<Class Name >");
            //Example: MethodInfo mi = type.GetMethod("printConsole");
            MethodInfo mi = type.GetMethod("<method name>");
            if (mi != null)
            {
                object result = null;
                ParameterInfo[] parameters = mi.GetParameters();
                object classInstance = Activator.CreateInstance(type, null);
                if (parameters.Length == 0)
                {
                    result = mi.Invoke(classInstance, null);
                }
            }
            return null;
        }
    }
}