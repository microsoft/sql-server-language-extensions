//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpUserDll.cs
//
// Purpose:
//  Class loading user dll and getting user executor
//
//*********************************************************************
using System;
using System.Collections.Generic;
using Microsoft.SqlServer.CSharpExtension.SDK;

namespace Microsoft.SqlServer.CSharpExtension
{
    /// <summary>
    /// This class loads the user dll and gets user executor.
    /// </summary>
    internal class CSharpUserDll
    {
        /// <summary>
        /// The absolute path to the public library folder
        /// </summary>
        private string _publicPath;

        /// <summary>
        /// The absolute path to the private library folder
        /// </summary>
        private string _privatePath;

        /// <summary>
        /// The file name of user dll
        /// </summary>
        private string _userLibName;

        /// <summary>
        /// The full name of user class
        /// </summary>
        private string _userClassFullName;

        /// <summary>
        /// The user executor extending AbstractSqlServerExtensionExecutor
        /// </summary>
        public AbstractSqlServerExtensionExecutor UserExecutor { get; set; }

        /// <summary>
        /// This constructor creates CSharpUserDll class with public/private external
        /// library path and the user dll file name and full class name.
        /// </summary>
        /// <param name="publicPath">
        /// The absolute path to the public library folder
        /// </param>
        /// <param name="privatePath">
        /// The absolute path to the private library folder
        /// </param>
        /// <param name="userScriptFullName">
        /// The full name of the user class in the form of filename;namespace.classname or namespace.classname
        /// </param>
        public CSharpUserDll(string publicPath, string privatePath, string userScriptFullName)
        {
            _publicPath = publicPath;
            _privatePath = privatePath;
            if(!string.IsNullOrEmpty(userScriptFullName))
            {
                string[] subStr = userScriptFullName.Split(';');
                _userLibName = (subStr.Length == 2) ? subStr[0] : string.Empty;
                _userClassFullName = (subStr.Length == 2) ? subStr[1] : userScriptFullName;
            }
        }

        /// <summary>
        /// This method loads the user dll and returns the user class extending executor.
        /// </summary>
        public AbstractSqlServerExtensionExecutor InstantiateUserExecutor()
        {
            Logging.Trace("CSharpUserDll::InstantiateUserExecutor");
            if(string.IsNullOrEmpty(_userClassFullName) && string.IsNullOrEmpty(_userLibName))
            {
                return null;
            }

            List<string> dllList = DllUtils.CreateDllList(_publicPath, _privatePath, _userLibName);
            Type userExecutorClass = DllUtils.GetUserDll(_userClassFullName, dllList);
            return (AbstractSqlServerExtensionExecutor)Activator.CreateInstance(userExecutorClass);
        }
    }
}
