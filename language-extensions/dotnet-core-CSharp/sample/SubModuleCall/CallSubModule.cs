//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CallSubModule.cs
//
// Purpose:
//  Sample C# code which can dynamically load a class in the SQL Server C# Language Extension. 
//  This can be used to wrap around different functionality to be extended on top of SQL server.
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
    /// This can be used to call any custom DLL to further extend the functionality of the C# Language Extension.
    /// </summary>  
    public class CallSubModule: AbstractSqlServerExtensionExecutor
    {
        /// <summary>
        /// This method overrides the Execute method from AbstractSqlServerExtensionExecutor.
        /// </summary>
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams)
        {
            // The following example demonstrates how to call any C# program/method from an external DLL. 
            // Access needs to be granted to respective folder before you execute these commands. 
            // To grant access, run the following commands in a Windows command prompt. 
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

            // Template: var DLL = Assembly.LoadFile(@"<Path to DLL>");
            //Example
            //Object DLL = Assembly.LoadFile(@"C:\Program Files\Microsoft SQL Server\MSSQL16.SQLSERVER2022\MSSQL\ExternalLibraries\6\65537\1\SubModule.dll");
            Object DLL = sqlParams["@dllLocation"];

            //Template : Type type = DLL.GetType("<Class Name >");
            //Example Type type = DLL.GetType("UserExecutor.HelloWorld");
            Type type = DLL.GetType(sqlParams["@className"]);

            //Template : MethodInfo mi = type.GetMethod("<method name>");
            //Example MethodInfo mi = type.GetMethod("printConsole");
            MethodInfo mi = type.GetMethod(sqlParams["@methodName"]);

            // Create empty output DataFrame with One column
            //
            DataFrame output = new DataFrame(new StringDataFrameColumn("text", 0));

            if (mi != null)
            {
                object result = null;
                ParameterInfo[] parameters = mi.GetParameters();
                object classInstance = Activator.CreateInstance(type, null);
                if (parameters.Length == 0)
                {
                    result = mi.Invoke(classInstance, null);
                    output.append("Method invoked Successfully");
                }
            }

            // Modify the parameters
            //
            sqlParams["@rowsCount"] = output.Rows.Count;
            sqlParams["@Status"] = "Success!";

            return output;
        }
    }
}
