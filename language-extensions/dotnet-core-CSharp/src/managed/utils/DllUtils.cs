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
        /// This method finds the first user class that implements the executor and the correct namespace.
        /// </summary>
        /// <param name="userClassName">
        /// The namespace of User Dll
        /// </param>
        /// <param name="interfaceToImplement">
        /// The type of the base interface that implemented by the user dll
        /// </param>
        /// <param name="types">
        /// The types of all the classes of the user dll
        /// </param>
        private static Type GetClassThatImplements(string userClassName, Type interfaceToImplement, IEnumerable<Type> types)
        {
            // Looks for the class that has the same namespace as the user namespace and the implemented interface can be assignable from the user interface
            return types.FirstOrDefault(t => t.FullName == userClassName && interfaceToImplement.IsAssignableFrom(t) && !t.IsInterface && !t.IsAbstract);
        }

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
                // Catch unexpected exception while loading other dlls
                //
                try
                {
                    Assembly userDll = Assembly.LoadFrom(dllPath);
                    Type userExecutorClass = DllUtils.GetClassThatImplements(userClassName, typeof(AbstractSqlServerExtensionExecutor), userDll.GetExportedTypes());
                    if (userExecutorClass != null)
                    {
                        return userExecutorClass;
                    }
                }
                catch (Exception e)
                {
                    // Catch unexpected exception without throwing the exception so that
                    // all the dlls will be loaded to find matched user executor
                    //
                    Logging.Error(e.StackTrace + "Error: " + e.Message);
                }
            }

            throw new Exception("Unable to find user class with full name: " + userClassName);
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
        public static List<string> CreateDllList(string publicPath, string privatePath)
        {
            List<string>dllList = new List<string>();
            if (!string.IsNullOrEmpty(publicPath))
            {
                dllList.AddRange(Directory.GetFiles(publicPath).Where(s => s.EndsWith(".dll")));
            }

            if (!string.IsNullOrEmpty(privatePath))
            {
                dllList.AddRange(Directory.GetFiles(privatePath).Where(s => s.EndsWith(".dll")));
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