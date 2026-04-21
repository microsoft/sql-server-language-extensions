//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpExtension.cs
//
// Purpose:
//  Implement the managed language extensions APIs
//
//*********************************************************************
using System;
using System.IO;
using System.IO.Compression;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using static Microsoft.SqlServer.CSharpExtension.Sql;

namespace Microsoft.SqlServer.CSharpExtension
{
    /// <summary>
    /// This class implements all language extensions APIs and returns results to native host.
    /// </summary>
    public static unsafe class CSharpExtension
    {
        /// <summary>
        /// The CSharpSession object tracking information of the current session.
        /// </summary>
        private static CSharpSession _currentSession;

        /// <summary>
        /// The absolute path to the installation directory of the extension.
        /// </summary>
        private static string _languagePath;

        /// <summary>
        /// The absolute path to the public external libraries directory for this external language.
        /// </summary>
        private static string _publicLibraryPath;

        /// <summary>
        /// The absolute path to the private external libraries directory for this external language.
        /// </summary>
        private static string _privateLibraryPath;

        /// <summary>
        /// PARAMETERS value provided during CREATE EXTERNAL LANGUAGE or ALTER EXTERNAL LANGUAGE.
        /// </summary>
        private static string _languageParams;

        /// <summary>
        /// This delegate declares the delegate type of Init.
        /// </summary>
        public delegate short InitDelegate(
            char  *languageParams,
            ulong languageParamsLen,
            char  *languagePath,
            ulong languagePathLen,
            char  *publicLibraryPath,
            ulong publicLibraryPathLen,
            char  *privateLibraryPath,
            ulong privateLibraryPathLen);

        /// <summary>
        /// This method implements Init API
        /// </summary>
        /// <param name="languageParams">
        /// Null-terminated string containing PARAMETERS value provided
        /// during CREATE EXTERNAL LANGUAGE or ALTER EXTERNAL LANGUAGE.
        /// </param>
        /// <param name="languageParamsLen">
        /// Length in bytes of languageParams (excluding the null termination character).
        /// </param>
        /// <param name="languagePath">
        /// Null-terminated UTF-8 string containing the absolute path
        /// to the installation directory of the extension.
        /// </param>
        /// <param name="languagePathLen">
        /// Length in bytes of languagePath (excluding the null termination character).
        /// </param>
        /// <param name="publicLibraryPath">
        /// Null-terminated UTF-8 string containing the absolute path
        /// to the public external libraries directory for this external language.
        /// </param>
        /// <param name="publicLibraryPathLen">
        /// Length in bytes of PublicLibraryPath (excluding the null termination character).
        /// </param>
        /// <param name="privateLibraryPath">
        /// Null-terminated UTF-8 string containing the absolute path to the
        /// private external libraries directory for this user and this external language.
        /// </param>
        /// <param name="privateLibraryPathLen">
        /// Length in bytes of PrivateLibraryPath (excluding the null termination character).
        /// </param>
        /// <returns>
        /// SQL_SUCCESS(0), SQL_ERROR(-1)
        /// </returns>
        public static short Init(
            char  *languageParams,
            ulong languageParamsLen,
            char  *languagePath,
            ulong languagePathLen,
            char  *publicLibraryPath,
            ulong publicLibraryPathLen,
            char  *privateLibraryPath,
            ulong privateLibraryPathLen)
        {
            Logging.Trace("CSharpExtension::Init");
            return ExceptionUtils.WrapError(() =>
            {
                _languageParams = (languageParams == null) ? "" : Interop.UTF8PtrToStr(languageParams, languageParamsLen);
                _languagePath = (languagePath == null) ? "" :Interop.UTF8PtrToStr(languagePath, languagePathLen);
                _publicLibraryPath = (publicLibraryPath == null) ? "" :Interop.UTF8PtrToStr(publicLibraryPath, publicLibraryPathLen);
                _privateLibraryPath = (privateLibraryPath== null) ? "" :Interop.UTF8PtrToStr(privateLibraryPath, privateLibraryPathLen);
            });
        }

        /// <summary>
        /// This delegate declares the delegate type of InitSession.
        /// </summary>
        public delegate short InitSessionDelegate(
            Guid   sessionId,
            ushort taskId,
            ushort numTasks,
            char   *script,
            ulong  scriptLength,
            ushort inputSchemaColumnsNumber,
            ushort parametersNumber,
            char   *inputDataName,
            ushort inputDataNameLength,
            char   *outputDataName,
            ushort outputDataNameLength);

