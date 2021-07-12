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
        /// The namespace of user dll
        /// </summary>
        private string _userNamespace;

        /// <summary>
        /// This constructor creates CSharpUserDll class with public/private external
        /// library path and the namespace of the user dll
        /// </summary>
        /// <param name="publicPath">
        /// The absolute path to the public library folder
        /// </param>
        /// <param name="privatePath">
        /// The absolute path to the private library folder
        /// </param>
        /// <param name="userNamespace">
        /// The namespace of the user class
        /// </param>
        public CSharpUserDll(string publicPath, string privatePath, string userNamespace)
        {
            _publicPath = publicPath;
            _privatePath = privatePath;
            _userNamespace = userNamespace;
        }

        /// <summary>
        /// This method loads the user dll and returns the user class extending executor.
        /// </summary>
        public AbstractSqlServerExtensionExecutor InstantiateUserExecutor()
        {
            Logging.Trace("CSharpUserDll::InstantiateUserExecutor");
            List<string> dllList = DllUtils.CreateDllList(_publicPath, _privatePath);
            Type userExecutorClass = DllUtils.GetUserDll(_userNamespace, dllList);
            return (AbstractSqlServerExtensionExecutor)Activator.CreateInstance(userExecutorClass);
        }
    }
}
