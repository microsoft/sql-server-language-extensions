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
using System.Data.SqlTypes;
using System.Runtime;
using System.Text;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using static Microsoft.SqlServer.CSharpExtension.Sql;
using static Microsoft.SqlServer.CSharpExtension.SqlNumericHelper;

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
                case SqlDataType.DotNetNumeric:
                    // Convert SQL_NUMERIC_STRUCT to SqlDecimal, handling OUTPUT parameter sentinel (precision=0)
                    //
                    _params[paramNumber].Value = ToSqlDecimalFromPointer((SqlNumericStruct*)paramValue);
                    break;
                case SqlDataType.DotNetChar:
                    _params[paramNumber].Value = Interop.UTF8PtrToStr((char*)paramValue, (ulong)strLenOrNullMap);
                    break;
                case SqlDataType.DotNetWChar:
                    // For NCHAR/NVARCHAR, strLenOrNullMap contains byte length.
                    // In C#, sizeof(char) is always 2 bytes (UTF-16), regardless of platform.
                    // Note: C++ wchar_t is 2 bytes on Windows but 4 bytes on Linux - this extension only supports Windows.
                    // The cast to (char*) is correct here because C# char is UTF-16 (same as Windows wchar_t).
                    //
                    _params[paramNumber].Value = Interop.UTF16PtrToStr((char*)paramValue, strLenOrNullMap / sizeof(char));
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
            if(!UserParams.TryGetValue(_params[paramNumber].Name, out object paramValue_))
            {
                *strLenOrNullMap = SQL_NULL_DATA;
                return;
            }

            _params[paramNumber].Value = paramValue_;
            CSharpParam param = _params[paramNumber];
            
            // Use null-coalescing pattern for safer null checking with value types
            // SqlDecimal is a struct, so we need to check both object null and SqlDecimal.IsNull
            //
            if(ReferenceEquals(param.Value, null))
            {
                *strLenOrNullMap = SQL_NULL_DATA;
                return;
            }
            
            // Special handling for SqlDecimal.Null (SqlDecimal is a struct, not a class)
            //
            if(param.DataType == SqlDataType.DotNetNumeric && param.Value is SqlDecimal sqlDecVal && sqlDecVal.IsNull)
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
                case SqlDataType.DotNetNumeric:
                    // Use declared precision/scale from T-SQL parameter definition, not SqlDecimal's intrinsic values
                    //
                    if (param.Value is SqlDecimal sqlDecimalValue)
                    {
                        byte precision = (byte)param.Size;
                        byte scale = (byte)param.DecimalDigits;
                        *strLenOrNullMap = Sql.SqlNumericStructSize;
                        ReplaceNumericStructParam(sqlDecimalValue, precision, scale, paramValue);
                    }
                    else
                    {
                        throw new InvalidCastException($"Expected SqlDecimal for NUMERIC parameter, got {param.Value?.GetType().Name ?? "null"}");
                    }
                    break;
                case SqlDataType.DotNetChar:
                    // strLenOrNullMap = byte length (ANSI: 1 byte per character)
                    //
                    int charByteLen = param.Value.Length;
                    int charMaxByteLen = (int)param.Size;
                    *strLenOrNullMap = (charByteLen < charMaxByteLen) ? charByteLen : charMaxByteLen;
                    ReplaceStringParam((string)param.Value, paramValue);
                    break;
                case SqlDataType.DotNetWChar:
                    // strLenOrNullMap = byte length (UTF-16: 2 bytes per character)
                    //
                    // so we multiply by sizeof(char) to convert to bytes.
                    //
                    int wcharByteLen = param.Value.Length * sizeof(char);
                    int wcharMaxByteLen = (int)param.Size * sizeof(char);
                    *strLenOrNullMap = (wcharByteLen < wcharMaxByteLen) ? wcharByteLen : wcharMaxByteLen;
                    ReplaceUnicodeStringParam((string)param.Value, paramValue);
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
        /// Uses proper memory pinning to ensure the value remains valid after method returns.
        /// </summary>
        private unsafe void ReplaceNumericParam<T>(
            T    value,
            void **paramValue) where T : unmanaged
        {
            // Box the value into a single-element array to create a heap-allocated copy, then pin it.
            // This ensures the pointer remains valid after the method returns.
            //
            T[] valueArray = new T[1] { value };
            GCHandle handle = GCHandle.Alloc(valueArray, GCHandleType.Pinned);
            _handleList.Add(handle);
            *paramValue = (void*)handle.AddrOfPinnedObject();
        }

        /// <summary>
        /// Replaces NUMERIC/DECIMAL parameter value by converting SqlDecimal to SQL_NUMERIC_STRUCT and pinning it.
        /// </summary>
        private unsafe void ReplaceNumericStructParam(
            SqlDecimal value,
            byte    precision,
            byte    scale,
            void    **paramValue)
        {
            SqlNumericStruct numericStruct = FromSqlDecimal(value, precision, scale);
            
            // Box into array for heap allocation (stack-allocated struct destroyed at method exit)
            // Pin to prevent GC from moving memory while native code holds the pointer
            //
            SqlNumericStruct[] valueArray = new SqlNumericStruct[1] { numericStruct };
            GCHandle handle = GCHandle.Alloc(valueArray, GCHandleType.Pinned);
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
                // For empty/null strings, allocate a single null byte
                //
                byte[] emptyBytes = new byte[1];
                GCHandle handle = GCHandle.Alloc(emptyBytes, GCHandleType.Pinned);
                _handleList.Add(handle);
                *paramValue = (void*)handle.AddrOfPinnedObject();
            }
            else
            {
                byte[] strBytes = Encoding.UTF8.GetBytes(value);
                GCHandle handle = GCHandle.Alloc(strBytes, GCHandleType.Pinned);
                _handleList.Add(handle);
                *paramValue = (void*)handle.AddrOfPinnedObject();
            }
        }

        /// <summary>
        /// This method replaces parameter value for Unicode string data types.
        /// If the string is not empty, the address of underlying Unicode bytes will be assigned to paramValue.
        /// </summary>
        private unsafe void ReplaceUnicodeStringParam(
            string value,
            void   **paramValue
        )
        {
            if(string.IsNullOrEmpty(value))
            {
                // For empty/null strings, allocate a single null wchar
                //
                byte[] emptyBytes = new byte[2];
                GCHandle handle = GCHandle.Alloc(emptyBytes, GCHandleType.Pinned);
                _handleList.Add(handle);
                *paramValue = (void*)handle.AddrOfPinnedObject();
            }
            else
            {
                byte[] strBytes = Encoding.Unicode.GetBytes(value);
                GCHandle handle = GCHandle.Alloc(strBytes, GCHandleType.Pinned);
                _handleList.Add(handle);
                *paramValue = (void*)handle.AddrOfPinnedObject();
            }
        }
    }
}