        /// <summary>
        /// This method implements InitSession API
        /// </summary>
        /// <param name="sessionId">
        /// GUID uniquely identifying this script session.
        /// </param>
        /// <param name="taskId">
        /// An integer uniquely identifying this execution process.
        /// </param>
        /// <param name="numTasks">
        /// Number of tasks for this session.
        /// </param>
        /// <param name="script">
        /// Null-terminated UTF-8 string containing the @script in sp_execute_external_script.
        /// </param>
        /// <param name="scriptLength">
        /// Length in bytes of ScriptScript (excluding the null termination character).
        /// </param>
        /// <param name="inputSchemaColumnsNumber">
        /// Number of columns in the result set from @input_data_1 in sp_execute_external_script.
        /// </param>
        /// <param name="parametersNumber">
        /// Number of input parameters from @params in sp_execute_external_script.
        /// </param>
        /// <param name="inputDataName">
        /// Null-terminated UTF-8 string containing the @input_data_1_name in sp_execute_external_script.
        /// </param>
        /// <param name="inputDataNameLength">
        /// Length in bytes of InputDataName (excluding the null termination character).
        /// </param>
        /// <param name="outputDataName">
        /// Null-terminated UTF-8 string containing the @output_data_1_name in sp_execute_external_script.
        /// </param>
        /// <param name="outputDataNameLength">
        /// Length in bytes of OutputDataName (excluding the null termination character).
        /// </param>
        /// <returns>
        /// SQL_SUCCESS(0), SQL_ERROR(-1)
        /// </returns>
        public static short InitSession(
            Guid   sessionId,
            ushort taskId,
            ushort numTasks,
            char   *script,
            ulong  scriptLength,
            ushort inputSchemaColumnsNumber,
            ushort parametersNumber,
            char   *inputDataName,
            ushort inputDataNameLength,
            char   *outputDataName,
            ushort outputDataNameLength)
        {
            Logging.Trace("CSharpExtension::InitSession");
            return ExceptionUtils.WrapError(() =>
            {
                string scriptStr = (script != null) ? Interop.UTF8PtrToStr(script, scriptLength) : string.Empty;
                string inputDataNameStr = Interop.UTF8PtrToStr(inputDataName, inputDataNameLength);
                string outputDataNameStr = Interop.UTF8PtrToStr(outputDataName, outputDataNameLength);
                CSharpUserDll userDll = new CSharpUserDll(
                    publicPath: _publicLibraryPath,
                    privatePath: _privateLibraryPath,
                    userScriptFullName: scriptStr);

                _currentSession = new CSharpSession(
                    sessionId: sessionId,
                    taskId: taskId,
                    numTasks: numTasks,
                    script: scriptStr,
                    inputSchemaColumnsNumber: inputSchemaColumnsNumber,
                    parametersNumber: parametersNumber,
                    inputDataName: inputDataNameStr,
                    outputDataName: outputDataNameStr,
                    userDll: userDll);
            });
        }

        /// <summary>
        /// This delegate declares the delegate type of InitColumn.
        /// </summary>
        public delegate short InitColumnDelegate(
            Guid   sessionId,
            ushort taskId,
            ushort columnNumber,
            char   *columnName,
            short  columnNameLength,
            short  dataType,
            ulong  columnSize,
            short  decimalDigits,
            short  nullable,
            short  partitionByNumber,
            short  orderByNumber);

        /// <summary>
        /// This method initializes column-specific data. We store the name
        /// and the data type of the column here.
        /// </summary>
        /// <param name="sessionId">
        /// GUID uniquely identifying this script session.
        /// </param>
        /// <param name="taskId">
        /// An integer uniquely identifying this execution process.
        /// </param>
        /// <param name="columnNumber">
        /// An integer identifying the index of this column in the input schema.
        /// Columns are numbered sequentially in increasing order starting at 0.
        /// </param>
        /// <param name="columnName">
        /// Null-terminated UTF-8 string containing the column's name.
        /// </param>
        /// <param name="columnNameLength">
        /// Length in bytes of ColumnName (excluding the null termination character).
        /// </param>
        /// <param name="dataType">
        /// The ODBC C type identifying this column's data type.
        /// </param>
        /// <param name="columnSize">
        /// The maximum size in bytes of the underlying data in this column.
        /// </param>
        /// <param name="decimalDigits">
        /// The decimal digits of underlying data in this column, as defined by Decimal Digits.
        /// </param>
        /// <param name="nullable">
        /// A value that indicates whether this column may contain NULL values.
        /// </param>
        /// <param name="partitionByNumber">
        /// A value that indicates the index of this column in the
        /// @input_data_1_partition_by_columns sequence in sp_execute_external_script.
        /// Columns are numbered sequentially in increasing order starting at 0.
        /// If this column is not included in the sequence, the value is -1.
        /// </param>
        /// <param name="orderByNumber">
        /// A value that indicates the index of this column in the
        /// @input_data_1_order_by_columns sequence in sp_execute_external_script.
        /// Columns are numbered sequentially in increasing order starting at 0.
        /// If this column is not included in the sequence, the value is -1.
        /// <returns>
        /// SQL_SUCCESS(0), SQL_ERROR(-1)
        /// </returns>
        public static short InitColumn(
            Guid   sessionId,
            ushort taskId,
            ushort columnNumber,
            char   *columnName,
            short  columnNameLength,
            short  dataType,
            ulong  columnSize,
            short  decimalDigits,
            short  nullable,
            short  partitionByNumber,
            short  orderByNumber)
        {
            Logging.Trace("CSharpExtension::InitColumn");
            return ExceptionUtils.WrapError(() =>
            {
                _currentSession.InitInputColumn(
                    columnNumber: columnNumber,
                    columnName: Interop.UTF8PtrToStr(columnName, (ulong)columnNameLength),
                    dataType: ToManagedDataType(dataType),
                    decimalDigits: decimalDigits,
                    nullable: nullable,
                    columnSize: columnSize);
            });
        }

