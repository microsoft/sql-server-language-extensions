//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: CSharpTestExecutor.cs
//
// Purpose:
//  Test classes extending the AbstractSqlServerExtensionExecutor.
//
//*********************************************************************
using System;
using System.Data.SqlTypes;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using Microsoft.Data.Analysis;
using Microsoft.SqlServer.CSharpExtension.SDK;
using static Microsoft.SqlServer.CSharpExtension.Sql;

namespace Microsoft.SqlServer.CSharpExtensionTest
{
    /// <summary>
    /// Shared constants for the test executors.
    /// </summary>
    internal static class CSharpTestExecutorConstants
    {
        /// <summary>
        /// Marker line the native test harness asserts on (see CSharpExecuteTests.cpp).
        /// Kept in one place so executors can't drift or typo it.
        /// </summary>
        public const string HelloMessage = "Hello .NET Core CSharpExtension!";

        /// <summary>
        /// Marker message emitted through the SDK ExtensionEventLogger by
        /// CSharpTestExecutorLogInformation. The native session-tagging test
        /// (CSharpExecuteTests.cpp) matches the forwarded XEvent on this text,
        /// so both sides must stay in sync.
        /// </summary>
        public const string LogEventMessage = "CSharpTestExecutorLogInformation emitted event";

        /// <summary>
        /// Marker message used to verify that a caller-supplied extension name flows
        /// end to end through the XEvent callback.
        /// </summary>
        public const string LogNamedEventMessage = "CSharpTestExecutorLogNamedExtension emitted event";

        /// <summary>
        /// Extension name used to verify caller-supplied attribution survives the
        /// managed-to-native-to-host callback path.
        /// </summary>
        public const string LogEventExtensionName = "TestExtension";
    }

