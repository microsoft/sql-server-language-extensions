//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpParam.cs
//
// Purpose:
//  A container that stores the input and output parameters.
//
//*************************************************************************************************
using System;
using System.Runtime;
using System.Text;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using static Microsoft.SqlServer.CSharpExtension.Sql;

namespace Microsoft.SqlServer.CSharpExtension
{
    /// <summary>
    /// This class stores input/output parameter.
    /// </summary>
    public class CSharpParamContainer
    {
        /// <summary>
        /// This Dictionary stores parameter number as key and CSharpParam object as value.
        /// </summary>
        private Dictionary<ushort, CSharpParam> _params;

        /// <summary>
        /// A list of parameter value handles to prevent garbage collector releasing the memory before the objects are used.
        /// </summary>
        private List<GCHandle> _handleList;

        /// <summary>
        /// Number of input parameters from @params in sp_execute_external_script.
        /// </summary>
        public ushort TotalParams { get; private set; }

        /// <summary>
        /// This property stores the parameters that will be passed to user. The key is the paramter
        /// name and the value is the parameter value.
        /// </summary>
        public Dictionary<string, dynamic> UserParams { get; private set; }

        /// <summary>
        /// This constructor initializes the UserParams with an empty dictionary and stores TotalParams.
        /// </summary>
        public CSharpParamContainer(ushort parametersNumber)
        {
            TotalParams = parametersNumber;
            UserParams = new Dictionary<string, dynamic>();
            _params = new Dictionary<ushort, CSharpParam>();
            _handleList = new List<GCHandle>();
        }

        /// <summary>
        /// This method adds a CSharpParam to the container
        /// and adds to user parameter dictionary as well.
        /// </summary>
        public unsafe void AddParam(
            ushort       paramNumber,
            string       paramName,
            SqlDataType  dataType,
            ulong        paramSize,
            short        decimalDigits,
            void         *paramValue,
            int          strLenOrNullMap,
            short        inputOutputType)
        {
            Logging.Trace("CSharpParamContainer::AddParam");
            if (paramName == null)
            {
                throw new ArgumentException("Invalid input parameter name supplied");
            }

            else if (paramNumber >= TotalParams || paramNumber < 0)
            {
                throw new ArgumentException("Invalid input parameter id supplied: " + paramNumber.ToString());
            }

            _params[paramNumber] = new CSharpParam{
                DataType = dataType,
                DecimalDigits = decimalDigits,
                Name = paramName,
                InputOutputType = inputOutputType,
                Number = paramNumber,
                Size = paramSize,
                StrLenOrNullMap = strLenOrNullMap
            };

            if (strLenOrNullMap == SQL_NULL_DATA)
            {
                return;
            }

            switch(dataType)
            {
                case SqlDataType.DotNetInteger:
                    _params[paramNumber].Value = *(int*)paramValue;
                    break;
                case SqlDataType.DotNetUInteger:
                    _params[paramNumber].Value = *(uint*)paramValue;
                    break;
                case SqlDataType.DotNetBigInt:
                    _params[paramNumber].Value = *(long*)paramValue;
                    break;
                case SqlDataType.DotNetUBigInt:
                    _params[paramNumber].Value = *(ulong*)paramValue;
                    break;
                case SqlDataType.DotNetTinyInt:
                    _params[paramNumber].Value = *(sbyte*)paramValue;
                    break;
                case SqlDataType.DotNetUTinyInt:
                    _params[paramNumber].Value = *(byte*)paramValue;
                    break;
                case SqlDataType.DotNetSmallInt:
                    _params[paramNumber].Value = *(short*)paramValue;
                    break;
                case SqlDataType.DotNetUSmallInt:
                    _params[paramNumber].Value = *(ushort*)paramValue;
                    break;
                case SqlDataType.DotNetDouble:
                    _params[paramNumber].Value = *(double*)paramValue;
                    break;
                case SqlDataType.DotNetFloat:
                    _params[paramNumber].Value = *(double*)paramValue;
                    break;
                case SqlDataType.DotNetReal:
                    _params[paramNumber].Value = *(float*)paramValue;
                    break;
                case SqlDataType.DotNetBit:
                    _params[paramNumber].Value = *(bool*)paramValue;
                    break;
                case SqlDataType.DotNetChar:
                    _params[paramNumber].Value = Interop.UTF8PtrToStr((char*)paramValue, (ulong)strLenOrNullMap);
                    break;
                case SqlDataType.DotNetWChar:
                    // strLenOrNullMap for wide chars is in BYTES from ODBC; convert to UTF-16 code units
                    int charLen = strLenOrNullMap / 2;
                    _params[paramNumber].Value = Interop.WCharPtrToStr((char*)paramValue, (ulong)charLen);
                    break;
                default:
                    throw new NotImplementedException("Parameter type for " + dataType.ToString() + " has not been implemented yet");
            }

            UserParams[paramName] = _params[paramNumber].Value;
        }