        /// <summary>
        /// This delegate declares the delegate type of InitParam.
        /// </summary>
        public delegate short InitParamDelegate(
            Guid   sessionId,
            ushort taskId,
            ushort paramNumber,
            char   *paramName,
            short  paramNameLength,
            short  dataType,
            ulong  paramSize,
            short  decimalDigits,
            void   *paramValue,
            int    strLenOrNullMap,
            short  inputOutputType);

        /// <summary>
        /// This method implements InitParam API
        /// </summary>
        /// <param name="sessionId">
        /// GUID uniquely identifying this script session.
        /// </param>
        /// <param name="taskId">
        /// An integer uniquely identifying this execution process.
        /// </param>
        /// <param name="paramNumber">
        /// An integer identifying the index of this parameter. Parameters
        /// are numbered sequentially in increasing order starting at 0.
        /// </param>
        /// <param name="paramName">
        /// Null-terminated UTF-8 string containing the parameter's name.
        /// </param>
        /// <param name="paramNameLength">
        /// Length in bytes of ParamName (excluding the null termination character).
        /// </param>
        /// <param name="dataType">
        /// The ODBC C type identifying this parameter's data type.
        /// </param>
        /// <param name="paramSize">
        /// The maximum size in bytes of the underlying data in this parameter.
        /// </param>
        /// <param name="decimalDigits">
        /// The decimal digits of underlying data in this parameter, as defined by Decimal Digits.
        /// </param>
        /// <param name="paramValue">
        /// A pointer to a buffer containing the parameter's value.
        /// </param>
        /// <param name="strLenOrNullMap">
        /// An integer value indicating the length in bytes of ParamValue,
        /// or SQL_NULL_DATA to indicate that the data is NULL.
        /// </param>
        /// <param name="inputOutputType">
        /// The type of the parameter.
        /// </param>
        /// <returns>
        /// SQL_SUCCESS(0), SQL_ERROR(-1)
        /// </returns>
        public static short InitParam(
            Guid   sessionId,
            ushort taskId,
            ushort paramNumber,
            char   *paramName,
            short  paramNameLength,
            short  dataType,
            ulong  paramSize,
            short  decimalDigits,
            void   *paramValue,
            int    strLenOrNullMap,
            short  inputOutputType)
        {
            Logging.Trace("CSharpExtension::InitParam");
            return ExceptionUtils.WrapError(() =>
            {
                _currentSession.InitParam(
                    paramNumber: paramNumber,
                    paramName: Interop.UTF8PtrToStr(paramName, (ulong)paramNameLength),
                    dataType: ToManagedDataType(dataType),
                    paramSize: paramSize,
                    decimalDigits: decimalDigits,
                    paramValue: paramValue,
                    strLenOrNullMap: strLenOrNullMap,
                    inputOutputType: inputOutputType);
            });
        }

        /// <summary>
        /// This delegate declares the delegate type of Execute.
        /// </summary>
        public delegate short ExecuteDelegate(
            Guid   sessionId,
            ushort taskId,
            ulong  rowsNumber,
            void   **data,
            int    **strLenOrNullMap,
            ushort *outputSchemaColumnsNumber);

