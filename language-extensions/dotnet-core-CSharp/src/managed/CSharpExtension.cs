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
            Console.SetOut(new InteropTextWriter(1));
            Console.SetError(new InteropTextWriter(2));
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
            Console.SetOut(new InteropTextWriter(1));
            Console.SetError(new InteropTextWriter(2));
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
    }

    /// <summary>
    /// Custom TextWriter to redirect Console output to native file descriptors.
    /// </summary>
    internal class InteropTextWriter : System.IO.TextWriter
    {
        private readonly int _fd;

        public InteropTextWriter(int fd)
        {
            _fd = fd;
        }

        public override System.Text.Encoding Encoding => System.Text.Encoding.UTF8;

        public override void Write(string value)
        {
            if (value == null) return;
            byte[] buffer = System.Text.Encoding.UTF8.GetBytes(value);
            if (System.Runtime.InteropServices.RuntimeInformation.IsOSPlatform(System.Runtime.InteropServices.OSPlatform.Windows))
            {
                IntPtr handle = IntPtr.Zero;
                if (_fd == 1) handle = GetStdHandle(-11); // STD_OUTPUT_HANDLE
                else if (_fd == 2) handle = GetStdHandle(-12); // STD_ERROR_HANDLE
                
                if (handle != IntPtr.Zero && handle != (IntPtr)(-1))
                {
                    uint written;
                    WriteFile(handle, buffer, (uint)buffer.Length, out written, IntPtr.Zero);
                }
            }
            else
            {
                write(_fd, buffer, (uint)buffer.Length);
            }
        }

        public override void WriteLine(string value)
        {
            Write(value + Environment.NewLine);
        }

        [System.Runtime.InteropServices.DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool WriteFile(IntPtr hFile, byte[] lpBuffer, uint nNumberOfBytesToWrite, out uint lpNumberOfBytesWritten, IntPtr lpOverlapped);

        [System.Runtime.InteropServices.DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr GetStdHandle(int nStdHandle);

        [System.Runtime.InteropServices.DllImport("libc", EntryPoint = "write")]
        private static extern int write(int fd, byte[] buffer, uint count);
    }
}