        /// <summary>
        /// This method replaces the parameter's ODBC-format input value stored in the container
        ///	with the updated output value from execution parameters' dictionary
        /// </summary>
        public unsafe void ReplaceParam(
            ushort paramNumber,
            void   **paramValue,
            int    *strLenOrNullMap)
        {
            Logging.Trace("CSharpParamContainer::ReplaceParam");
            if(!UserParams.ContainsKey(_params[paramNumber].Name))
            {
                *strLenOrNullMap = SQL_NULL_DATA;
                return;
            }

            _params[paramNumber].Value = UserParams[_params[paramNumber].Name];
            CSharpParam param = _params[paramNumber];
            if(param.Value == null)
            {
                *strLenOrNullMap = SQL_NULL_DATA;
                return;
            }

            *strLenOrNullMap = (int)param.Size;
            switch(param.DataType)
            {
                case SqlDataType.DotNetInteger:
                    ReplaceNumericParam<int>((int)param.Value, paramValue);
                    break;
                case SqlDataType.DotNetUInteger:
                    ReplaceNumericParam<uint>((uint)param.Value, paramValue);
                    break;
                case SqlDataType.DotNetBigInt:
                    ReplaceNumericParam<long>((long)param.Value, paramValue);
                    break;
                case SqlDataType.DotNetUBigInt:
                    ReplaceNumericParam<ulong>((ulong)param.Value, paramValue);
                    break;
                case SqlDataType.DotNetTinyInt:
                    ReplaceNumericParam<sbyte>((sbyte)param.Value, paramValue);
                    break;
                case SqlDataType.DotNetUTinyInt:
                    ReplaceNumericParam<byte>((byte)param.Value, paramValue);
                    break;
                case SqlDataType.DotNetSmallInt:
                    ReplaceNumericParam<short>((short)param.Value, paramValue);
                    break;
                case SqlDataType.DotNetUSmallInt:
                    ReplaceNumericParam<ushort>((ushort)param.Value, paramValue);
                    break;
                case SqlDataType.DotNetDouble:
                    ReplaceNumericParam<double>((double)param.Value, paramValue);
                    break;
                case SqlDataType.DotNetFloat:
                    ReplaceNumericParam<double>((double)param.Value, paramValue);
                    break;
                case SqlDataType.DotNetReal:
                    ReplaceNumericParam<float>((float)param.Value, paramValue);
                    break;
                case SqlDataType.DotNetBit:
                    bool boolValue = Convert.ToBoolean(param.Value);
                    ReplaceNumericParam<bool>(boolValue, paramValue);
                    break;
                case SqlDataType.DotNetChar:
                    *strLenOrNullMap = (param.Value.Length < *strLenOrNullMap) ? param.Value.Length : *strLenOrNullMap;
                    ReplaceStringParam((string)param.Value, paramValue);
                    break;
                case SqlDataType.DotNetWChar:
                    // Capacity (*strLenOrNullMap) currently holds max bytes from metadata. Compute bytes for value.
                    int capacityBytesW = *strLenOrNullMap;
                    int valueBytesW = ((string)param.Value).Length * 2;
                    *strLenOrNullMap = (valueBytesW < capacityBytesW) ? valueBytesW : capacityBytesW;
                    ReplaceWCharParam((string)param.Value, paramValue);
                    break;
                default:
                    throw new NotImplementedException("Parameter type for " + param.DataType.ToString() + " has not been implemented yet");
            }
        }

        /// <summary>
        /// This method cleans up all the handles for garbage collection.
        /// </summary>
        public void HandleCleanup()
        {
            Logging.Trace("CSharpParamContainer:HandleCleanup");
            if(_handleList != null)
            {
                foreach(GCHandle handle in _handleList)
                {
                    handle.Free();
                }
            }

            _handleList = null;
        }

        /// <summary>
        /// This method replaces parameter value for numeric data types.
        /// </summary>
        private unsafe void ReplaceNumericParam<T>(
            T    value,
            void **paramValue) where T : unmanaged
        {
            // Allocate a single-element array so we can pin stable managed memory.
            T[] holder = new T[1];
            holder[0] = value;
            GCHandle handle = GCHandle.Alloc(holder, GCHandleType.Pinned);
            _handleList.Add(handle);
            *paramValue = (void*)handle.AddrOfPinnedObject();
        }

        /// <summary>
        /// This method replaces parameter value for string data types.
        /// If the string is not empty, the address of underlying bytes will be assigned to paramValue.
        /// </summary>
        private unsafe void ReplaceStringParam(
            string value,
            void   **paramValue
        )
        {
            if(string.IsNullOrEmpty(value))
            {
                _handleList.Add(GCHandle.Alloc(value));
                fixed(void* strPtr = value)
                {
                    *paramValue = strPtr;
                }
            }
            else
            {
                byte[] strBytes = Encoding.UTF8.GetBytes(value);
                _handleList.Add(GCHandle.Alloc(strBytes));
                fixed(void* strPtr = strBytes)
                {
                    *paramValue = strPtr;
                }
            }
        }

        /// <summary>
        /// This method replaces parameter value for wide string (UTF-16) data types.
        /// </summary>
        private unsafe void ReplaceWCharParam(
            string value,
            void   **paramValue
        )
        {
            if(string.IsNullOrEmpty(value))
            {
                _handleList.Add(GCHandle.Alloc(value));
                fixed(void* wStrPtr = value)
                {
                    *paramValue = wStrPtr;
                }
            }
            else
            {
                // For wide char we can pin the string directly (UTF-16 in .NET)
                _handleList.Add(GCHandle.Alloc(value));
                fixed(void* wStrPtr = value)
                {
                    *paramValue = wStrPtr;
                }
            }
        }
    }
}