        /// <summary>
        /// This method implements Execute API.
        /// Execute the @script in sp_execute_external_script.
        /// </summary>
        /// <param name="sessionId">
        /// GUID uniquely identifying this script session.
        /// </param>
        /// <param name="taskId">
        /// An integer uniquely identifying this execution process.
        /// </param>
        /// <param name="rowsNumber">
        /// The number of rows in the Data.
        /// </param>
        /// <param name="data">
        /// A two-dimensional array that contains the result set of @input_data_1
        /// in sp_execute_external_script.
        /// </param>
        /// <param name="strLenOrNullMap">
        /// An integer value indicating the length in bytes of ParamValue,
        /// or SQL_NULL_DATA to indicate that the data is NULL.
        /// </param>
        /// <param name="outputSchemaColumnsNumber">
        /// Pointer to a buffer in which to return the number of columns in the
        /// expected result set of the @script in sp_execute_external_script.
        /// </param>
        /// <returns>
        /// SQL_SUCCESS(0), SQL_ERROR(-1)
        /// </returns>
        public static short Execute(
            Guid   sessionId,
            ushort taskId,
            ulong  rowsNumber,
            void   **data,
            int    **strLenOrNullMap,
            ushort *outputSchemaColumnsNumber)
        {
            Logging.Trace("CSharpExtension::Execute");
            return ExceptionUtils.WrapError(() =>
            {
                _currentSession.Execute(
                    rowsNumber,
                    data,
                    strLenOrNullMap,
                    outputSchemaColumnsNumber);
            });
        }

        /// <summary>
        /// This delegate declares the delegate type of GetResultColumn.
        /// </summary>
        public delegate short GetResultColumnDelegate(
            Guid   sessionId,
            ushort taskId,
            ushort columnNumber,
            short  *dataType,
            ulong  *columnSize,
            short  *decimalDigits,
            short  *nullable);

        /// <summary>
        /// This method implements GetResultColumn API.
        /// Retrieve the information regarding a given output column for a particular session.
        /// </summary>
        /// <param name="sessionId">
        /// GUID uniquely identifying this script session.
        /// </param>
        /// <param name="taskId">
        /// An integer uniquely identifying this execution process.
        /// </param>
        /// <param name="columnNumber">
        /// An integer identifying the index of this column in the output schema.
        /// Columns are numbered sequentially in increasing order starting at 0.
        /// </param>
        /// <param name="dataType">
        /// A pointer to the buffer that contains the ODBC C type identifying this column's data type.
        /// </param>
        /// <param name="columnSize">
        /// A pointer to a buffer that contains the maximum size in bytes of the underlying data in this column.
        /// </param>
        /// <param name="decimalDigits">
        /// A pointer to a buffer that contains the decimal digits of underlying data in this column.
        /// If the number of decimal digits cannot be determined or is not applicable, the value is discarded.
        /// </param>
        /// <param name="nullable">
        /// A pointer to a buffer that contains a value, which indicates whether this column may contain NULL values.
        /// </param>
        /// <returns>
        /// SQL_SUCCESS(0), SQL_ERROR(-1)
        /// </returns>
        public static short GetResultColumn(
            Guid   sessionId,
            ushort taskId,
            ushort columnNumber,
            short  *dataType,
            ulong  *columnSize,
            short  *decimalDigits,
            short  *nullable)
        {
            Logging.Trace("CSharpExtension::GetResultColumn");
            return ExceptionUtils.WrapError(() =>
            {
                _currentSession.GetResultColumn(
                    columnNumber,
                    dataType,
                    columnSize,
                    decimalDigits,
                    nullable);
            });
        }

        /// <summary>
        /// This delegate declares the delegate type of GetResults.
        /// </summary>
        public delegate short GetResultsDelegate(
            Guid   sessionId,
            ushort TaskId,
            ulong  *rowsNumber,
            void   ***data,
            int    ***strLenOrNullMap);

        /// <summary>
        /// This method implements GetResults API.
        /// Retrieve the result set from executing the @script in sp_execute_external_script.
        /// </summary>
        /// <param name="sessionId">
        /// GUID uniquely identifying this script session.
        /// </param>
        /// <param name="taskId">
        /// An integer uniquely identifying this execution process.
        /// </param>
        /// <param name="rowsNumber">
        /// A pointer to a buffer that contains the number of rows in the Data.
        /// </param>
        /// <param name="data">
        /// A pointer to a two-dimensional array allocated by the extension that
        /// contains the result set of @script n sp_execute_external_script.
        /// </param>
        /// <param name="strLenOrNullMap">
        ///  A pointer to a two-dimensional array allocated by the extension that
        ///  contains the length/NULL indicator for each value in Data.
        /// <returns>
        /// SQL_SUCCESS(0), SQL_ERROR(-1)
        /// </returns>
        public static short GetResults(
            Guid   sessionId,
            ushort TaskId,
            ulong  *rowsNumber,
            void   ***data,
            int    ***strLenOrNullMap)
        {
            Logging.Trace("CSharpExtension::GetResults");
            return ExceptionUtils.WrapError(() =>
            {
                _currentSession.GetResults(
                    rowsNumber,
                    data,
                    strLenOrNullMap);
            });
        }

        /// <summary>
        /// This delegate declares the delegate type of GetOutputParam.
        /// </summary>
        public delegate short GetOutputParamDelegate(
            Guid   sessionId,
            ushort taskId,
            ushort paramNumber,
            void   **paramValue,
            int    *strLenOrNullMap);

