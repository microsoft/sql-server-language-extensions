//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: DllUtils.cs
//
// Purpose:
//  Provides methods needed to find the user dll and class
//
//*********************************************************************
using System;
using System.Linq;
using System.IO;
using System.Reflection;
using System.Collections.Generic;
using Microsoft.SqlServer.CSharpExtension.SDK;

namespace Microsoft.SqlServer.CSharpExtension
{
    /// <summary>
    /// This class contains method to find the user dll and class.
    /// </summary>
    internal class DllUtils
    {
        /// <summary>
        /// This method loops through all dll in the paths and returns the first class that implements the executor.
        /// </summary>
        /// <param name="userClassName">
        /// The full name of the user class in the form of namespace.classname
        /// </param>
        /// <param name="dllList">
        /// A list containing all the dlls under the public and private library path
        /// </param>
        public static Type GetUserDll(string userClassName, List<string> dllList)
        {
            // AppDomain.CurrentDomain.AssemblyResolve occurs when the resolution of an assembly fails.
            //
            AppDomain.CurrentDomain.AssemblyResolve += AssemblyResolve;
            foreach(string dllPath in dllList)
            {
                if (Path.GetFileName(dllPath).StartsWith("gtest", StringComparison.OrdinalIgnoreCase))
                {
                    continue;
                }

                // Catch unexpected exception while loading other dlls
                //
                try
                {
                    Assembly userDll = Assembly.LoadFrom(dllPath);
                    Type userExecutorClass = userDll.GetType(userClassName);
                    if (userExecutorClass != null)
                    {
                        return userExecutorClass;
                    }
                }
                catch (BadImageFormatException)
                {
                    // Ignore BadImageFormatException (e.g. native DLLs)
                }
                catch (Exception e)
                {
                    // Catch unexpected exception without throwing the exception so that
                    // all the dlls will be loaded to find matched user executor
                    //
                    Logging.Error(e.StackTrace + "Error: " + e.Message);
                }
            }

            if (dllList.Count == 0)
            {
                string msg = "Unable to find user dll under " + string.Join(", ", dllList);
                throw new Exception(msg);
            }

            string msg2 = "Unable to find user class with full name: " + userClassName + "\nPlease provide user class in the form of LibraryName;Namespace.Classname or Namespace.Classname";
            throw new Exception(msg2);
        }

        /// <summary>
        /// This method creates a list of paths to all the dlls under the library paths.
        /// </summary>
        /// <param name="publicPath">
        /// Public external library path
        /// </param>
        /// <param name="privatePath">
        /// Private external library path
        /// </param>
        public static List<string> CreateDllList(
            string publicPath,
            string privatePath,
            string userLibName)
        {
            List<string>dllList = new List<string>();
            if(string.IsNullOrEmpty(userLibName))
            {
                if (!string.IsNullOrEmpty(privatePath))
                {
                    dllList.AddRange(Directory.GetFiles(privatePath));
                }

                if (!string.IsNullOrEmpty(publicPath))
                {
                    dllList.AddRange(Directory.GetFiles(publicPath));
                }
            }
            else
            {
                if (!string.IsNullOrEmpty(privatePath))
                {
                    dllList.AddRange(Directory.GetFiles(privatePath, userLibName));
                }

                if (!string.IsNullOrEmpty(publicPath))
                {
                    dllList.AddRange(Directory.GetFiles(publicPath, userLibName));
                }
            }

            if (dllList.Count == 0)
            {
                throw new Exception("Unable to find user dll under " + publicPath + " and " + privatePath);
            }

            return dllList;
        }

        /// <summary>
        /// This method finds the corresponding loaded dll for user dll's dependencies.
        /// It searches for the corresponding loaded dll that matches args.Name.
        /// </summary>
        private static Assembly AssemblyResolve(object sender, ResolveEventArgs args)
        {
            return AppDomain.CurrentDomain.GetAssemblies().Where(a => a.FullName == args.Name).SingleOrDefault();
        }
    }
}
