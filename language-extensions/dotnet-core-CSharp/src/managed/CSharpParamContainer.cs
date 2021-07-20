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
        private Dictionary<ushort, CSharpParam> _params = new Dictionary<ushort, CSharpParam>();

        /// <summary>
        /// This constructor initializes the UserParams with an empty dictionary and stores TotalParams.
        /// </summary>
        public CSharpParamContainer(ushort parametersNumber)
        {
            TotalParams = parametersNumber;
            UserParams = new Dictionary<string, dynamic>();
        }

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
        /// This method adds a CSharpParam to the container
        /// and adds to user parameter dictionary as well.
        /// </summary>
        public unsafe void AddParam(
            ushort paramNumber,
            string paramName,
            SqlDataType  dataType,
            ulong  paramSize,
            short  decimalDigits,
            void   *paramValue,
            int    strLenOrNullMap,
            short  inputOutputType)
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
                default:
                    throw new NotImplementedException("Parameter type for " + dataType.ToString() + " has not been implemented yet");
            }

            UserParams[paramName] = _params[paramNumber].Value;
        }
    }
}