        /// <summary>
        /// This method implements GetOutputParam API.
        /// Retrieve the information regarding a given output parameter for a particular session.
        /// </summary>
        /// <param name="sessionId">
        /// GUID uniquely identifying this script session.
        /// </param>
        /// <param name="taskId">
        /// An integer uniquely identifying this execution process.
        /// </param>
        /// <param name="paramNumber">
        /// An integer identifying the index of this parameter. Parameters
        /// are numbered sequentially in increasing order starting at 0.
        /// </param>
        /// <param name="paramValue">
        /// A pointer to a buffer containing the parameter's value.
        /// </param>
        /// <param name="strLenOrNullMap">
        /// A pointer to a buffer that contains an integer value indicating the length in bytes
        /// of ParamValue, or SQL_NULL_DATA to indicate that the data is NULL.
        /// </param>
        /// <returns>
        /// SQL_SUCCESS(0), SQL_ERROR(-1)
        /// </returns>
        public static short GetOutputParam(
            Guid   sessionId,
            ushort taskId,
            ushort paramNumber,
            void   **paramValue,
            int    *strLenOrNullMap)
        {
            Logging.Trace("CSharpExtension::GetOutputParam");
            return ExceptionUtils.WrapError(() =>
            {
                _currentSession.GetOutputParam(
                    paramNumber,
                    paramValue,
                    strLenOrNullMap);
            });
        }

        /// <summary>
        /// This delegate declares the delegate type of CleanupSession.
        /// </summary>
        public delegate short CleanupSessionDelegate(
            Guid   sessionId,
            ushort taskId);

        /// <summary>
        /// This method implements CleanupSession API.
        /// Clean up per-session information.
        /// </summary>
        /// <param name="sessionId">
        /// GUID uniquely identifying this script session.
        /// </param>
        /// <param name="taskId">
        /// An integer uniquely identifying this execution process.
        /// </param>
        /// <returns>
        /// SQL_SUCCESS(0), SQL_ERROR(-1)
        /// </returns>
        public static short CleanupSession(
            Guid   sessionId,
            ushort taskId)
        {
            Logging.Trace("CSharpExtension::CleanupSession");
            return ExceptionUtils.WrapError(() =>
            {
                if(_currentSession != null)
                {
                    _currentSession.CleanupSession();
                    _currentSession = null;
                }

                _currentSession = null;
            });
        }

        /// <summary>
        /// This delegate declares the delegate type of InstallExternalLibrary.
        /// </summary>
        public delegate short InstallExternalLibraryDelegate(
            Guid setupSessionId,
            char *libraryName,
            int  libraryNameLength,
            char *libraryFile,
            int  libraryFileLength,
            char *libraryInstallDirectory,
            int  libraryInstallDirectoryLength,
            char **libraryError,
            int  *libraryErrorLength);

