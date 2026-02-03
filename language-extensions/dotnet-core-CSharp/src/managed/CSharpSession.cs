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
using System.Runtime.InteropServices;
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
        /// Output DataSet containing output columns
        /// </summary>
        private CSharpOutputDataSet _outputDataSet;

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

            _outputDataSet = new CSharpOutputDataSet
            {
                Name = outputDataName,
                ColumnsNumber = 0
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
            _userDll.UserExecutor = _userDll.InstantiateUserExecutor();

            if(_userDll.UserExecutor != null)
            {
                _outputDataSet.CSharpDataFrame = _userDll.UserExecutor.Execute(_inputDataSet.CSharpDataFrame, _paramContainer.UserParams);
            }

            if(_outputDataSet.CSharpDataFrame != null)
            {
                _outputDataSet.ColumnsNumber = (ushort)_outputDataSet.CSharpDataFrame.Columns.Count;

                // Pass explicit output column types from user executor (e.g., SqlTypes.NVARCHAR)
                //
                if (_userDll.UserExecutor != null && _userDll.UserExecutor.OutputColumnTypes.Count > 0)
                {
                    _outputDataSet.SetExplicitOutputColumnTypes(_userDll.UserExecutor.OutputColumnTypes);
                }

                // Pass input column metadata to preserve SQL types (e.g., nvarchar) for output columns
                //
                _outputDataSet.SetInputColumnMetadata(_inputDataSet.Columns);
                _outputDataSet.ExtractColumns(_outputDataSet.CSharpDataFrame);
                *outputSchemaColumnsNumber = _outputDataSet.ColumnsNumber;
            }
            else
            {
                *outputSchemaColumnsNumber = 0;
            }
        }

        /// <summary>
        /// This method returns information about the output column.
        /// </summary>
        public void GetResultColumn(
            ushort columnNumber,
            short  *dataType,
            ulong  *columnSize,
            short  *decimalDigits,
            short  *nullable)
        {
            Logging.Trace("CSharpSession::GetResultColumn");
            if (columnNumber >= _outputDataSet.ColumnsNumber || columnNumber < 0)
            {
                throw new ArgumentException("Invalid input column id supplied: " + columnNumber.ToString());
            }

            *dataType = ToSQLDataType(_outputDataSet[columnNumber].DataType);
            *columnSize = _outputDataSet[columnNumber].Size;
            *decimalDigits = _outputDataSet[columnNumber].DecimalDigits;
            *nullable = _outputDataSet[columnNumber].Nullable;
        }

        /// <summary>
        /// This method retrieves the result set from executing the @script in sp_execute_external_script.
        /// </summary>
        public void GetResults(
            ulong    *rowsNumber,
            void     ***data,
            int      ***strLenOrNullMap)
        {
            Logging.Trace("CSharpSession::GetResults");
            if(_outputDataSet.CSharpDataFrame != null)
            {
                *rowsNumber = (ulong)_outputDataSet.CSharpDataFrame.Rows.Count;
                _outputDataSet.RetrieveColumns(data, strLenOrNullMap);
            }
            else
            {
                *rowsNumber = 0;
            }
        }

        /// <summary>
        /// This method returns the data and size of the output parameter.
        /// </summary>
        public void GetOutputParam(
            ushort         paramNumber,
            void           **paramValue,
            int            *strLenOrNullMap)
        {
            Logging.Trace("CSharpSession::GetOutputParam");
            if(paramValue != null && strLenOrNullMap != null)
            {
                _paramContainer.ReplaceParam(paramNumber, paramValue, strLenOrNullMap);
            }
            else
            {
                throw new ArgumentException("Unexpected null error occurred");
            }
        }

        /// <summary>
        /// This method cleans up per-session information.
        /// </summary>
        public void CleanupSession()
        {
            Logging.Trace("CSharpSession::CleanupSession");
            _paramContainer.HandleCleanup();
            _outputDataSet.HandleCleanup();
        }
    }
}