    public class CSharpTestExecutor: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            Console.WriteLine(CSharpTestExecutorConstants.HelloMessage);
            return input;
        }
    }

    /// <summary>
    /// Test executor that emits an event through the public SDK ExtensionEventLogger
    /// facade from inside Execute. Exercises the AsyncLocal session tagging in
    /// Logging/CSharpSession.Execute so the native test can assert the forwarded
    /// XEvent carries the executing session's ID and task ID.
    /// </summary>
    public class CSharpTestExecutorLogInformation: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            ExtensionEventLogger.LogInformation(CSharpTestExecutorConstants.LogEventMessage);
            return input;
        }
    }

    /// <summary>
    /// Test executor that emits an event through the ExtensionEventLogger extensionName
    /// overload, attributing it to a named extension. Lets the native test assert the
    /// forwarded XEvent carries the caller-supplied extension name rather than the default.
    /// </summary>
    public class CSharpTestExecutorLogNamedExtension: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            ExtensionEventLogger.Log(
                ExtensionTraceLevel.Information,
                CSharpTestExecutorConstants.LogNamedEventMessage,
                errorCode: 0,
                extensionName: CSharpTestExecutorConstants.LogEventExtensionName);
            return input;
        }
    }

    /// <summary>
    /// Test executor that builds loopback connection strings under on-prem and XDB
    /// environment configurations. Results and expected failure messages are returned
    /// through output parameters for the native test harness to verify.
    /// </summary>
    public class CSharpTestExecutorConnectionStringBuilder: AbstractSqlServerExtensionExecutor
    {
        private const string IsXdbEnvVar = "IS_XDB";
        private const string IsWcowEnvVar = "IS_WCOW";
        private const string LoopbackEndpointEnvVar = "SqlTdsLoopbackConnectionEndpoint";
        private const string LoopbackPipeEnvVar = "LoopbackConnectionPipe";
        private const string CertificateHashEnvVar = "ExtensibilityCertificateHash";
        private const string PhysicalDbNameEnvVar = "PhysicalDbName";

        private static readonly string[] s_environmentVariableNames =
        {
            IsXdbEnvVar,
            IsWcowEnvVar,
            LoopbackEndpointEnvVar,
            LoopbackPipeEnvVar,
            CertificateHashEnvVar,
            PhysicalDbNameEnvVar,
        };

        /// <summary>
        /// Builds ODBC loopback connection strings under representative box,
        /// SQL DB, and invalid host configurations. Each result or captured
        /// exception detail is returned through a descriptively named output
        /// parameter for the native test harness.
        /// </summary>
        /// <param name="input">Input data frame supplied by the test harness.</param>
        /// <param name="sqlParams">
        /// Input/output parameter dictionary used to return test observations.
        /// </param>
        /// <returns>The unmodified input data frame.</returns>
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams)
        {
            var originalEnvironment = new Dictionary<string, string>();
            foreach (string name in s_environmentVariableNames)
            {
                originalEnvironment.Add(name, Environment.GetEnvironmentVariable(name));
                Environment.SetEnvironmentVariable(name, null);
            }

            try
            {
                sqlParams["@onPremImpliedAuth"] =
                    ExtensionConnectionStringBuilder.BuildOdbcLoopbackConnectionString("TestDb");

                Environment.SetEnvironmentVariable(
                    IsXdbEnvVar,
                    "TRUE");
                Environment.SetEnvironmentVariable(
                    LoopbackPipeEnvVar,
                    "loopback-pipe");
                Environment.SetEnvironmentVariable(
                    CertificateHashEnvVar,
                    "01 23 45 67 89 AB CD EF");
                sqlParams["@xdbImpliedAuth"] =
                    ExtensionConnectionStringBuilder.BuildOdbcLoopbackConnectionString("TestDb");

                Environment.SetEnvironmentVariable(
                    LoopbackPipeEnvVar,
                    null);

                // Expected message: LoopbackConnectionPipe is missing. XDB implied
                // authentication requires the launchpad-provided named-pipe endpoint.
                sqlParams["@missingXdbPipeError"] = CaptureInvalidOperationMessage(
                    () => ExtensionConnectionStringBuilder.BuildOdbcLoopbackConnectionString());

                Environment.SetEnvironmentVariable(
                    LoopbackPipeEnvVar,
                    "loopback-pipe");
                Environment.SetEnvironmentVariable(
                    CertificateHashEnvVar,
                    null);

                // Expected message: ExtensibilityCertificateHash is missing. XDB implied
                // authentication requires the launchpad-provided client certificate hash.
                sqlParams["@missingXdbCertificateError"] = CaptureInvalidOperationMessage(
                    () => ExtensionConnectionStringBuilder.BuildOdbcLoopbackConnectionString());

                Environment.SetEnvironmentVariable(
                    IsXdbEnvVar,
                    null);
                Environment.SetEnvironmentVariable(
                    PhysicalDbNameEnvVar,
                    "PhysicalDb");
                sqlParams["@physicalDbFallback"] =
                    ExtensionConnectionStringBuilder.BuildOdbcLoopbackConnectionString(string.Empty);

                Environment.SetEnvironmentVariable(
                    PhysicalDbNameEnvVar,
                    null);
                sqlParams["@masterDbFallback"] =
                    ExtensionConnectionStringBuilder.BuildOdbcLoopbackConnectionString();

                sqlParams["@onPremSqlAuth"] =
                    ExtensionConnectionStringBuilder.BuildOdbcLoopbackConnectionString(
                        "TestDb",
                        "TestUser",
                        "TestPassword");

                Environment.SetEnvironmentVariable(
                    IsXdbEnvVar,
                    "TRUE");
                Environment.SetEnvironmentVariable(
                    LoopbackEndpointEnvVar,
                    "localhost,1433");
                sqlParams["@xdbSqlAuth"] =
                    ExtensionConnectionStringBuilder.BuildOdbcLoopbackConnectionString(
                        "TestDb",
                        "TestUser",
                        "TestPassword");

                // Expected parameter: password. SQL authentication requires a non-empty
                // password whenever a user name is supplied, so omitting it must fail.
                sqlParams["@missingPasswordParamName"] = CaptureArgumentExceptionParameterName(
                    () => ExtensionConnectionStringBuilder.BuildOdbcLoopbackConnectionString(
                        "TestDb",
                        "TestUser"));

                // Expected parameter: userName. A password without a user name is
                // incomplete SQL-auth configuration and must not select implied auth.
                sqlParams["@missingUserNameParamName"] = CaptureArgumentExceptionParameterName(
                    () => ExtensionConnectionStringBuilder.BuildOdbcLoopbackConnectionString(
                        "TestDb",
                        password: "TestPassword"));

                sqlParams["@emptyUserNameParamName"] = CaptureArgumentExceptionParameterName(
                    () => ExtensionConnectionStringBuilder.BuildOdbcLoopbackConnectionString(
                        "TestDb",
                        string.Empty,
                        "TestPassword"));

                Environment.SetEnvironmentVariable(IsXdbEnvVar, null);
                sqlParams["@escapedDbName"] =
                    ExtensionConnectionStringBuilder.BuildOdbcLoopbackConnectionString(
                        "TestDb;Encrypt=no");

                sqlParams["@escapedCredentials"] =
                    ExtensionConnectionStringBuilder.BuildOdbcLoopbackConnectionString(
                        "TestDb",
                        "TestUser=Injected",
                        "{Test;Password}");

                Environment.SetEnvironmentVariable(
                    PhysicalDbNameEnvVar,
                    "PhysicalDb;UID=Injected");
                sqlParams["@escapedPhysicalDb"] =
                    ExtensionConnectionStringBuilder.BuildOdbcLoopbackConnectionString();

                Environment.SetEnvironmentVariable(PhysicalDbNameEnvVar, null);
                Environment.SetEnvironmentVariable(IsXdbEnvVar, "TRUE");
                Environment.SetEnvironmentVariable(LoopbackPipeEnvVar, "loopback-pipe");
                Environment.SetEnvironmentVariable(CertificateHashEnvVar, "01 23 45 67 89 AB CD EF");
                sqlParams["@customApplicationName"] =
                    ExtensionConnectionStringBuilder.BuildOdbcLoopbackConnectionString(
                        "TestDb",
                        applicationName: "OtherExtension;UID=Injected");

                Environment.SetEnvironmentVariable(IsWcowEnvVar, "TRUE");
                sqlParams["@unsupportedWcowError"] = CaptureInvalidOperationMessage(
                    () => ExtensionConnectionStringBuilder.BuildOdbcLoopbackConnectionString());

                Environment.SetEnvironmentVariable(IsWcowEnvVar, null);
                Environment.SetEnvironmentVariable(IsXdbEnvVar, "FALSE");
                sqlParams["@invalidXdbMarkerError"] = CaptureInvalidOperationMessage(
                    () => ExtensionConnectionStringBuilder.BuildOdbcLoopbackConnectionString());

                Environment.SetEnvironmentVariable(IsXdbEnvVar, "TRUE");
                Environment.SetEnvironmentVariable(
                    LoopbackEndpointEnvVar,
                    "localhost,1433;Encrypt=no");
                sqlParams["@invalidXdbEndpointError"] = CaptureInvalidOperationMessage(
                    () => ExtensionConnectionStringBuilder.BuildOdbcLoopbackConnectionString(
                        "TestDb",
                        "TestUser",
                        "TestPassword"));
            }
            finally
            {
                foreach (KeyValuePair<string, string> variable in originalEnvironment)
                {
                    Environment.SetEnvironmentVariable(variable.Key, variable.Value);
                }
            }

            return input;
        }

        private static string CaptureInvalidOperationMessage(Action action)
        {
            try
            {
                action();
            }
            catch (InvalidOperationException exception)
            {
                return exception.Message;
            }

            return string.Empty;
        }

        private static string CaptureArgumentExceptionParameterName(Action action)
        {
            try
            {
                action();
            }
            catch (ArgumentException exception)
            {
                return exception.ParamName;
            }

            return string.Empty;
        }
    }

    public class CSharpTestExecutorIntParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            sqlParams["@param0"] = Int32.MaxValue;
            sqlParams["@param1"] = Int32.MinValue;
            sqlParams["@param2"] = 4;
            sqlParams["@param3"] = 0;
            sqlParams["@param4"] = null;
            return null;
        }
    }

    public class CSharpTestExecutorBitParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            sqlParams["@param0"] = true;
            sqlParams["@param1"] = false;
            sqlParams["@param2"] = 3;
            sqlParams["@param3"] = 0;
            sqlParams["@param4"] = null;
            return null;
        }
    }

    public class CSharpTestExecutorRealParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            sqlParams["@param0"] = 3.4e38F;
            sqlParams["@param1"] = -3.4e38F;
            sqlParams["@param2"] = 2.3e4;
            sqlParams["@param3"] = 0;
            sqlParams["@param4"] = null;
            return null;
        }
    }

    public class CSharpTestExecutorDoubleParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            sqlParams["@param0"] = 1.79e308;
            sqlParams["@param1"] = -1.79e308;
            sqlParams["@param2"] = 1.45e38;
            sqlParams["@param3"] = 0;
            sqlParams["@param4"] = null;
            return null;
        }
    }

    public class CSharpTestExecutorBigIntParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            sqlParams["@param0"] = Int64.MaxValue;
            sqlParams["@param1"] = Int64.MinValue;
            sqlParams["@param2"] = 9372036854775;
            sqlParams["@param3"] = 0;
            sqlParams["@param4"] = null;
            return null;
        }
    }

    public class CSharpTestExecutorSmallIntParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            sqlParams["@param0"] = Int16.MaxValue;
            sqlParams["@param1"] = Int16.MinValue;
            sqlParams["@param2"] = 3007;
            sqlParams["@param3"] = 0;
            sqlParams["@param4"] = null;
            return null;
        }
    }

    public class CSharpTestExecutorTinyIntParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            sqlParams["@param0"] = 255;
            sqlParams["@param1"] = 0;
            sqlParams["@param2"] = 123;
            sqlParams["@param3"] = -1;
            sqlParams["@param4"] = null;
            return null;
        }
    }

    /// <summary>
    /// Comprehensive test executor for DECIMAL/NUMERIC OUTPUT parameters.
    /// Covers: max/min values, high precision/scale, financial values, zero, nulls.
    /// Consolidated from CSharpTestExecutorDecimalParam + CSharpTestExecutorDecimalHighScaleParam.
    /// </summary>
    public class CSharpTestExecutorDecimalParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams)
        {
            // Maximum value: DECIMAL(38,0) max = 10^38 - 1
            sqlParams["@param0"] = SqlDecimal.Parse("99999999999999999999999999999999999999");
            
            // Minimum value (negative max)
            sqlParams["@param1"] = SqlDecimal.Parse("-99999999999999999999999999999999999999");
            
            // High scale: DECIMAL(38,10) - 38 digits with 10 fractional
            sqlParams["@param2"] = SqlDecimal.Parse("1234567890123456789012345678.1234567890");
            
            // Zero
            sqlParams["@param3"] = new SqlDecimal(0);
            
            // High fractional precision: DECIMAL(38,28) - 10 integer + 28 fractional
            sqlParams["@param4"] = SqlDecimal.Parse("1234567890.1234567890123456789012345678");
            
            // Typical financial: DECIMAL(19,4)
            sqlParams["@param5"] = SqlDecimal.Parse("123456789012345.6789");
            
            // Negative financial
            sqlParams["@param6"] = SqlDecimal.Parse("-123456789012345.6789");
            
            // Null
            sqlParams["@param7"] = null;
            
            return null;
        }
    }

    public class CSharpTestExecutorStringParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            sqlParams["@param0"] = "HELLO";
            sqlParams["@param1"] = "C#Extension";
            sqlParams["@param2"] = "";
            sqlParams["@param3"] = "WORLD";
            sqlParams["@param4"] = null;
            sqlParams["@param5"] = null;
            return null;
        }
    }

    /// <summary>
    /// Test executor that returns a DataFrame with mixed-scale SqlDecimal values in the same column.
    /// This exercises the precision clamping fix in ExtractNumericColumn: when maxIntDigits + maxScale > 38,
    /// scale is reduced to (38 - maxIntDigits) to preserve integer digits.
    /// </summary>
    public class CSharpTestExecutorMixedScaleDecimalOutput : AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams)
        {
            // Create output DataFrame with one SqlDecimal column containing mixed scales
            //
            var column = new PrimitiveDataFrameColumn<SqlDecimal>("MixedScaleCol", 3);
            column[0] = SqlDecimal.Parse("999999999999999999");                       // 18 int digits, scale=0
            column[1] = SqlDecimal.Parse("0.000000000000000000000000000001");           // 0 int digits, scale=30
            column[2] = SqlDecimal.Parse("42");                                        // 2 int digits, scale=0
            // maxIntDigits=18, maxScale=30, sum=48 > 38 → clamp: precision=38, scale=20

            return new DataFrame(column);
        }
    }

    public class CSharpTestExecutorWStringParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            sqlParams["@param0"] = "HELLO";
            sqlParams["@param1"] = "C#Extension";
            sqlParams["@param2"] = string.Empty;
            sqlParams["@param3"] = "WORLD";
            sqlParams["@param4"] = null;  // null NVARCHAR (variable-length)
            sqlParams["@param5"] = null;  // null NCHAR (fixed-length)
            return null;
        }
    }

    /// <summary>
    /// Test executor for NVARCHAR(MAX) scenarios with large strings.
    /// </summary>
    public class CSharpTestExecutorWStringMaxParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            // Generate a large string (10,000 characters) to test NVARCHAR(MAX) handling
            //
            sqlParams["@param0"] = new string('A', 10000);

            // Generate a string with repeating Unicode pattern (5,000 characters)
            //
            string unicodePattern = "你好世界€";  // 5 characters
            var sb = new System.Text.StringBuilder();
            for (int i = 0; i < 1000; i++)
            {
                sb.Append(unicodePattern);
            }
            sqlParams["@param1"] = sb.ToString();

            // Test null for NVARCHAR(MAX)
            //
            sqlParams["@param2"] = null;

            return null;
        }
    }

    /// <summary>
    /// Test executor for Unicode variety: emojis, accented characters, mixed scripts.
    /// </summary>
    public class CSharpTestExecutorWStringUnicodeParam: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            // Test emoji characters (surrogate pairs in UTF-16)
            //
            sqlParams["@param0"] = "Hi\U0001F600\U0001F44D";  // "Hi" + grinning face + thumbs up

            // Test accented/European characters
            //
            sqlParams["@param1"] = "Café résumé naïve";

            // Test mixed scripts (Latin, Chinese, Japanese, Korean)
            //
            sqlParams["@param2"] = "Hello世界こんにちは";

            // Test currency and special symbols
            //
            sqlParams["@param3"] = "€100 £50 ¥1000 ©®™";

            return null;
        }
    }

    /// <summary>
    /// Test executor demonstrating NVARCHAR output support for DataFrame columns.
    /// Uses StringOutputColumnTypes to specify that string columns should be NVARCHAR.
    /// </summary>
    public class CSharpTestExecutorNVarcharOutput: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            Console.WriteLine(CSharpTestExecutorConstants.HelloMessage);
            // Specify that output column "text" should be NVARCHAR (UTF-16)
            StringOutputColumnTypes["text"] = StringOutputType.NVarChar;
            
            // Return input unchanged - the column type will be NVARCHAR instead of VARCHAR
            return input;
        }
    }

    /// <summary>
    /// Test executor demonstrating mixed VARCHAR and NVARCHAR output columns.
    /// </summary>
    public class CSharpTestExecutorMixedStringOutput: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            Console.WriteLine(CSharpTestExecutorConstants.HelloMessage);
            // Column "ascii_col" stays VARCHAR (default, no need to specify)
            
            // Column "unicode_col" should be NVARCHAR (by name)
            StringOutputColumnTypes["unicode_col"] = StringOutputType.NVarChar;
            
            return input;
        }
    }

    /// <summary>
    /// Test executor for basic pass-through (no NVARCHAR configuration).
    /// </summary>
    public class CSharpTestExecutorPreserveInputTypes: AbstractSqlServerExtensionExecutor
    {
        public override DataFrame Execute(DataFrame input, Dictionary<string, dynamic> sqlParams){
            Console.WriteLine(CSharpTestExecutorConstants.HelloMessage);
            // No explicit StringOutputColumnTypes configuration.
            // All string columns will be VARCHAR (default).
            return input;
        }
    }
}