        /// <summary>
        /// This method implements InstallExternalLibrary API.
        /// Installs an external library to the specified directory.
        /// The library file is expected to be a zip. If it contains an inner zip,
        /// that zip is extracted to the install directory. Otherwise, all files
        /// are copied directly.
        /// </summary>
        /// <returns>
        /// SQL_SUCCESS(0), SQL_ERROR(-1)
        /// </returns>
        public static short InstallExternalLibrary(
            Guid setupSessionId,
            char *libraryName,
            int  libraryNameLength,
            char *libraryFile,
            int  libraryFileLength,
            char *libraryInstallDirectory,
            int  libraryInstallDirectoryLength,
            char **libraryError,
            int  *libraryErrorLength)
        {
            Logging.Trace("CSharpExtension::InstallExternalLibrary");

            short result = SQL_SUCCESS;
            string tempFolder = null;

            try
            {
                string libFilePath = Interop.UTF8PtrToStr(libraryFile, (ulong)libraryFileLength);
                string installDir = Interop.UTF8PtrToStr(libraryInstallDirectory, (ulong)libraryInstallDirectoryLength);
                string libName = Interop.UTF8PtrToStr(libraryName, (ulong)libraryNameLength);

                ValidateLibraryName(libName);

                if (!Directory.Exists(installDir))
                {
                    Directory.CreateDirectory(installDir);
                }

                string manifestPath = Path.Combine(installDir, libName + ".manifest");
                HashSet<string> oldManifestEntries = ReadManifestEntries(manifestPath);

                if (!IsZipFile(libFilePath))
                {
                    // Raw DLL. Clean up any previous manifest-based install of the same
                    // library, then copy the file as "{libName}.dll" so DllUtils can find it.
                    if (File.Exists(manifestPath))
                    {
                        CleanupManifest(manifestPath, installDir);
                    }

                    File.Copy(libFilePath, Path.Combine(installDir, DllFileNameFor(libName)), true);
                    return SQL_SUCCESS;
                }

                // ZIP path. Stage the new content to a temp folder, validate it, then
                // clean up the old version and move the new files into place. This keeps
                // ALTER EXTERNAL LIBRARY transactional: a corrupt ZIP leaves the existing
                // install intact.
                tempFolder = Path.Combine(installDir, Guid.NewGuid().ToString());
                ZipFile.ExtractToDirectory(libFilePath, tempFolder);

                if (Directory.GetFiles(tempFolder).Length == 0 &&
                    Directory.GetDirectories(tempFolder).Length == 0)
                {
                    throw new InvalidOperationException(
                        "The library archive contains no entries.");
                }

                string innerZipPath = null;
                foreach (string file in Directory.GetFiles(tempFolder))
                {
                    if (Path.GetExtension(file).Equals(".zip", StringComparison.OrdinalIgnoreCase))
                    {
                        innerZipPath = file;
                        break;
                    }
                }

                // Collect relative paths we're about to install, validating each path
                // stays under installDir (defense-in-depth zip-slip check on top of
                // ZipFile.ExtractToDirectory's own validation).
                var extractedFiles = new List<string>();
                if (innerZipPath != null)
                {
                    using (var innerArchive = ZipFile.OpenRead(innerZipPath))
                    {
                        foreach (var entry in innerArchive.Entries)
                        {
                            if (string.IsNullOrEmpty(entry.Name))
                                continue;

                            extractedFiles.Add(ValidateRelativePath(installDir, entry.FullName));
                        }
                    }
                }
                else
                {
                    CollectRelativeFiles(tempFolder, "", extractedFiles);
                }

                CheckForConflicts(installDir, libName, extractedFiles, oldManifestEntries);

                // All checks passed. Remove the previous version's files (if any), then
                // extract/copy the new content into installDir.
                if (File.Exists(manifestPath))
                {
                    CleanupManifest(manifestPath, installDir);
                }

                if (innerZipPath != null)
                {
                    ZipFile.ExtractToDirectory(innerZipPath, installDir, false);
                }
                else
                {
                    foreach (string file in Directory.GetFiles(tempFolder))
                    {
                        File.Copy(file, Path.Combine(installDir, Path.GetFileName(file)), false);
                    }
                    foreach (string dir in Directory.GetDirectories(tempFolder))
                    {
                        CopyDirectory(dir, Path.Combine(installDir, Path.GetFileName(dir)));
                    }
                }

                // If no file in installDir matches "{libName}.*", copy the first .dll
                // found as "{libName}.dll" so DllUtils.CreateDllList can discover it.
                string aliasFileName = DllFileNameFor(libName);
                if (Directory.GetFiles(installDir, libName + ".*").Length == 0 &&
                    !File.Exists(Path.Combine(installDir, aliasFileName)))
                {
                    string[] dlls = Directory.GetFiles(installDir, "*.dll");
                    if (dlls.Length > 0)
                    {
                        string alias = Path.Combine(installDir, aliasFileName);
                        File.Copy(dlls[0], alias, false);
                        extractedFiles.Add(aliasFileName);
                    }
                }

                if (extractedFiles.Count > 0)
                {
                    File.WriteAllLines(manifestPath, extractedFiles);
                }
            }
            catch (Exception e)
            {
                string stackTracePart = string.IsNullOrEmpty(e.StackTrace) ? string.Empty : e.StackTrace + Environment.NewLine;
                Logging.Error(stackTracePart + "Error: " + e.Message);
                SetLibraryError(e.Message, libraryError, libraryErrorLength);
                result = SQL_ERROR;
            }
            finally
            {
                if (tempFolder != null && Directory.Exists(tempFolder))
                {
                    try { Directory.Delete(tempFolder, true); }
                    catch { /* best-effort */ }
                }
            }

            return result;
        }

        /// <summary>
        /// This delegate declares the delegate type of UninstallExternalLibrary.
        /// </summary>
        public delegate short UninstallExternalLibraryDelegate(
            Guid setupSessionId,
            char *libraryName,
            int  libraryNameLength,
            char *libraryInstallDirectory,
            int  libraryInstallDirectoryLength,
            char **libraryError,
            int  *libraryErrorLength);

