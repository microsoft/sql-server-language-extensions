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
                catch (Exception e)
                {
                    // Catch unexpected exception without throwing the exception so that
                    // all the dlls will be loaded to find matched user executor
                    //
                    Logging.Error(e.StackTrace + "Error: " + e.Message);
                }
            }

            string msg = "Unable to find user class with full name: " + userClassName + "\nPlease provide user class in the form of LibraryName;Namespace.Classname or Namespace.Classname";
            throw new Exception(msg);
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
                // Callers may pass either a bare library name ("regex") or an explicit
                // filename ("Foo.dll"). Try the exact name first so filenames with
                // extensions resolve correctly; fall back to the "{name}.*" wildcard
                // for bare names.
                AddMatches(privatePath, userLibName, dllList);
                AddMatches(publicPath, userLibName, dllList);
            }

            if (dllList.Count == 0)
            {
                throw new Exception("Unable to find user dll under " + publicPath + " and " + privatePath);
            }

            return dllList;
        }

        /// <summary>
        /// Adds DLL matches for <paramref name="userLibName"/> under
        /// <paramref name="searchPath"/>. Tries the exact name first (so callers
        /// that pass "Foo.dll" resolve correctly) and falls back to the
        /// "{name}.*" wildcard for bare names (so callers that pass "Foo"
        /// still match "Foo.dll", "Foo.runtimeconfig.json", etc.).
        /// </summary>
        /// <param name="searchPath">
        /// Directory to search. Returns immediately if null/empty or absent;
        /// missing public/private library paths are not an error -- the other
        /// path may still yield matches.
        /// </param>
        /// <param name="userLibName">
        /// Library name to match. May be a bare stem ("regex") or include a
        /// .dll suffix ("Foo.dll"). Wildcards / path separators are not
        /// expected -- the caller is responsible for validation.
        /// </param>
        /// <param name="dllList">
        /// Output list to append discovered .dll paths to. Caller-owned;
        /// AddMatches never clears or replaces.
        /// </param>
        private static void AddMatches(string searchPath, string userLibName, List<string> dllList)
        {
            if (string.IsNullOrEmpty(searchPath) || !Directory.Exists(searchPath))
            {
                return;
            }

            string exactPath = Path.Combine(searchPath, userLibName);
            if (File.Exists(exactPath))
            {
                dllList.Add(exactPath);
                return;
            }

            // The "{name}.*" wildcard matches non-DLL siblings too
            // ("{name}.runtimeconfig.json", "{name}.deps.json", etc.). Only
            // .dll files are valid Assembly.LoadFrom targets, so filter the
            // wildcard match to avoid spamming the error log when the loader
            // tries to load a JSON file as an assembly.
            dllList.AddRange(Directory.GetFiles(searchPath, userLibName + ".*")
                .Where(f => f.EndsWith(".dll", StringComparison.OrdinalIgnoreCase)));
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
