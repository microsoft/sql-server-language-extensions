//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: ManagedExtension.cs
//
// Purpose:
//  Implement the managed extensibility APIs
//
//*********************************************************************
using System;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using static Microsoft.SqlServer.CSharpExtension.Sql;

namespace Microsoft.SqlServer.CSharpExtension
{
    /// <summary>
    /// This class implements all extensibility APIs and returns results to native host
    /// </summary>
    public static unsafe class ManagedExtension
    {
        /// <summary>
        /// This declares the delegate type of Init
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
            return ExceptionUtils.WrapError(() =>
            {
                _languageParams = (languageParams == null) ? "" : Interop.UTF8PtrToStr(languageParams, languageParamsLen);
                _languagePath = (languagePath == null) ? "" :Interop.UTF8PtrToStr(languagePath, languagePathLen);
                _publicLibraryPath = (publicLibraryPath == null) ? "" :Interop.UTF8PtrToStr(publicLibraryPath, publicLibraryPathLen);
                _privateLibraryPath = (privateLibraryPath== null) ? "" :Interop.UTF8PtrToStr(privateLibraryPath, privateLibraryPathLen);
            });
        }

        /// <summary>
        /// This declares the delegate type of InitSession
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
        /// Length in bytes of PublicLibraryPath (excluding the null termination character).
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
            return ExceptionUtils.WrapError(() =>
            {
                var scriptStr = Interop.UTF8PtrToStr(script, scriptLength);
                var inputDataNameStr = Interop.UTF8PtrToStr(inputDataName, inputDataNameLength);
                var outputDataNameStr = Interop.UTF8PtrToStr(outputDataName, outputDataNameLength);

                _currentSession = new Session(
                    sessionId: sessionId,
                    taskId: taskId,
                    numTasks: numTasks,
                    script: scriptStr,
                    inputSchemaColumnsNumber: inputSchemaColumnsNumber,
                    parametersNumber: parametersNumber,
                    inputDataName: inputDataNameStr,
                    outputDataName: outputDataNameStr);
            });
        }

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
            return ExceptionUtils.WrapError(() =>
            {
                _currentSession.InitInputColumn(
                    columnNumber: columnNumber,
                    columnName: Interop.UTF8PtrToStr(columnName, (ulong)columnNameLength),
                    dataType: dataType,
                    decimalDigits: decimalDigits,
                    nullable: nullable,
                    columnSize: columnSize);
            });
        }

        public delegate short InitParamDelegate(
            Guid   sessionId,
            ushort taskId,
            ushort paramNumber,
            char   *paramName,
            short  paramNameLength,
            short  dataType,
            ulong  argSize,
            short  decimalDigits,
            void   *argValue,
            int    strLenOrNullMap,
            short  inputOutputType);

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
            return ExceptionUtils.WrapError(() =>
            {
                _currentSession.InitParam(
                paramNumber: paramNumber,
                paramName: Interop.UTF8PtrToStr(paramName, (ulong)paramNameLength),
                dataType: dataType,
                paramSize: paramSize,
                decimalDigits: decimalDigits,
                paramValue: paramValue,
                strLenOrNullMap: strLenOrNullMap,
                inputOutputType: inputOutputType);
            });
        }

        public delegate short ExecuteDelegate(
            Guid   sessionId,
            ushort taskId,
            ulong  rowsNumber,
            void   **data,
            int    **strLenOrNullMap,
            ushort *outputSchemaColumnsNumber);

        public static short Execute(
            Guid   sessionId,
            ushort taskId,
            ulong  rowsNumber,
            void   **data,
            int    **strLenOrNullMap,
            ushort *outputSchemaColumnsNumber)
        {
            return ExceptionUtils.WrapError(() =>
            {
                _currentSession.Execute(rowsNumber, data, strLenOrNullMap, outputSchemaColumnsNumber);
            });
        }

        public delegate short GetResultColumnDelegate(
            Guid   sessionId,
            ushort taskId,
            ushort columnNumber,
            short  *dataType,
            ulong  *columnSize,
            short  *decimalDigits,
            short  *nullable);

        public static short GetResultColumn(
            Guid   sessionId,
            ushort taskId,
            ushort columnNumber,
            short  *dataType,
            ulong  *columnSize,
            short  *decimalDigits,
            short  *nullable)
        {
            return SQL_SUCCESS;
        }

        public delegate short GetResultsDelegate(
            Guid   sessionId,
            ushort TaskId,
            ulong  *rowsNumber,
            void   **data,
            int    ***strLenOrNullMap);

        public static short GetResults(
            Guid   sessionId,
            ushort TaskId,
            ulong  *rowsNumber,
            void   ***data,
            int    ***strLenOrNullMap)
        {
            return SQL_SUCCESS;
        }

        public delegate short GetOutputParamDelegate(
            Guid   sessionId,
            ushort taskId,
            ushort paramNumber,
            void   *paramValue,
            int    *strLenOrNullMap);

        public static short GetOutputParam(
            Guid   sessionId,
            ushort taskId,
            ushort paramNumber,
            void   *paramValue,
            int    *strLenOrNullMap)
        {
            return SQL_SUCCESS;
        }

        public delegate short CleanupSessionDelegate(
            Guid   sessionId,
            ushort taskId);

        public static short CleanupSession(
            Guid   sessionId,
            ushort taskId)
        {
            return ExceptionUtils.WrapError(() =>
            {
                _currentSession = null;
            });
        }

        private static Session _currentSession = null;
        private static string _languagePath = null;
        private static string _publicLibraryPath = null;
        private static string _privateLibraryPath = null;
        private static string _languageParams = null;
    }
}