        /// <summary>
        /// This method implements UninstallExternalLibrary API.
        /// Uninstalls an external library from the specified directory.
        /// </summary>
        /// <returns>
        /// SQL_SUCCESS(0), SQL_ERROR(-1)
        /// </returns>
        public static short UninstallExternalLibrary(
            Guid setupSessionId,
            char *libraryName,
            int  libraryNameLength,
            char *libraryInstallDirectory,
            int  libraryInstallDirectoryLength,
            char **libraryError,
            int  *libraryErrorLength)
        {
            Logging.Trace("CSharpExtension::UninstallExternalLibrary");

            short result = SQL_SUCCESS;

            try
            {
                string installDir = Interop.UTF8PtrToStr(libraryInstallDirectory, (ulong)libraryInstallDirectoryLength);
                string libName = Interop.UTF8PtrToStr(libraryName, (ulong)libraryNameLength);

                if (Directory.Exists(installDir))
                {
                    // Check for a manifest written during install that lists
                    // all files extracted from the library's ZIP content.
                    string manifestPath = Path.Combine(installDir, libName + ".manifest");
                    if (File.Exists(manifestPath))
                    {
                        CleanupManifest(manifestPath, installDir);
                    }

                    // Non-ZIP installs write a single "{libName}.dll" file and
                    // no manifest; remove that file directly.
                    string libraryFile = Path.Combine(installDir, DllFileNameFor(libName));
                    if (File.Exists(libraryFile))
                    {
                        File.Delete(libraryFile);
                    }
                }
            }
            catch (Exception e)
            {
                string stackTracePart = string.IsNullOrEmpty(e.StackTrace) ? string.Empty : e.StackTrace + Environment.NewLine;
                Logging.Error(stackTracePart + "Error: " + e.Message);
                SetLibraryError(e.Message, libraryError, libraryErrorLength);
                result = SQL_ERROR;
            }

            return result;
        }

        /// <summary>
        /// Allocates an unmanaged error string and populates the error output parameters.
        /// </summary>
        private static void SetLibraryError(string errorMessage, char **libraryError, int *libraryErrorLength)
        {
            if (libraryError != null && libraryErrorLength != null)
            {
                byte[] errorBytes = System.Text.Encoding.UTF8.GetBytes(errorMessage);
                IntPtr errorPtr = Marshal.AllocHGlobal(errorBytes.Length + 1);
                Marshal.Copy(errorBytes, 0, errorPtr, errorBytes.Length);
                ((byte*)errorPtr)[errorBytes.Length] = 0;
                *libraryError = (char*)errorPtr;
                *libraryErrorLength = errorBytes.Length;
            }
        }

        /// <summary>
        /// Recursively copies a directory and its contents.
        /// </summary>
        private static void CopyDirectory(string sourceDir, string destDir)
        {
            Directory.CreateDirectory(destDir);

            foreach (string file in Directory.GetFiles(sourceDir))
            {
                string destFile = Path.Combine(destDir, Path.GetFileName(file));
                File.Copy(file, destFile, true);
            }

            foreach (string dir in Directory.GetDirectories(sourceDir))
            {
                string destSubDir = Path.Combine(destDir, Path.GetFileName(dir));
                CopyDirectory(dir, destSubDir);
            }
        }

        /// <summary>
        /// Recursively collects all file paths relative to the root directory.
        /// </summary>
        private static void CollectRelativeFiles(string directory, string prefix, List<string> results)
        {
            foreach (string file in Directory.GetFiles(directory))
            {
                string relPath = string.IsNullOrEmpty(prefix)
                    ? Path.GetFileName(file)
                    : Path.Combine(prefix, Path.GetFileName(file));
                results.Add(relPath);
            }

            foreach (string dir in Directory.GetDirectories(directory))
            {
                string dirName = Path.GetFileName(dir);
                string newPrefix = string.IsNullOrEmpty(prefix)
                    ? dirName
                    : Path.Combine(prefix, dirName);
                CollectRelativeFiles(dir, newPrefix, results);
            }
        }

        // Case-sensitivity matches the host OS's filesystem semantics.
        private static readonly StringComparer s_pathComparer =
            OperatingSystem.IsWindows() ? StringComparer.OrdinalIgnoreCase : StringComparer.Ordinal;
        private static readonly StringComparison s_pathComparison =
            OperatingSystem.IsWindows() ? StringComparison.OrdinalIgnoreCase : StringComparison.Ordinal;

        // Returns the on-disk file name for a raw-DLL install of `libName`.
        // If the library was registered with a name that already ends in ".dll"
        // (e.g. CREATE EXTERNAL LIBRARY [Scriptoria.dll]), we must not append
        // a second ".dll" — that produced "Scriptoria.dll.dll" files that the
        // CLR assembly resolver could not locate.
        private static string DllFileNameFor(string libName)
        {
            if (!string.IsNullOrEmpty(libName) &&
                libName.EndsWith(".dll", s_pathComparison))
            {
                return libName;
            }
            return libName + ".dll";
        }

        // Rejects library names that could escape installDir when combined via Path.Combine.
        private static void ValidateLibraryName(string libName)
        {
            if (string.IsNullOrEmpty(libName))
            {
                throw new ArgumentException("Library name must not be empty.");
            }
            if (libName.IndexOfAny(new[] { '/', '\\', '\0' }) >= 0 ||
                libName.Contains("..") ||
                Path.IsPathRooted(libName))
            {
                throw new ArgumentException(
                    $"Library name '{libName}' contains invalid characters.");
            }
        }

