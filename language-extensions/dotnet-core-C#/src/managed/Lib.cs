//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: Lib.cs
//
// Purpose:
//  Implement the managed extensibility APIs
//
//*********************************************************************
using System;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Microsoft.SqlServer.CSharpExtension
{
    /// <summary>
    /// This class implements all extensibility APIs and returns results to native host
    /// </summary>
    public static unsafe class Lib
    {
        /// <summary>
        /// This declares the delegate type of Init
        /// </summary>
        public delegate short InitDelegate(
            char* languageParams,
            ulong languageParamsLen,
            char* languagePath,
            ulong languagePathLen,
            char* publicLibraryPath,
            ulong publicLibraryPathLen,
            char* privateLibraryPath,
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
        /// SQL_SUCCESS(0), SQL_ERROR(1)
        /// </returns>
        public static short Init(
            char* languageParams,
            ulong languageParamsLen,
            char* languagePath,
            ulong languagePathLen,
            char* publicLibraryPath,
            ulong publicLibraryPathLen,
            char* privateLibraryPath,
            ulong privateLibraryPathLen)
        {
            return 0;
        }

        public delegate short InitSessionDelegate(
            Guid sessionId,
            ushort taskId,
            ushort numTasks,
            char* script,
            ulong scriptLength,
            ushort inputSchemaColumnsNumber,
            ushort parametersNumber,
            char* inputDataName,
            ushort inputDataNameLength,
            char* outputDataName,
            ushort outputDataNameLength);

        public static short InitSession(
            Guid sessionId,
            ushort taskId,
            ushort numTasks,
            char* script,
            ulong scriptLength,
            ushort inputSchemaColumnsNumber,
            ushort parametersNumber,
            char* inputDataName,
            ushort inputDataNameLength,
            char* outputDataName,
            ushort outputDataNameLength)
        {
            return 0;
        }

        public delegate short InitColumnDelegate(
            char* sessionId,
            ushort taskId,
            ushort columnNumber,
            char* columnName,
            ushort columnNameLength,
            short dataType,
            ulong columnSize,
            short decimalDigits,
            short nullable,
            ushort partitionByNumber,
            ushort orderByNumber);

        public static short InitColumn(
            char* sessionId,
            ushort taskId,
            ushort columnNumber,
            char* columnName,
            ushort columnNameLength,
            short dataType,
            ulong columnSize,
            short decimalDigits,
            short nullable,
            ushort partitionByNumber, 
            ushort orderByNumber)     
        {
            return 0;
        }

        public delegate short InitParamDelegate(
            char* sessionId,
            ushort taskId,
            ushort paramNumber,
            char* paramName,
            ushort paramNameLength,
            short dataType,
            ulong argSize,
            short decimalDigits,
            void* argValue,
            int strLenOrNullMap,
            short inputOutputType);
        
        public static short InitParam(
            char* sessionId,
            ushort taskId,
            ushort paramNumber,
            char* paramName,
            ushort paramNameLength,
            short dataType,
            ulong argSize,
            short decimalDigits,
            void* argValue,
            int strLenOrNullMap,
            short inputOutputType)
        {
            return 0;
        }

        public delegate short ExecuteDelegate(
            char* sessionId,
            ushort taskId,
            ulong rowsNumber,
            void** data,
            int** strLenOrNullMap,
            ushort* outputSchemaColumnsNumber);
    
        public static short Execute(
            char* sessionId,
            ushort taskId,
            ulong rowsNumber,
            void** data,
            int** strLenOrNullMap,
            ushort* outputSchemaColumnsNumber)
        {
            return 0;
        }

        public delegate short GetResultColumnDelegate(
            char* sessionId,
            ushort taskId,
            ushort columnNumber,
            short* dataType,
            ulong* columnSize,
            short* decimalDigits,
            short* nullable);
        public static short GetResultColumn(
            char* sessionId,
            ushort taskId,
            ushort columnNumber,
            short* dataType,
            ulong* columnSize,
            short* decimalDigits,
            short* nullable)
        {
            return 0;
        }

        public delegate short GetResultsDelegate(
            char* SessionId,
            ushort TaskId,
            ulong* rowsNumber,
            void*** data,
            int*** strLenOrNullMap);
        
        public static short GetResults(
            char* SessionId,
            ushort TaskId,
            ulong* rowsNumber,
            void*** data,
            int*** strLenOrNullMap)
        {
            return 0;
        }
        
        public delegate short GetOutputParamDelegate(
            char* sessionId,
            ushort taskId,
            ushort paramNumber,
            void* paramValue,
            int* strLenOrNullMap);

        public static short GetOutputParam(
            char* sessionId,
            ushort taskId,
            ushort paramNumber,
            void* paramValue,
            int* strLenOrNullMap)
        {
            return 0;
        }

        public delegate short CleanupSessionDelegate(
            char* sessionId,
            ushort taskId);

        public static short CleanupSession(
            char* sessionId,
            ushort taskId)
        {
            return 0;
        }
    }
}
