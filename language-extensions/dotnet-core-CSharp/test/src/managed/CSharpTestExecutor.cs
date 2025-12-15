//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpTestExecutor.cs
//
// Purpose:
//  Test classes extending the AbstractSqlServerExtensionExecutor.
//
//*********************************************************************
using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using Microsoft.Data.Analysis;
using Microsoft.SqlServer.CSharpExtension.SDK;

namespace Microsoft.SqlServer.CSharpExtensionTest
{
    public class CSharpTestExecutor: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            Console.WriteLine("Hello .NET Core CSharpExtension!");
            return input;
        }
    }

    public class CSharpTestExecutorIntParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            sqlParams["@param0"] = Int32.MaxValue;
            sqlParams["@param1"] = Int32.MinValue;
            sqlParams["@param2"] = 4;
            sqlParams["@param3"] = 0;
            sqlParams["@param4"] = null;
            return null;
        }
    }

    public class CSharpTestExecutorBitParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            sqlParams["@param0"] = true;
            sqlParams["@param1"] = false;
            sqlParams["@param2"] = 3;
            sqlParams["@param3"] = 0;
            sqlParams["@param4"] = null;
            return null;
        }
    }

    public class CSharpTestExecutorRealParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            sqlParams["@param0"] = 3.4e38F;
            sqlParams["@param1"] = -3.4e38F;
            sqlParams["@param2"] = 2.3e4;
            sqlParams["@param3"] = 0;
            sqlParams["@param4"] = null;
            return null;
        }
    }

    public class CSharpTestExecutorDoubleParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            sqlParams["@param0"] = 1.79e308;
            sqlParams["@param1"] = -1.79e308;
            sqlParams["@param2"] = 1.45e38;
            sqlParams["@param3"] = 0;
            sqlParams["@param4"] = null;
            return null;
        }
    }

    public class CSharpTestExecutorBigIntParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            sqlParams["@param0"] = Int64.MaxValue;
            sqlParams["@param1"] = Int64.MinValue;
            sqlParams["@param2"] = 9372036854775;
            sqlParams["@param3"] = 0;
            sqlParams["@param4"] = null;
            return null;
        }
    }

    public class CSharpTestExecutorSmallIntParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            sqlParams["@param0"] = Int16.MaxValue;
            sqlParams["@param1"] = Int16.MinValue;
            sqlParams["@param2"] = 3007;
            sqlParams["@param3"] = 0;
            sqlParams["@param4"] = null;
            return null;
        }
    }

    public class CSharpTestExecutorTinyIntParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            sqlParams["@param0"] = 255;
            sqlParams["@param1"] = 0;
            sqlParams["@param2"] = 123;
            sqlParams["@param3"] = -1;
            sqlParams["@param4"] = null;
            return null;
        }
    }

    public class CSharpTestExecutorStringParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            sqlParams["@param0"] = "HELLO";
            sqlParams["@param1"] = "C#Extension";
            sqlParams["@param2"] = "";
            sqlParams["@param3"] = "WORLD";
            sqlParams["@param4"] = null;
            sqlParams["@param5"] = null;
            return null;
        }
    }

    public class CSharpTestExecutorWStringParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            sqlParams["@param0"] = "HELLO";
            sqlParams["@param1"] = "C#Extension";
            sqlParams["@param2"] = "";
            sqlParams["@param3"] = "WORLD";
            sqlParams["@param4"] = null;
            sqlParams["@param5"] = null;
            return null;
        }
    }
}