        // A ZIP file of any variant begins with the 'PK' magic bytes (0x50 0x4B).
        private static bool IsZipFile(string path)
        {
            using (var fs = new FileStream(path, FileMode.Open, FileAccess.Read))
            {
                byte[] magic = new byte[2];
                return fs.Read(magic, 0, 2) == 2 && magic[0] == 0x50 && magic[1] == 0x4B;
            }
        }

        // Reads an existing manifest into a set of relative paths. Empty set if absent.
        private static HashSet<string> ReadManifestEntries(string manifestPath)
        {
            var set = new HashSet<string>(s_pathComparer);
            if (File.Exists(manifestPath))
            {
                foreach (string line in File.ReadAllLines(manifestPath))
                {
                    if (!string.IsNullOrWhiteSpace(line))
                    {
                        set.Add(line);
                    }
                }
            }
            return set;
        }

        // Converts a ZIP entry path to a platform-native relative path and verifies
        // it does not escape installDir (defense-in-depth zip-slip check).
        private static string ValidateRelativePath(string installDir, string zipEntryFullName)
        {
            string relPath = zipEntryFullName.Replace('/', Path.DirectorySeparatorChar);
            string fullInstall = Path.GetFullPath(installDir);
            string fullCombined = Path.GetFullPath(Path.Combine(fullInstall, relPath));
            string sep = Path.DirectorySeparatorChar.ToString();
            string prefix = fullInstall.EndsWith(sep) ? fullInstall : fullInstall + sep;

            if (!fullCombined.StartsWith(prefix, s_pathComparison))
            {
                throw new InvalidOperationException(
                    $"Library archive contains entry with invalid path: '{zipEntryFullName}'.");
            }
            return relPath;
        }

        // Throws if any staged relative path collides with an existing file that is not
        // owned by the previous install of this same library.
        private static void CheckForConflicts(
            string installDir,
            string libName,
            List<string> relPaths,
            HashSet<string> ownedByPrevious)
        {
            foreach (string relPath in relPaths)
            {
                if (ownedByPrevious.Contains(relPath))
                {
                    continue;
                }
                if (File.Exists(Path.Combine(installDir, relPath)))
                {
                    throw new InvalidOperationException(
                        $"Cannot install library '{libName}': file '{relPath}' already exists in the install directory.");
                }
            }
        }

        /// <summary>
        /// Reads a manifest, deletes each listed file, removes any directories
        /// that become empty (bottom-up), then deletes the manifest itself.
        /// </summary>
        private static void CleanupManifest(string manifestPath, string installDir)
        {
            string fullInstall = Path.GetFullPath(installDir);
            string sep = Path.DirectorySeparatorChar.ToString();
            string prefix = fullInstall.EndsWith(sep) ? fullInstall : fullInstall + sep;

            string[] entries = File.ReadAllLines(manifestPath);
            var parentDirs = new HashSet<string>(s_pathComparer);

            foreach (string relPath in entries)
            {
                if (string.IsNullOrWhiteSpace(relPath))
                {
                    continue;
                }

                string fullPath;
                try
                {
                    fullPath = Path.GetFullPath(Path.Combine(fullInstall, relPath));
                }
                catch
                {
                    continue;
                }

                // Defense in depth: skip any entry that resolves outside installDir.
                if (!fullPath.StartsWith(prefix, s_pathComparison))
                {
                    continue;
                }

                if (File.Exists(fullPath))
                {
                    File.Delete(fullPath);
                }

                string dir = Path.GetDirectoryName(fullPath);
                while (!string.IsNullOrEmpty(dir) &&
                       !dir.Equals(fullInstall, s_pathComparison))
                {
                    parentDirs.Add(dir);
                    dir = Path.GetDirectoryName(dir);
                }
            }

            // Remove empty directories deepest first.
            var sortedDirs = new List<string>(parentDirs);
            sortedDirs.Sort((a, b) => SeparatorCount(b).CompareTo(SeparatorCount(a)));
            foreach (string dir in sortedDirs)
            {
                if (Directory.Exists(dir) &&
                    Directory.GetFiles(dir).Length == 0 &&
                    Directory.GetDirectories(dir).Length == 0)
                {
                    Directory.Delete(dir, false);
                }
            }

            File.Delete(manifestPath);
        }

        private static int SeparatorCount(string path)
        {
            int count = 0;
            for (int i = 0; i < path.Length; i++)
            {
                if (path[i] == Path.DirectorySeparatorChar)
                {
                    count++;
                }
            }
            return count;
        }
    }
}
