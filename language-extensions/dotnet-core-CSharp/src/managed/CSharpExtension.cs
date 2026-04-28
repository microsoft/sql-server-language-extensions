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
using System.Threading;
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
        /// Case-sensitivity comparer matching the host OS's filesystem
        /// semantics. Used for set keys that contain on-disk file paths.
        /// </summary>
        private static readonly StringComparer s_pathComparer =
            OperatingSystem.IsWindows() ? StringComparer.OrdinalIgnoreCase : StringComparer.Ordinal;

        /// <summary>
        /// Case-sensitivity rule matching the host OS's filesystem semantics.
        /// Used for string-comparison APIs (StartsWith / EndsWith / Equals)
        /// that operate on on-disk file paths.
        /// </summary>
        private static readonly StringComparison s_pathComparison =
            OperatingSystem.IsWindows() ? StringComparison.OrdinalIgnoreCase : StringComparison.Ordinal;

        /// <summary>
        /// Sleep interval (in milliseconds) between attempts to acquire the
        /// per-installDir install lock when another process holds it. See
        /// <see cref="AcquireInstallLock"/>.
        /// </summary>
        private const int s_lockRetryDelayMs = 100;

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
        /// The library file MAY be a ZIP archive (with optional inner ZIP and
        /// arbitrary nested file tree) OR a raw DLL. Raw-DLL support matches
        /// the pre-PR ExtHost behavior: the file is copied as "{libName}.dll".
        /// ZIP support is the new capability added by this PR; it allows
        /// libraries to ship multiple DLLs, runtime config, or supporting
        /// files in a single archive.
        /// </summary>
        /// <remarks>
        /// NOTE: Unlike most other extension APIs, Install/Uninstall report
        /// errors via the libraryError out-parameter rather than through
        /// ExceptionUtils.WrapError. This matches the sqlexternallibrary.h
        /// contract: the host expects the error string to come back through
        /// libraryError / libraryErrorLength so it can surface it as part of
        /// CREATE/ALTER/DROP EXTERNAL LIBRARY diagnostics.
        /// </remarks>
        /// <param name="setupSessionId">
        /// Session GUID supplied by ExtHost. Currently unused by the CSharp
        /// extension; logged downstream for trace correlation only.
        /// </param>
        /// <param name="libraryName">
        /// UTF-8 buffer holding the library name from CREATE EXTERNAL LIBRARY.
        /// NOT null-terminated -- read exactly libraryNameLength bytes.
        /// </param>
        /// <param name="libraryNameLength">
        /// Byte length of <paramref name="libraryName"/>.
        /// </param>
        /// <param name="libraryFile">
        /// UTF-8 buffer holding the absolute path to the library content file
        /// (a raw DLL or a ZIP archive). NOT null-terminated -- read exactly
        /// libraryFileLength bytes.
        /// </param>
        /// <param name="libraryFileLength">
        /// Byte length of <paramref name="libraryFile"/>.
        /// </param>
        /// <param name="libraryInstallDirectory">
        /// UTF-8 buffer holding the absolute path to the install directory.
        /// May be the public or private external library path. Created if
        /// it doesn't exist. NOT null-terminated.
        /// </param>
        /// <param name="libraryInstallDirectoryLength">
        /// Byte length of <paramref name="libraryInstallDirectory"/>.
        /// </param>
        /// <param name="libraryError">
        /// On failure, set to a freshly-allocated UTF-8 error string for
        /// ExtHost to surface to the user. ExtHost takes ownership of the
        /// allocation. On success, set to nullptr.
        /// </param>
        /// <param name="libraryErrorLength">
        /// On failure, set to the byte length of <paramref name="libraryError"/>
        /// (excluding the null terminator -- ExtHost adds +1 when copying).
        /// On success, set to 0.
        /// </param>
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

                // Serialize against any other concurrent Install/Uninstall on
                // the same installDir. See AcquireInstallLock remarks for why:
                // without serialization, two installs can both pass
                // CheckForConflicts against pre-cleanup state, then one's
                // CleanupManifest destroys its previous version while the
                // other's File.Copy collides on overwrite:false -- silent
                // data loss for one of them.
                using (FileStream installLock = AcquireInstallLock(installDir))
                {
                    if (!Directory.Exists(installDir))
                    {
                        Directory.CreateDirectory(installDir);
                    }

                    string manifestPath = Path.Combine(installDir, libName + ".manifest");
                    HashSet<string> oldManifestEntries = ReadManifestEntries(manifestPath);

                    if (IsZipFile(libFilePath))
                    {
                        tempFolder = InstallZipPackage(libFilePath, installDir, libName,
                            manifestPath, oldManifestEntries);
                    }
                    else
                    {
                        InstallRawDll(libFilePath, installDir, libName, manifestPath);
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
        /// Installs a raw DLL (non-ZIP) library file. Copies the file as
        /// "{libName}.dll" into <paramref name="installDir"/> and writes a
        /// single-entry manifest so future ALTER calls can clean it up.
        /// </summary>
        /// <remarks>
        /// Matches the pre-PR ExtHost <c>CopyFileW(..., bFailIfExists=TRUE)</c>
        /// contract: if "{libName}.dll" already exists at the target path AND
        /// it is NOT owned by this library (i.e. not listed in our manifest),
        /// throws rather than silently overwriting a file that may belong to
        /// another library.
        /// </remarks>
        private static void InstallRawDll(
            string libFilePath,
            string installDir,
            string libName,
            string manifestPath)
        {
            string dllFileName = DllFileNameFor(libName);
            string installedDllPath = Path.Combine(installDir, dllFileName);

            if (File.Exists(manifestPath))
            {
                // Prior manifest-based install of the SAME library: clean it up
                // first so the upcoming copy has a free slot.
                CleanupManifest(manifestPath, installDir);
            }
            else if (File.Exists(installedDllPath))
            {
                // No manifest: we don't own this file. Fail rather than overwrite.
                throw new IOException(
                    $"Cannot install library '{libName}': file '{dllFileName}' " +
                    "already exists in the install directory and is not owned by this library.");
            }

            File.Copy(libFilePath, installedDllPath, false);

            // Track the raw-DLL install in a manifest too. This is what makes
            // ALTER from raw-DLL to ZIP work: the ZIP path's CheckForConflicts
            // sees "{libName}.dll" in oldManifestEntries and treats it as
            // owned-by-previous, allowing the upgrade to proceed cleanly.
            File.WriteAllLines(manifestPath, new[] { dllFileName });
            Logging.Trace(
                $"Wrote manifest: {manifestPath} with 1 entry (raw-DLL install)");
        }

        /// <summary>
        /// Installs a ZIP-archive library. Stages the new content to a temp
        /// folder, validates it (zip-slip, empty-archive, conflict checks),
        /// then cleans up the previous version and copies the new content
        /// into <paramref name="installDir"/>.
        /// </summary>
        /// <returns>
        /// The path of the temp folder used for staging, so the caller can
        /// clean it up in its outer <c>finally</c> regardless of whether the
        /// install succeeded or threw.
        /// </returns>
        /// <remarks>
        /// A corrupt ZIP leaves the existing install intact (validation runs
        /// against the staged copy in tempFolder, before we touch installDir).
        /// On-disk replacement is NOT atomic at the per-file level. A crash
        /// between CleanupManifest and the copy phase can leave the install
        /// directory inconsistent; SQL Server's catalog-based recovery
        /// re-installs from the catalog on the next session.
        /// </remarks>
        private static string InstallZipPackage(
            string libFilePath,
            string installDir,
            string libName,
            string manifestPath,
            HashSet<string> oldManifestEntries)
        {
            string tempFolder = Path.Combine(installDir, Guid.NewGuid().ToString());
            ZipFile.ExtractToDirectory(libFilePath, tempFolder);

            if (Directory.GetFiles(tempFolder).Length == 0 &&
                Directory.GetDirectories(tempFolder).Length == 0)
            {
                throw new InvalidOperationException(
                    "The library archive contains no entries.");
            }

            string innerZipPath = FindInnerZip(tempFolder);
            List<string> extractedFiles = CollectStagedFiles(installDir, tempFolder, innerZipPath);

            // Reject archives that contain only empty directories (no files).
            // The earlier "no entries" guard checks for an entirely-empty
            // tempFolder, but a ZIP whose entries are all directory markers
            // (e.g. "lib/", "lib/net8.0/") passes that check while leaving
            // CollectStagedFiles with zero file entries. Without this guard,
            // ALTER would silently destroy the previous version: CleanupManifest
            // deletes its content, nothing is copied (extractedFiles is empty),
            // and the manifest write is skipped. The library would end up GONE
            // with no replacement and no manifest tracking what was lost.
            if (extractedFiles.Count == 0)
            {
                throw new InvalidOperationException(
                    "The library archive contains no files.");
            }

            string aliasFileName = DllFileNameFor(libName);
            string aliasSourceRelPath = DetermineAliasSource(libName, aliasFileName, extractedFiles);
            if (aliasSourceRelPath != null)
            {
                // Append the alias to extractedFiles BEFORE CheckForConflicts
                // so any "{libName}.dll" collision with another library fails
                // fast with no content written to installDir.
                extractedFiles.Add(aliasFileName);
            }

            CheckForConflicts(installDir, libName, extractedFiles, oldManifestEntries);

            // All checks passed. Remove the previous version's files (if any),
            // then extract / copy the new content into installDir.
            if (File.Exists(manifestPath))
            {
                CleanupManifest(manifestPath, installDir);
            }

            ExtractContentToInstallDir(installDir, tempFolder, innerZipPath);

            if (aliasSourceRelPath != null)
            {
                CreateAlias(installDir, aliasSourceRelPath, aliasFileName);
            }

            File.WriteAllLines(manifestPath, extractedFiles);
            Logging.Trace(
                $"Wrote manifest: {manifestPath} with {extractedFiles.Count} entries");

            return tempFolder;
        }

        /// <summary>
        /// Finds the FIRST top-level ".zip" entry inside the staged
        /// <paramref name="tempFolder"/>, or null if none is present.
        /// </summary>
        /// <remarks>
        /// If the outer ZIP contains exactly one inner .zip at its top level,
        /// it is treated as the real package and extracted in place of the
        /// outer. This matches the way the SQL Server engine wraps
        /// user-supplied archives. Multiple inner zips are unsupported; any
        /// after the first are extracted as opaque files (callers that need
        /// to ship multiple zips should pack them inside subdirectories).
        /// </remarks>
        private static string FindInnerZip(string tempFolder)
        {
            foreach (string file in Directory.GetFiles(tempFolder))
            {
                if (Path.GetExtension(file).Equals(".zip", StringComparison.OrdinalIgnoreCase))
                {
                    return file;
                }
            }
            return null;
        }

        /// <summary>
        /// Builds the list of relative paths that will be installed into
        /// <paramref name="installDir"/>, validating each path stays under
        /// installDir (defense-in-depth zip-slip check on top of
        /// ZipFile.ExtractToDirectory's own validation).
        /// </summary>
        private static List<string> CollectStagedFiles(
            string installDir,
            string tempFolder,
            string innerZipPath)
        {
            List<string> extractedFiles = new List<string>();
            if (innerZipPath != null)
            {
                using (ZipArchive innerArchive = ZipFile.OpenRead(innerZipPath))
                {
                    foreach (ZipArchiveEntry entry in innerArchive.Entries)
                    {
                        if (string.IsNullOrEmpty(entry.Name))
                        {
                            // Directory entry (no filename portion). Skip.
                            continue;
                        }
                        extractedFiles.Add(ValidateRelativePath(installDir, entry.FullName));
                    }
                }
            }
            else
            {
                CollectRelativeFiles(tempFolder, "", extractedFiles);
            }
            return extractedFiles;
        }

        /// <summary>
        /// Decides which extracted file (if any) should be cloned as the
        /// "{libName}.dll" alias so DllUtils.CreateDllList can discover the
        /// library. Returns the relative path of the source DLL to clone, or
        /// null if no alias is needed (a root-level "{libName}.*" is already
        /// present, or no DLL exists to clone from).
        /// </summary>
        /// <remarks>
        /// DllUtils.CreateDllList searches only the TOP LEVEL of the library
        /// path (non-recursive). So a deeply-nested DLL like
        /// "lib/net8.0/{libName}.dll" does NOT make the library discoverable,
        /// and we still need to create a root-level alias. Only a root-level
        /// "{libName}.*" suppresses alias creation.
        /// </remarks>
        private static string DetermineAliasSource(
            string libName,
            string aliasFileName,
            List<string> extractedFiles)
        {
            foreach (string relPath in extractedFiles)
            {
                bool isRootLevel =
                    relPath.IndexOf('/') < 0 &&
                    relPath.IndexOf('\\') < 0;
                if (!isRootLevel)
                {
                    continue;
                }
                string name = Path.GetFileName(relPath);
                if (name.StartsWith(libName + ".", s_pathComparison) ||
                    name.Equals(aliasFileName, s_pathComparison))
                {
                    // A root-level file matches "{libName}.*" -- already discoverable.
                    return null;
                }
            }
            foreach (string relPath in extractedFiles)
            {
                if (Path.GetExtension(relPath).Equals(".dll", s_pathComparison))
                {
                    return relPath;
                }
            }
            return null;
        }

        /// <summary>
        /// Extracts the staged content from <paramref name="tempFolder"/> (or
        /// directly from the inner zip if <paramref name="innerZipPath"/> is
        /// non-null) into the live <paramref name="installDir"/>.
        /// </summary>
        private static void ExtractContentToInstallDir(
            string installDir,
            string tempFolder,
            string innerZipPath)
        {
            if (innerZipPath != null)
            {
                // SAFETY: Extracting directly to the live installDir is currently
                // safe because ZipFile.ExtractToDirectory does not restore symlink
                // entries as actual symlinks on any platform -- they are written
                // as regular files containing the link target text. So even on
                // Linux, an attacker-controlled inner ZIP cannot smuggle a symlink
                // into installDir via this path today.
                //
                // If a future .NET version changes this (e.g. honors symlink
                // entries on Linux), or if we switch to a different extraction
                // library, this path MUST be re-routed through a separate temp
                // folder followed by an IsReparsePoint-guarded copy -- mirror
                // the non-inner-zip branch below for the pattern.
                ZipFile.ExtractToDirectory(innerZipPath, installDir, false);
            }
            else
            {
                // Skip reparse points at the root of tempFolder for the same
                // reason CopyDirectory does inside subdirectories: a root-level
                // symlink could cause File.Copy to copy data from outside the
                // staged tree, and a root-level reparse-point directory could
                // make CopyDirectory recurse out of tempFolder. CopyDirectory
                // only checks the entries it iterates, not its top-level
                // sourceDir argument, so the guards live here at the call site.
                foreach (string file in Directory.GetFiles(tempFolder))
                {
                    if (IsReparsePoint(file))
                    {
                        continue;
                    }
                    File.Copy(file, Path.Combine(installDir, Path.GetFileName(file)), false);
                }
                foreach (string dir in Directory.GetDirectories(tempFolder))
                {
                    if (IsReparsePoint(dir))
                    {
                        continue;
                    }
                    CopyDirectory(dir, Path.Combine(installDir, Path.GetFileName(dir)));
                }
            }
        }

        /// <summary>
        /// Creates the "{libName}.dll" alias by cloning the source DLL chosen
        /// by <see cref="DetermineAliasSource"/>. Caller must have verified
        /// the alias is needed and conflict-checked before this is invoked.
        /// </summary>
        private static void CreateAlias(
            string installDir,
            string aliasSourceRelPath,
            string aliasFileName)
        {
            string aliasSrc = Path.Combine(installDir, aliasSourceRelPath);
            string alias = Path.Combine(installDir, aliasFileName);
            if (File.Exists(aliasSrc))
            {
                File.Copy(aliasSrc, alias, false);
            }
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
        /// Uninstalls an external library from the specified directory by
        /// reading "{libName}.manifest" and deleting each listed file, then
        /// pruning newly-empty subdirectories. Files belonging to other
        /// libraries (not listed in this manifest) are left intact.
        /// </summary>
        /// <remarks>
        /// See InstallExternalLibrary remarks: errors are reported via the
        /// libraryError out-parameter rather than ExceptionUtils.WrapError,
        /// per the sqlexternallibrary.h contract. A missing installDir or
        /// missing manifest is treated as a no-op success (the library is
        /// already in the desired state).
        /// </remarks>
        /// <param name="setupSessionId">
        /// Session GUID supplied by ExtHost. Currently unused.
        /// </param>
        /// <param name="libraryName">
        /// UTF-8 buffer holding the library name from DROP EXTERNAL LIBRARY.
        /// NOT null-terminated -- read exactly libraryNameLength bytes.
        /// </param>
        /// <param name="libraryNameLength">
        /// Byte length of <paramref name="libraryName"/>.
        /// </param>
        /// <param name="libraryInstallDirectory">
        /// UTF-8 buffer holding the absolute path to the install directory.
        /// May be the public or private external library path. NOT
        /// null-terminated.
        /// </param>
        /// <param name="libraryInstallDirectoryLength">
        /// Byte length of <paramref name="libraryInstallDirectory"/>.
        /// </param>
        /// <param name="libraryError">
        /// On failure, set to a freshly-allocated UTF-8 error string for
        /// ExtHost to surface to the user. ExtHost takes ownership of the
        /// allocation. On success, set to nullptr.
        /// </param>
        /// <param name="libraryErrorLength">
        /// On failure, set to the byte length of <paramref name="libraryError"/>
        /// (excluding the null terminator). On success, set to 0.
        /// </param>
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

                // Reject names containing path separators etc. before they are used
                // to build manifestPath / libraryFile via Path.Combine. Without this,
                // a malicious libName could resolve outside installDir.
                ValidateLibraryName(libName);

                if (Directory.Exists(installDir))
                {
                    // Serialize against any other concurrent Install/Uninstall
                    // on the same installDir. An uninstall that races an install
                    // of a different library can otherwise see CleanupManifest
                    // delete its own files between the install's CheckForConflicts
                    // and File.Copy, leaving the install with a stale view of disk
                    // state. See AcquireInstallLock remarks for the full threat
                    // model.
                    using (FileStream installLock = AcquireInstallLock(installDir))
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
                // Length excludes the null terminator -- ExtHost adds +1 when
                // copying via Utf8ToNullTerminatedUtf16Le / strcpy_s.
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
                if (IsReparsePoint(file))
                {
                    continue;
                }
                string destFile = Path.Combine(destDir, Path.GetFileName(file));
                // overwrite: false so that if filesystem state changed between the
                // conflict check and here (TOCTOU), we fail loud rather than silently
                // replacing a file belonging to another library.
                File.Copy(file, destFile, false);
            }

            foreach (string dir in Directory.GetDirectories(sourceDir))
            {
                if (IsReparsePoint(dir))
                {
                    continue;
                }
                string destSubDir = Path.Combine(destDir, Path.GetFileName(dir));
                CopyDirectory(dir, destSubDir);
            }
        }

        /// <summary>
        /// Returns true if <paramref name="path"/> is a reparse point
        /// (symbolic link, junction, mount point).
        /// </summary>
        /// <remarks>
        /// We treat all reparse points discovered in the staged tempFolder as
        /// untrusted. On Linux a ZIP archive can carry symlink entries that
        /// survive extraction; following them while copying into installDir
        /// could (a) copy data from outside the staged tree (information leak)
        /// or (b) cause the recursive copy to escape the staged area entirely
        /// and write into arbitrary filesystem locations.
        /// </remarks>
        private static bool IsReparsePoint(string path)
        {
            return (File.GetAttributes(path) & FileAttributes.ReparsePoint) != 0;
        }

        // Block forever in AcquireInstallLock -- match the engine's
        // "this install will eventually finish" contract. SQL Server's outer
        // statement-level cancellation is the right place to interrupt a
        // pathologically-stuck install, not an arbitrary timeout in here.
        // (s_lockRetryDelayMs is declared at the top of the class with the
        // other class members.)

        /// <summary>
        /// Acquires an exclusive cross-process lock for operations on
        /// <paramref name="installDir"/>. Returns a disposable handle that
        /// releases the lock when disposed.
        /// </summary>
        /// <remarks>
        /// Two concurrent CSharp extension processes (e.g. two SQL sessions
        /// each running CREATE/ALTER/DROP EXTERNAL LIBRARY against the same
        /// install directory) must not be allowed to interleave the
        /// CheckForConflicts / CleanupManifest / copy / WriteAllLines
        /// sequence. Without serialization, both can pass CheckForConflicts
        /// against pre-cleanup state, then one's CleanupManifest destroys
        /// its previous version's content while the other's File.Copy
        /// collides on overwrite:false -- leaving the second library GONE
        /// with no replacement and no manifest.
        ///
        /// Implementation: open "{installDir}/.install.lock" with
        /// FileShare.None. The OS releases the handle on process crash so
        /// there is no stale-lock risk. FileOptions.DeleteOnClose removes
        /// the lock file when the holder closes its handle, keeping the
        /// install directory clean across runs.
        ///
        /// Acquisition blocks indefinitely with a 100ms retry interval.
        /// In the uncontended common case (single session), acquisition
        /// completes on the first attempt with no measurable overhead.
        /// </remarks>
        private static FileStream AcquireInstallLock(string installDir)
        {
            Directory.CreateDirectory(installDir);
            string lockPath = Path.Combine(installDir, ".install.lock");

            while (true)
            {
                try
                {
                    return new FileStream(
                        lockPath,
                        FileMode.OpenOrCreate,
                        FileAccess.ReadWrite,
                        FileShare.None,
                        bufferSize: 1,
                        FileOptions.DeleteOnClose);
                }
                catch (IOException)
                {
                    // Another process holds the lock. Wait and retry.
                    Thread.Sleep(s_lockRetryDelayMs);
                }
            }
        }

        /// <summary>
        /// Recursively collects all file paths relative to the root directory.
        /// </summary>
        /// <remarks>
        /// Reparse points (symlinks, junctions) are skipped at both the file
        /// and directory level. The result of this walk feeds CheckForConflicts
        /// and the on-disk manifest, both of which assume every entry will be
        /// physically copied by CopyDirectory. Since CopyDirectory skips
        /// reparse points, recording them here would create phantom manifest
        /// entries that point at nothing on disk.
        /// </remarks>
        private static void CollectRelativeFiles(string directory, string prefix, List<string> results)
        {
            foreach (string file in Directory.GetFiles(directory))
            {
                if (IsReparsePoint(file))
                {
                    continue;
                }
                string relPath = string.IsNullOrEmpty(prefix)
                    ? Path.GetFileName(file)
                    : Path.Combine(prefix, Path.GetFileName(file));
                results.Add(relPath);
            }

            foreach (string dir in Directory.GetDirectories(directory))
            {
                if (IsReparsePoint(dir))
                {
                    continue;
                }
                string dirName = Path.GetFileName(dir);
                string newPrefix = string.IsNullOrEmpty(prefix)
                    ? dirName
                    : Path.Combine(prefix, dirName);
                CollectRelativeFiles(dir, newPrefix, results);
            }
        }

        /// <summary>
        /// Returns the on-disk file name for a raw-DLL install of
        /// <paramref name="libName"/>.
        /// </summary>
        /// <remarks>
        /// If the library was registered with a name that already ends in
        /// ".dll" (e.g. CREATE EXTERNAL LIBRARY [Scriptoria.dll]), we must
        /// not append a second ".dll" -- that produced "Scriptoria.dll.dll"
        /// files that the CLR assembly resolver could not locate.
        /// </remarks>
        /// <param name="libName">
        /// The library name as supplied via libraryName to InstallExternalLibrary.
        /// </param>
        /// <returns>
        /// "{libName}.dll" if libName does not already end in ".dll",
        /// otherwise libName unchanged.
        /// </returns>
        private static string DllFileNameFor(string libName)
        {
            string result;
            if (!string.IsNullOrEmpty(libName) &&
                libName.EndsWith(".dll", s_pathComparison))
            {
                result = libName;
            }
            else
            {
                result = libName + ".dll";
            }

            return result;
        }

        /// <summary>
        /// Validates that <paramref name="libName"/> is safe to use in path
        /// composition. Throws <see cref="ArgumentException"/> on rejection.
        /// </summary>
        /// <remarks>
        /// Without this check, a malicious or legacy libName like "../foo"
        /// or "/etc/foo" could make
        /// <c>Path.Combine(installDir, libName + ".manifest")</c> resolve
        /// outside installDir, allowing unintended file reads / writes /
        /// deletes. Also rejects names that are only an extension
        /// (e.g. ".dll") because the resulting "{libName}.manifest" paths
        /// would be hidden dotfiles on Linux and opaque on both platforms.
        /// </remarks>
        /// <param name="libName">
        /// The library name as supplied via libraryName to
        /// InstallExternalLibrary or UninstallExternalLibrary.
        /// </param>
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

            // Reject names that are only an extension (e.g. ".dll", ".txt").
            // Path.GetFileNameWithoutExtension returns "" for these, meaning
            // the name has no stem -- DllFileNameFor would return the bare
            // extension and the resulting "{libName}.manifest" / "{libName}.dll"
            // paths would be hidden dotfiles on Linux and opaque on both
            // platforms.
            if (string.IsNullOrEmpty(Path.GetFileNameWithoutExtension(libName)))
            {
                throw new ArgumentException(
                    $"Library name '{libName}' must not be only an extension.");
            }
        }

        /// <summary>
        /// Returns true if the file at <paramref name="path"/> appears to be
        /// a ZIP archive based on its leading two "magic bytes".
        /// </summary>
        /// <remarks>
        /// All ZIP variants (.zip, .jar, .war, .docx, etc.) begin with the
        /// two-byte signature "PK" (0x50 0x4B), inherited from PKWARE's
        /// original 1989 PKZIP format. We sniff content rather than trust
        /// the file extension because:
        /// <list type="bullet">
        ///   <item>
        ///   Callers can register a library file as e.g. "foo.zip" without
        ///   the bytes actually being a ZIP archive (the engine doesn't
        ///   require the registered name to match content).
        ///   </item>
        ///   <item>
        ///   Callers can register a library file with no extension or with a
        ///   non-".zip" extension (e.g. ".dll", ".bin") whose contents ARE a
        ///   ZIP archive.
        ///   </item>
        /// </list>
        /// The two-byte sniff is intentionally minimal: a file beginning with
        /// "PK" but corrupt internally will still reach
        /// ZipFile.ExtractToDirectory, which throws a clear exception that
        /// the outer catch surfaces via libraryError. The sniff's job is
        /// only to dispatch between the raw-DLL and ZIP install paths --
        /// not to validate ZIP integrity.
        /// </remarks>
        private static bool IsZipFile(string path)
        {
            // FileShare.Read so we don't fight with anti-virus scanners,
            // backup agents, or another concurrent SQL session that may have
            // the source file open for read.
            using (FileStream fs = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read))
            {
                byte[] magic = new byte[2];
                return fs.Read(magic, 0, 2) == 2 && magic[0] == 0x50 && magic[1] == 0x4B;
            }
        }

        /// <summary>
        /// Reads an existing manifest file into a set of relative paths.
        /// Returns an empty set if the manifest does not exist.
        /// </summary>
        /// <param name="manifestPath">
        /// Absolute path to the "{libName}.manifest" file. Safe to pass a
        /// non-existent path.
        /// </param>
        private static HashSet<string> ReadManifestEntries(string manifestPath)
        {
            HashSet<string> set = new HashSet<string>(s_pathComparer);
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

        /// <summary>
        /// Converts a ZIP entry path to a platform-native relative path and
        /// verifies it does not escape <paramref name="installDir"/>
        /// (defense-in-depth zip-slip check).
        /// </summary>
        /// <remarks>
        /// Two transformations + one check:
        /// <list type="number">
        /// <item>
        /// <b>Separator normalization.</b> ZIP entries always use forward
        /// slashes per the ZIP spec (PKWARE APPNOTE 4.4.17.1). We rewrite
        /// '/' to <see cref="Path.DirectorySeparatorChar"/> so subsequent
        /// <see cref="Path.Combine"/> / <see cref="File.Copy"/> calls
        /// produce native paths on Windows ('\\') and Linux ('/'). We do
        /// NOT touch backslashes -- they are illegal in ZIP entry names per
        /// spec, and on Linux a backslash is a legitimate filename
        /// character that must be preserved as-is.
        /// </item>
        /// <item>
        /// <b>Canonicalization via <see cref="Path.GetFullPath"/>.</b>
        /// Resolves any "..", ".", or symlink-style segments in both
        /// installDir and the combined path so the StartsWith check below
        /// is performed on absolute, canonical paths.
        /// </item>
        /// <item>
        /// <b>Containment check.</b> The combined path must start with
        /// <c>{fullInstall}{DirectorySeparatorChar}</c>. Comparing with the
        /// trailing separator is critical: without it, an installDir of
        /// "C:\install" would falsely accept entries that resolve to
        /// "C:\installEvil\..." (sibling directory with shared prefix).
        /// </item>
        /// </list>
        /// .NET's ZipFile.ExtractToDirectory already performs its own
        /// zip-slip check on extraction (since .NET Core 2.1). This
        /// function is defense in depth: the manifest we write must list
        /// only paths inside installDir, so that uninstall's
        /// File.Delete(...) calls cannot be tricked into deleting
        /// arbitrary files via a malicious manifest entry that survived
        /// extraction.
        /// </remarks>
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
            HashSet<string> parentDirs = new HashSet<string>(s_pathComparer);

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
                catch (Exception ex)
                {
                    // A malformed manifest entry shouldn't abort the rest of
                    // the cleanup, but it must leave a diagnostic trail so
                    // orphaned files don't disappear silently. Use Error
                    // (not Trace) -- the comment promises a trail and Trace
                    // is typically off in production deployments.
                    Logging.Error(
                        $"CleanupManifest: skipping manifest entry '{relPath}': {ex.Message}");
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
            List<string> sortedDirs = new List<string>(parentDirs);
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
