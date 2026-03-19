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
                    // Convert SQL_NUMERIC_STRUCT to SqlDecimal
                    // Special handling for OUTPUT parameters: if precision=0, treat as uninitialized
                    SqlNumericStruct* numericPtr = (SqlNumericStruct*)paramValue;
                    if (numericPtr->precision == 0)
                    {
                        // OUTPUT parameter with uninitialized struct - use SqlDecimal.Null
                        // The C# executor will set the actual value
                        _params[paramNumber].Value = SqlDecimal.Null;
                    }
                    else
                    {
                        // INPUT or INPUT_OUTPUT parameter with valid value
                        _params[paramNumber].Value = ToSqlDecimal(*numericPtr);
                    }
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
            if(ReferenceEquals(param.Value, null))
            {
                *strLenOrNullMap = SQL_NULL_DATA;
                return;
            }
            
            // Special handling for SqlDecimal.Null (SqlDecimal is a struct, not a class)
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
                    // Convert SqlDecimal to SQL_NUMERIC_STRUCT
                    // Use the precision and scale from the parameter metadata
                    // Note: param.Value could be SqlDecimal or potentially null (handled above)
                    if (param.Value is SqlDecimal sqlDecimalValue)
                    {
                        // WHY use param.Size for precision?
                        // - For DECIMAL/NUMERIC parameters, param.Size contains the declared precision (not bytes)
                        // - This follows standard ODBC behavior where ColumnSize = precision for SQL_NUMERIC/SQL_DECIMAL
                        // - CRITICAL: The SqlNumericStruct precision MUST match the declared parameter precision
                        //   or SQL Server rejects it with "Invalid data for type decimal" (Msg 9803)
                        // - Example: DECIMAL(3,3) parameter MUST have precision=3 in the struct, not precision=38
                        byte precision = (byte)param.Size;
                        byte scale = (byte)param.DecimalDigits;
                        // WHY set strLenOrNullMap to 19?
                        // - For fixed-size types like SQL_NUMERIC_STRUCT, strLenOrNullMap contains the byte size
                        // - SQL_NUMERIC_STRUCT is exactly 19 bytes: precision(1) + scale(1) + sign(1) + val(16)
                        // - This tells ODBC how many bytes to read from the paramValue pointer
                        *strLenOrNullMap = 19; // sizeof(SqlNumericStruct)
                        ReplaceNumericStructParam(sqlDecimalValue, precision, scale, paramValue);
                    }
                    else
                    {
                        throw new InvalidCastException($"Expected SqlDecimal for NUMERIC parameter, got {param.Value?.GetType().Name ?? "null"}");
                    }
                    break;
                case SqlDataType.DotNetChar:
                    // For CHAR/VARCHAR, strLenOrNullMap is in bytes (1 byte per character for ANSI).
                    // param.Size is the declared parameter size in characters (from SQL Server's CHAR(n)/VARCHAR(n)).
                    // For ANSI strings, character count equals byte count.
                    //
                    int charByteLen = param.Value.Length;
                    int charMaxByteLen = (int)param.Size;
                    *strLenOrNullMap = (charByteLen < charMaxByteLen) ? charByteLen : charMaxByteLen;
                    ReplaceStringParam((string)param.Value, paramValue);
                    break;
                case SqlDataType.DotNetWChar:
                    // For NCHAR/NVARCHAR, strLenOrNullMap must be in bytes (UTF-16: 2 bytes per character).
                    // In C#, sizeof(char) is always 2 bytes (UTF-16), regardless of platform.
                    // Note: C++ wchar_t is 2 bytes on Windows but 4 bytes on Linux - this extension only supports Windows.
                    // param.Size is the declared parameter size in characters (from SQL Server's NCHAR(n)/NVARCHAR(n)),
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
        /// This method replaces parameter value for NUMERIC/DECIMAL data types.
        /// Converts SqlDecimal to SQL_NUMERIC_STRUCT and uses proper memory pinning.
        /// Follows the same pattern as Java extension's numeric parameter handling.
        /// </summary>
        /// <param name="value">The SqlDecimal value to convert.</param>
        /// <param name="precision">Total number of digits (1-38).</param>
        /// <param name="scale">Number of digits after decimal point (0-precision).</param>
        /// <param name="paramValue">Output pointer to receive the pinned SqlNumericStruct.</param>
        private unsafe void ReplaceNumericStructParam(
            SqlDecimal value,
            byte    precision,
            byte    scale,
            void    **paramValue)
        {
            // Convert SqlDecimal to SQL_NUMERIC_STRUCT
            SqlNumericStruct numericStruct = FromSqlDecimal(value, precision, scale);
            
            // Box the struct into a single-element array to create a heap-allocated copy, then pin it.
            // 
            // WHY box into an array before pinning?
            // - Local struct 'numericStruct' is stack-allocated and will be destroyed when method returns
            // - We need a heap-allocated copy that survives after this method returns
            // - GCHandle.Alloc requires a heap object; structs must be boxed first
            // - Single-element array is the simplest way to create a heap-allocated struct
            //
            // WHY pin with GCHandle?
            // - Native code will dereference the paramValue pointer during execution
            // - Without pinning, garbage collector could move the object, invalidating the pointer
            // - GCHandleType.Pinned prevents GC from moving the object until we free the handle
            //
            // WHY add handle to _handleList?
            // - If we don't keep a reference, GC could free the handle immediately
            // - _handleList keeps handles alive until container is disposed/reset
            // - Handles are freed in ResetParams or class disposal, ensuring proper cleanup
            //
            SqlNumericStruct[] valueArray = new SqlNumericStruct[1] { numericStruct };
            GCHandle handle = GCHandle.Alloc(valueArray, GCHandleType.Pinned);
            _handleList.Add(handle);
            *paramValue = (void*)handle.AddrOfPinnedObject();
            
            Logging.Trace($"ReplaceNumericStructParam: Converted SqlDecimal {value} to SqlNumericStruct (precision={precision}, scale={scale})");
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
