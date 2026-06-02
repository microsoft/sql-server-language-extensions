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
        /// Windows reserved DOS device names. Files and directories with
        /// these stems behave specially even on modern NTFS (CreateFile maps
        /// "CON" / "C:\path\CON.txt" to the console device, etc.), so we
        /// reject them as library names regardless of host OS to keep
        /// behavior consistent.
        /// </summary>
        private static readonly HashSet<string> s_reservedDeviceNames =
            new HashSet<string>(StringComparer.OrdinalIgnoreCase)
            {
                "CON", "PRN", "AUX", "NUL",
                "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
                "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9",
            };

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
        /// Delegate type matching the host's LogXEvent callback signature.
        /// </summary>
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void LogXEventCallbackDelegate(
            byte   *extensionName,
            ulong  extensionNameLength,
            Guid   sessionId,
            ushort taskId,
            ushort traceLevel,
            int    errorCode,
            byte   *message,
            ulong  messageLength);

        /// <summary>
        /// Managed representation of the SQLEXTENSION_HOST_CALLBACKS structure.
        /// Must match the native struct layout exactly.
        /// </summary>
        [StructLayout(LayoutKind.Sequential)]
        public struct SqlExtensionHostCallbacks
        {
            // Highest SQLEXTENSION_HOST_CALLBACKS_VERSION_* the host populates.
            // Extension must validate before reading version-gated fields.
            //
            public ushort Version;

            // Explicit padding so SizeInBytes is naturally 4-byte aligned regardless
            // of compiler packing settings. Must be zero.
            //
            public ushort Reserved0;

            // sizeof(SQLEXTENSION_HOST_CALLBACKS) as the host saw it at build time.
            // Extension must validate this is greater or equal the size of every field it intends
            // to read. Lets a newer Extension safely run against an older host that supplied a smaller struct.
            //
            public uint SizeInBytes;

            // Version 1 callbacks.
            //
            public IntPtr LogXEvent;

            // Reserved for future expansion. Zero-initialized by the host. Extension must not read or call these.
            //
            public IntPtr Reserved1;
            public IntPtr Reserved2;
        }

        /// <summary>
        /// Minimal SQLEXTENSION_HOST_CALLBACKS version this extension understands.
        /// If host callbacks version is greater than this, extension will still parse and read
        /// known host callbacks and ignore unknown fields, allowing forward compatibility.
        /// </summary>
        /// 
        private const ushort MinSupportedHostCallbacksVersion = 1;

        /// <summary>
        /// This delegate declares the delegate type of SetHostCallbacks.
        /// </summary>
        public delegate short SetHostCallbacksDelegate(
            SqlExtensionHostCallbacks *hostCallbacks);

        /// <summary>
        /// This method implements SetHostCallbacks API.
        /// Receives a pointer to the host callbacks structure, reads the callback
        /// function pointers during this call, and stores any needed managed
        /// delegates so managed code can call back into the host.
        /// </summary>
        /// <param name="hostCallbacks">
        /// Pointer to the SQLEXTENSION_HOST_CALLBACKS structure provided by the host.
        /// </param>
        /// <returns>
        /// SQL_SUCCESS(0), SQL_ERROR(-1)
        /// </returns>
        public static short SetHostCallbacks(
            SqlExtensionHostCallbacks *hostCallbacks)
        {
            Logging.Trace("CSharpExtension::SetHostCallbacks");
            return ExceptionUtils.WrapError(() =>
            {
                if (hostCallbacks == null)
                {
                    throw new ArgumentNullException(nameof(hostCallbacks));
                }

                // Validate the struct version before reading any version-gated fields.
                //
                if (hostCallbacks->Version < MinSupportedHostCallbacksVersion)
                {
                    Logging.Error(
                        "CSharpExtension::SetHostCallbacks: unsupported host callbacks version: " +
                        hostCallbacks->Version);

                    throw new NotSupportedException(
                        "Unsupported SQLEXTENSION_HOST_CALLBACKS version: " +
                        hostCallbacks->Version);
                }

                if (hostCallbacks->LogXEvent != IntPtr.Zero)
                {
                    var logXEvent = Marshal.GetDelegateForFunctionPointer<LogXEventCallbackDelegate>(
                        hostCallbacks->LogXEvent);
                    Logging.SetLogXEventCallback(logXEvent);

                    Logging.LogXEvent(
                        extensionName: null,
                        Guid.Empty,
                        taskId: 0,
                        traceLevel: Logging.TraceLevel.Information,
                        errorCode: 0,
                        "CSharp extension loaded, host callbacks registered (version " + hostCallbacks->Version + ")");
                }
                else
                {
                    // Host opted out of XEvent logging.
                    // Clear any previously stored delegate.
                    //
                    Logging.SetLogXEventCallback(null);
                }
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

                    // Dispatch on the user-registered library name's
                    // extension first. SQL Server hands the extension a
                    // staged temp file with a generated name (typically no
                    // .zip / .dll extension), so libFilePath is not a
                    // reliable signal of user intent. The only reliable
                    // signal is libName, which the engine forwards verbatim
                    // from CREATE EXTERNAL LIBRARY [<name>] -- e.g. a user
                    // who wrote CREATE EXTERNAL LIBRARY [Foo.dll] gets
                    // libName = "Foo.dll" and expects a raw-DLL install.
                    //
                    // When libName carries no extension (some test fixtures
                    // register libraries by bare name, pointing libFilePath
                    // at "foo-DLL.zip" or "foo-RAWDLL.dll"), fall back to
                    // libFilePath's extension so legacy callers continue to
                    // work without modification.
                    if (DispatchAsZip(libName, libFilePath))
                    {
                        InstallZipPackage(libFilePath, installDir, libName,
                            manifestPath, oldManifestEntries, out tempFolder);
                    }
                    else
                    {
                        InstallRawDll(libFilePath, installDir, libName, manifestPath);
                    }
                }
            }
            catch (Exception e)
            {
                Logging.Error(e.ToString());
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
        /// <param name="tempFolder">
        /// Out-parameter set to the staging tempFolder path AS SOON AS it is
        /// chosen, BEFORE any extraction is attempted. This lets the caller
        /// clean it up in its outer <c>finally</c> even if the extraction or
        /// any subsequent step throws -- otherwise a half-extracted tempFolder
        /// would leak inside installDir.
        /// </param>
        /// <remarks>
        /// A corrupt ZIP leaves the existing install intact (validation runs
        /// against the staged copy in tempFolder, before we touch installDir).
        /// On-disk replacement is NOT atomic at the per-file level. A crash
        /// between CleanupManifest and the copy phase can leave the install
        /// directory inconsistent; SQL Server's catalog-based recovery
        /// re-installs from the catalog on the next session.
        /// </remarks>
        private static void InstallZipPackage(
            string libFilePath,
            string installDir,
            string libName,
            string manifestPath,
            HashSet<string> oldManifestEntries,
            out string tempFolder)
        {
            // Publish tempFolder to the caller BEFORE doing any work that
            // could throw, so the caller's finally can clean it up regardless
            // of where we fail (extract throw, conflict throw, copy throw).
            tempFolder = Path.Combine(installDir, Guid.NewGuid().ToString());
            ZipFile.ExtractToDirectory(libFilePath, tempFolder);

            if (Directory.GetFiles(tempFolder).Length == 0 &&
                Directory.GetDirectories(tempFolder).Length == 0)
            {
                throw new InvalidOperationException(
                    "The library archive contains no entries.");
            }

            // Pick the staging directory whose contents will be copied into
            // installDir. If the outer ZIP wraps a single inner ZIP at its
            // top level (the engine-wrapped pattern), extract that inner ZIP
            // into a sibling subfolder of tempFolder and treat IT as the
            // content root. The inner ZIP is then walked and copied via the
            // same IsReparsePoint-guarded path as the no-inner-zip case --
            // so a future .NET runtime that honors symlink entries on Linux
            // (or a switch to a different extraction library) cannot smuggle
            // a symlink into installDir, even from an attacker-controlled
            // inner ZIP. The sub-folder lives inside tempFolder, so the
            // outer caller's finally cleans it up alongside the rest.
            string innerZipPath = FindInnerZip(tempFolder);
            string contentRoot;
            if (innerZipPath != null)
            {
                contentRoot = Path.Combine(tempFolder, "inner-content");
                Directory.CreateDirectory(contentRoot);
                ZipFile.ExtractToDirectory(innerZipPath, contentRoot);
            }
            else
            {
                contentRoot = tempFolder;
            }

            List<string> extractedFiles = CollectStagedFiles(contentRoot);

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
            string aliasSourceRelPath = DetermineAliasSource(aliasFileName, extractedFiles);
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

            ExtractContentToInstallDir(installDir, contentRoot);

            if (aliasSourceRelPath != null)
            {
                CreateAlias(installDir, aliasSourceRelPath, aliasFileName);
            }

            File.WriteAllLines(manifestPath, extractedFiles);
            Logging.Trace(
                $"Wrote manifest: {manifestPath} with {extractedFiles.Count} entries");
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
        /// Builds the list of relative paths under <paramref name="contentRoot"/>
        /// that will be installed into installDir. Reparse points are skipped
        /// (CollectRelativeFiles enforces this) so manifest entries always
        /// correspond to a regular file that CopyDirectory will physically copy.
        /// </summary>
        /// <remarks>
        /// Both the "outer ZIP only" and "outer ZIP wrapping inner ZIP" code
        /// paths route through this single on-disk walk. For the inner-zip
        /// case, contentRoot is the sub-folder into which InstallZipPackage
        /// extracted the inner ZIP; for the no-inner-zip case it IS tempFolder
        /// itself. Walking the on-disk tree (rather than enumerating ZIP
        /// entries) keeps the manifest aligned with what
        /// ExtractContentToInstallDir will actually copy after IsReparsePoint
        /// filtering.
        /// </remarks>
        private static List<string> CollectStagedFiles(string contentRoot)
        {
            List<string> extractedFiles = new List<string>();
            CollectRelativeFiles(contentRoot, "", extractedFiles);
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
        /// and we still need to create a root-level alias.
        ///
        /// Suppression is intentionally narrow: only a root-level file whose
        /// name EXACTLY equals <paramref name="aliasFileName"/> (i.e.
        /// "{libName}.dll" -- the file the loader will actually try to map)
        /// counts as "already discoverable". An earlier, looser check used
        /// <c>name.StartsWith("{libName}.")</c>, which incorrectly treated
        /// sidecars such as "{libName}.deps.json", "{libName}.runtimeconfig.json",
        /// or (in the libName-with-".dll"-suffix case) "{libName}.dll.config"
        /// as if they made the library loadable. They do not: the loader
        /// resolves "{libName}" to a PE binary by exact filename, so a
        /// sidecar at the root with no real DLL there would suppress alias
        /// creation and leave the install un-loadable. The exact-match rule
        /// keeps suppression aligned with what DllUtils can actually load.
        ///
        /// When more than one candidate DLL exists (no root-level
        /// "{aliasFileName}" but, say, both "lib/net8.0/foo.dll" and
        /// "lib/net6.0/bar.dll" are present), we deterministically pick the
        /// first by ordinal sort of the relative path. Without this sort the
        /// pick depends on <c>Directory.GetFiles</c> order, which is
        /// filesystem-defined: NTFS returns name-sorted, ext4 / XFS return
        /// inode-creation-order, and the same ZIP can yield different alias
        /// sources on different hosts. Ordinal sort gives stable, repeatable
        /// behavior across platforms and across re-installs.
        /// </remarks>
        private static string DetermineAliasSource(
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
                if (name.Equals(aliasFileName, s_pathComparison))
                {
                    // The file the loader will actually map is already at
                    // the root -- no alias needed. Sidecars matching
                    // "{libName}.*" are intentionally NOT treated as
                    // suppressing: see <remarks> above.
                    return null;
                }
            }
            // Pick the lexicographically-first .dll candidate so the choice
            // is stable across hosts/runs (see <remarks> above).
            string chosen = null;
            foreach (string relPath in extractedFiles)
            {
                if (!Path.GetExtension(relPath).Equals(".dll", s_pathComparison))
                {
                    continue;
                }
                if (chosen == null ||
                    string.CompareOrdinal(relPath, chosen) < 0)
                {
                    chosen = relPath;
                }
            }
            return chosen;
        }

        /// <summary>
        /// Copies the staged content from <paramref name="contentRoot"/>
        /// into the live <paramref name="installDir"/>, skipping reparse
        /// points at every level.
        /// </summary>
        /// <remarks>
        /// Both the "outer ZIP only" and "outer ZIP wrapping inner ZIP" code
        /// paths converge here. Inner-ZIP content has already been extracted
        /// to a sub-folder of tempFolder by InstallZipPackage; this method
        /// then copies it into installDir using the same IsReparsePoint
        /// guard at every recursion level. The guards must live at the call
        /// site for the top-level entries because CopyDirectory only checks
        /// the children it iterates, not its top-level sourceDir argument:
        /// a root-level reparse-point file could cause File.Copy to read
        /// data from outside the staged tree, and a root-level reparse-point
        /// directory could make CopyDirectory recurse out of contentRoot.
        /// </remarks>
        private static void ExtractContentToInstallDir(
            string installDir,
            string contentRoot)
        {
            foreach (string file in Directory.GetFiles(contentRoot))
            {
                if (IsReparsePoint(file))
                {
                    continue;
                }
                File.Copy(file, Path.Combine(installDir, Path.GetFileName(file)), false);
            }

            foreach (string dir in Directory.GetDirectories(contentRoot))
            {
                if (IsReparsePoint(dir))
                {
                    continue;
                }
                CopyDirectory(dir, Path.Combine(installDir, Path.GetFileName(dir)));
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
                            // Manifest covers everything we own: ZIP-extracted
                            // files AND -- since C3 -- the single "{libName}.dll"
                            // entry written by a raw-DLL install. CleanupManifest
                            // deletes them all.
                            CleanupManifest(manifestPath, installDir);
                        }
                        else
                        {
                            // Legacy fallback: a library installed by a pre-PR
                            // version of the extension would have been a raw DLL
                            // with no manifest. Remove "{libName}.dll" directly
                            // so older installs can still be uninstalled cleanly.
                            string libraryFile = Path.Combine(installDir, DllFileNameFor(libName));
                            if (File.Exists(libraryFile))
                            {
                                File.Delete(libraryFile);
                            }
                        }
                    }
                }
            }
            catch (Exception e)
            {
                Logging.Error(e.ToString());
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
        /// <para>
        /// Concrete attack scenario: a malicious ZIP entry named
        /// "sneaky.dll" is packed with Unix mode 0o120755 (symbolic link)
        /// and content "/etc/shadow". On a Linux runtime that materializes
        /// such entries as real symlinks during extraction, sneaky.dll
        /// becomes a symlink in tempFolder pointing at /etc/shadow.
        /// Without an IsReparsePoint guard, the subsequent
        /// <c>File.Copy(sneaky.dll, installDir/sneaky.dll, false)</c>
        /// would follow the link, READ /etc/shadow with the SQL Server
        /// service account's privileges, and write its contents into
        /// installDir/sneaky.dll -- which is then world-readable to any
        /// principal with read access to the library directory. With the
        /// guard, the symlink is skipped, the manifest does not list it,
        /// and the installed library directory contains only files that
        /// originated inside the ZIP. The same reasoning applies at the
        /// directory level: a reparse-point directory could redirect
        /// CopyDirectory's recursion into /, /home, or any other tree the
        /// service account can read, exfiltrating arbitrary file content
        /// into installDir.
        /// </para>
        /// Today's .NET ZipFile.ExtractToDirectory writes Unix-symlink-mode
        /// entries as regular files containing the link target text on
        /// every platform, so this scenario is theoretical against the
        /// current runtime; the guard is in place for the case where a
        /// future runtime starts honoring the bits, or we switch to a
        /// different extraction library. Regression test:
        /// <see cref="!:InnerZipFutureSymlinkRejectedTest"/> in
        /// CSharpLibraryTests.cpp.
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
        /// Implementation: open the file "{installDir}\install.lock"
        /// (INSIDE installDir) with FileShare.None. The lock must live
        /// inside installDir, not as a sibling of it, because SQL Server's
        /// per-database / per-language ExternalLibraries hierarchy ACLs the
        /// satellite's AppContainer SID with Modify on the per-user leaf
        /// directory only; the parent (e.g.
        /// "ExternalLibraries\&lt;dbid&gt;\&lt;langid&gt;\") grants the
        /// AppContainer Read access only, so a sibling placement
        /// "ExternalLibraries\&lt;dbid&gt;\&lt;langid&gt;\1.install.lock"
        /// fails with UnauthorizedAccessException at FileStream creation
        /// and the install path can never start.
        ///
        /// The OS releases the handle on process crash so there is no
        /// stale-lock risk. FileOptions.DeleteOnClose removes the lock
        /// file when the holder closes its handle, keeping the install
        /// area clean across runs.
        ///
        /// Library names that would collide with the lock filename
        /// ("install.lock", or "install" producing an "install.manifest")
        /// are not reserved here -- callers should not register such a
        /// name. The collision surfaces as a sharing violation at
        /// install time rather than silent overwrite.
        ///
        /// Acquisition blocks indefinitely with a 100ms retry interval.
        /// In the uncontended common case (single session), acquisition
        /// completes on the first attempt with no measurable overhead.
        /// </remarks>
        private static FileStream AcquireInstallLock(string installDir)
        {
            // Ensure installDir exists -- the lock file lives inside it.
            Directory.CreateDirectory(installDir);

            // Lock file lives INSIDE installDir. See remarks for the ACL
            // rationale (the satellite's AppContainer SID has write access
            // on installDir only, not on its parent).
            string lockPath = Path.Combine(installDir, "install.lock");

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
                catch (IOException ex) when (IsSharingViolation(ex))
                {
                    // Another holder has the lock; wait and retry. Any other
                    // IOException subtype (DirectoryNotFoundException,
                    // PathTooLongException, IO failure mid-creation, etc.)
                    // and anything outside IOException
                    // (UnauthorizedAccessException, ArgumentException,
                    // SecurityException, ...) propagates so that
                    // non-transient failures fail fast rather than spinning.
                    Thread.Sleep(s_lockRetryDelayMs);
                }
            }
        }

        /// <summary>
        /// Returns true when <paramref name="ex"/>'s HResult indicates that
        /// the underlying file is held by another process with an exclusive
        /// share -- i.e. the only situation in which retrying acquisition
        /// of the install lock makes sense.
        /// </summary>
        /// <remarks>
        /// Win32 maps two errors here:
        /// <list type="bullet">
        ///   <item>ERROR_SHARING_VIOLATION (32, 0x80070020) -- another open
        ///   handle's FileShare flags exclude the requested access.</item>
        ///   <item>ERROR_LOCK_VIOLATION (33, 0x80070021) -- a byte-range lock
        ///   conflicts with the requested access.</item>
        /// </list>
        /// .NET on Linux maps EAGAIN/EWOULDBLOCK/EBUSY to the same HResults
        /// when fileshare flags conflict, so the same constants work
        /// cross-platform.
        /// </remarks>
        private static bool IsSharingViolation(IOException ex)
        {
            int hr = ex.HResult & 0xFFFF;
            return hr == 32 || hr == 33;
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
        /// would be hidden dotfiles on Linux and opaque on both platforms;
        /// rejects whitespace-only names because Windows trims trailing
        /// whitespace from filenames silently and Linux behavior is
        /// surprising at best; and rejects Windows reserved DOS device
        /// names (CON, PRN, NUL, AUX, COM1-9, LPT1-9) because CreateFile
        /// maps any path ending in those stems to a device handle even on
        /// modern NTFS.
        /// </remarks>
        /// <param name="libName">
        /// The library name as supplied via libraryName to
        /// InstallExternalLibrary or UninstallExternalLibrary.
        /// </param>
        private static void ValidateLibraryName(string libName)
        {
            if (string.IsNullOrWhiteSpace(libName))
            {
                throw new ArgumentException("Library name must not be empty or whitespace.");
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
            string stem = Path.GetFileNameWithoutExtension(libName);
            if (string.IsNullOrEmpty(stem))
            {
                throw new ArgumentException(
                    $"Library name '{libName}' must not be only an extension.");
            }

            // Reject Windows reserved DOS device names. CreateFile interprets
            // any path whose final stem matches one of these as a handle to
            // the corresponding device, which makes "{installDir}/CON.dll"
            // and "{installDir}/CON.manifest" both behave unexpectedly. We
            // reject on every OS so behavior is consistent for libraries
            // moved between hosts.
            if (s_reservedDeviceNames.Contains(stem))
            {
                throw new ArgumentException(
                    $"Library name '{libName}' uses a reserved device name.");
            }
        }

        /// <summary>
        /// Returns true if <paramref name="path"/>'s filename ends with the
        /// ".zip" extension (case-insensitive). Dispatch is by extension --
        /// not by inspecting file content -- so the registered filename's
        /// intent is honored: a file named "foo.zip" is always treated as a
        /// ZIP archive (and fails loudly via ZipFile.ExtractToDirectory if
        /// the bytes are not a valid archive), and a file named "foo.dll"
        /// is always treated as a raw DLL (and copied as-is even if its
        /// bytes happen to start with "PK").
        /// </summary>
        /// <remarks>
        /// We previously sniffed the leading two "magic bytes" ("PK") to
        /// decide. That silently rewrote a malformed "foo.zip" upload into
        /// "foo.dll" on disk, hiding upload-corruption bugs from the caller
        /// and surprising readers of the install directory. Extension-based
        /// dispatch trades one form of robustness (tolerating misnamed
        /// files) for a more important one (predictable failures and no
        /// silent renaming of user-registered filenames).
        /// </remarks>
        private static bool HasZipExtension(string path)
        {
            return string.Equals(Path.GetExtension(path), ".zip",
                StringComparison.OrdinalIgnoreCase);
        }

        /// <summary>
        /// Decides whether to dispatch <paramref name="libName"/> /
        /// <paramref name="libFilePath"/> through the ZIP install path or
        /// the raw-DLL install path.
        /// </summary>
        /// <remarks>
        /// Precedence:
        /// <list type="number">
        ///   <item>If <paramref name="libName"/> ends in ".zip", dispatch as
        ///   a ZIP archive. The user wrote
        ///   <c>CREATE EXTERNAL LIBRARY [Foo.zip]</c>; their intent is
        ///   unambiguous.</item>
        ///   <item>If <paramref name="libName"/> ends in ".dll", dispatch
        ///   as a raw DLL. The user wrote
        ///   <c>CREATE EXTERNAL LIBRARY [Foo.dll]</c>; their intent is
        ///   unambiguous and the staged temp file contents must be a PE
        ///   binary (any attempt to parse it as a ZIP throws
        ///   InvalidDataException).</item>
        ///   <item>Otherwise, fall back to <paramref name="libFilePath"/>'s
        ///   extension. SQL Server's ExtHost passes a generated temp file
        ///   name with no semantic extension; some test fixtures, however,
        ///   register libraries under a bare name (e.g.
        ///   <c>"testpackageB"</c>) and point libFilePath at a fixture file
        ///   that does carry a meaningful ".zip" / ".dll" suffix. The
        ///   fallback preserves backward compatibility for those callers.
        ///   </item>
        /// </list>
        /// </remarks>
        private static bool DispatchAsZip(string libName, string libFilePath)
        {
            string libNameExt = Path.GetExtension(libName);
            if (string.Equals(libNameExt, ".zip",
                StringComparison.OrdinalIgnoreCase))
            {
                return true;
            }

            if (string.Equals(libNameExt, ".dll",
                StringComparison.OrdinalIgnoreCase))
            {
                return false;
            }

            return HasZipExtension(libFilePath);
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
                    // orphaned files don't disappear silently.
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
