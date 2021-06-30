//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: Session.cs
//
// Purpose:
//  Class encapsulating operations performed per session
//
//*********************************************************************
using System;
using System.Collections.Generic;

namespace Microsoft.SqlServer.CSharpExtension
{
    /// <summary>
    /// This class pertain data to a session and encapsulate
    /// operations performed per session
    /// </summary>
    unsafe class Session
    {
        /// <summary>
        /// Initializes the session with constructor
        /// </summary>
        public Session(
            Guid   sessionId,
            ushort taskId,
            ushort numTasks,
            string script,
            ushort inputSchemaColumnsNumber,
            ushort parametersNumber,
            string inputDataName,
            string outputDataName)
        {
            _sessionId = sessionId;
            _taskId = taskId;
            _numTasks = numTasks;
            _script = script;
            _inputSchemaColumnsNumber = inputSchemaColumnsNumber;
            _parametersNumber = parametersNumber;
            _inputDataName = inputDataName;
            _outputDataName = outputDataName;
        }

        /// <summary>
        /// Initializes the input column for this session
        /// </summary>
        public void InitInputColumn(
            ushort columnNumber,
            string columnName,
            ulong  columnSize,
            short  dataType,
            short  nullable,
            short  decimalDigits)
        {
        }

        /// <summary>
        /// Initializes the parameter for this session
        /// </summary>
        public void InitParam(
            ushort paramNumber,
            string paramName,
            short  dataType,
            ulong  paramSize,
            short  decimalDigits,
            void   *paramValue,
            int    strLenOrNullMap,
            short  inputOutputType)
        {
        }

        /// <summary>
        /// Execute the workflow for the session
        /// </summary>
        public void Execute(
            ulong  rowsNumber,
            void   **data,
            int    **strLenOrNullMap,
            ushort *outputSchemaColumnsNumber)
        {
        }

        /// <summary>
        /// Returns information about the output column
        /// </summary>
        public short GetResultColumn(
            ushort columnNumber,
            short  *dataType,
            ulong  *columnSize,
            short  *decimalDigits,
            short  *nullable)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// Returns the data and size of the output parameter
        /// </summary>
        public short GetOutputParam(
            ushort paramNumber,
            void   *paramValue,
            int    *strLenOrNullMap)
        {
            throw new NotImplementedException();
        }

        private Guid   _sessionId;
        private ushort _taskId;
        private ushort _numTasks;
        private string _script;
        private ushort _inputSchemaColumnsNumber;
        private ushort _parametersNumber;
        private string _inputDataName;
        private string _outputDataName;
    }
}
