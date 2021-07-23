//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpSession.cs
//
// Purpose:
//  Class encapsulating operations performed per session
//
//*********************************************************************
using System;
using System.Collections.Generic;
using Microsoft.Data.Analysis;
using Microsoft.SqlServer.CSharpExtension.SDK;
using static Microsoft.SqlServer.CSharpExtension.Sql;

namespace Microsoft.SqlServer.CSharpExtension
{
    /// <summary>
    /// This class pertain data to a session and encapsulate
    /// operations performed per session.
    /// </summary>
    unsafe class CSharpSession
    {
        /// <summary>
        /// GUID uniquely identifying this script session.
        /// </summary>
        private Guid   _sessionId;

        /// <summary>
        /// An integer uniquely identifying this execution process.
        /// </summary>
        private ushort _taskId;

        /// <summary>
        /// Number of tasks for this session.
        /// </summary>
        private ushort _numTasks;

        /// <summary>
        /// Null-terminated UTF-8 string containing the @script in sp_execute_external_script.
        /// </summary>
        private string _script;

        /// <summary>
        /// Number of columns in the result set from @input_data_1 in sp_execute_external_script.
        /// </summary>
        private ushort _inputSchemaColumnsNumber;

        /// <summary>
        /// Number of input parameters from @params in sp_execute_external_script.
        /// </summary>
        private ushort _parametersNumber;

        /// <summary>
        /// Null-terminated UTF-8 string containing the @input_data_1_name in sp_execute_external_script.
        /// </summary>
        private string _inputDataName;

        /// <summary>
        /// Null-terminated UTF-8 string containing the @output_data_1_name in sp_execute_external_script.
        /// </summary>
        private string _outputDataName;

        /// <summary>
        /// Input DataSet containing input columns
        /// </summary>
        private CSharpInputDataSet _inputDataSet;

        /// <summary>
        /// Parameter containter containing intput/output parameters
        /// </summary>
        private CSharpParamContainer _paramContainer;

        /// <summary>
        /// User Dll containing namespace and path.
        /// The user dll is expected to implement the SDK.
        /// </summary>
        private CSharpUserDll _userDll;

        /// <summary>
        /// This method initializes the session with constructor.
        /// </summary>
        public CSharpSession(
            Guid          sessionId,
            ushort        taskId,
            ushort        numTasks,
            string        script,
            ushort        inputSchemaColumnsNumber,
            ushort        parametersNumber,
            string        inputDataName,
            string        outputDataName,
            CSharpUserDll userDll)
        {
            Logging.Trace("CSharpSession::CSharpSession");
            _sessionId = sessionId;
            _taskId = taskId;
            _numTasks = numTasks;
            _script = script;
            _inputSchemaColumnsNumber = inputSchemaColumnsNumber;
            _parametersNumber = parametersNumber;
            _inputDataName = inputDataName;
            _outputDataName = outputDataName;
            _userDll = userDll;
            _inputDataSet = new CSharpInputDataSet
            {
                Name = inputDataName,
                ColumnsNumber = inputSchemaColumnsNumber
            };

            _paramContainer = new CSharpParamContainer(parametersNumber);
        }

        /// <summary>
        /// This method initializes the input column for this session.
        /// </summary>
        public void InitInputColumn(
            ushort       columnNumber,
            string       columnName,
            ulong        columnSize,
            SqlDataType  dataType,
            short        nullable,
            short        decimalDigits)
        {
            Logging.Trace("CSharpSession::InitInputColumn");
            _inputDataSet.InitColumn(
                columnNumber,
                columnName,
                columnSize,
                dataType,
                nullable,
                decimalDigits
            );
        }

        /// <summary>
        /// This method initializes the parameter for this session.
        /// </summary>
        public void InitParam(
            ushort       paramNumber,
            string       paramName,
            SqlDataType  dataType,
            ulong        paramSize,
            short        decimalDigits,
            void         *paramValue,
            int          strLenOrNullMap,
            short        inputOutputType)
        {
            Logging.Trace("CSharpSession::InitParam");
            _paramContainer.AddParam(
                paramNumber,
                paramName,
                dataType,
                paramSize,
                decimalDigits,
                paramValue,
                strLenOrNullMap,
                inputOutputType
            );
        }

        /// <summary>
        /// This method executes the workflow for the session.
        /// </summary>
        public void Execute(
            ulong  rowsNumber,
            void   **data,
            int    **strLenOrNullMap,
            ushort *outputSchemaColumnsNumber)
        {
            Logging.Trace("CSharpSession::Execute");
            _inputDataSet.AddColumns(rowsNumber, data, strLenOrNullMap);
            AbstractSqlServerExtensionExecutor userExecutor = _userDll.InstantiateUserExecutor();
            DataFrame outputDataFrame = userExecutor.Execute(_inputDataSet.InputDataFrame, _paramContainer.UserParams);
        }

        /// <summary>
        /// This method returns information about the output column.
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
        /// This method returns the data and size of the output parameter.
        /// </summary>
        public short GetOutputParam(
            ushort paramNumber,
            void   *paramValue,
            int    *strLenOrNullMap)
        {
            throw new NotImplementedException();
        }
    }
}
