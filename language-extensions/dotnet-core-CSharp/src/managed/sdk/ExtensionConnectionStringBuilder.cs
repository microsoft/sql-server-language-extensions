//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: ExtensionConnectionStringBuilder.cs
//
// Purpose:
//  Public SDK helper that builds an ODBC connection string for the
//  local SQL Server loopback endpoint. A single entry point,
//  BuildLoopbackConnectionString, hides the platform-detection and
//  driver / auth composition so extension user code does not need to
//  reason about whether it is executing on XDB (SQL DB) or on
//  classic on-prem SQL Server.
//
//*********************************************************************
using System;

namespace Microsoft.SqlServer.CSharpExtension.SDK
{
    /// <summary>
    /// Builds ODBC connection strings for the local SQL Server loopback
    /// endpoint published to the external extension host process
    /// (exthost.exe).
    /// <para>
    /// The <see cref="BuildLoopbackConnectionString"/> entry point
    /// builds either an implied-auth or SQL-authenticated connection
    /// string. When a user name is supplied, SQL authentication is
    /// used; otherwise, implied authentication is used.
    /// </para>
    /// <para>
    /// The method selects between the
    /// XDB (SQL DB) and classic on-prem loopback shapes based
    /// on the <c>IS_XDB</c> environment variable that launchpad sets
    /// on every XDB satellite process, so callers do not need to inspect
    /// the environment themselves.
    /// </para>
    /// </summary>
    public static class ExtensionConnectionStringBuilder
    {
        /// <summary>
        /// Environment variable set to <c>"TRUE"</c> by launchpad on
        /// every XDB satellite process. Authoritative signal
        /// used to distinguish XDB (SQL DB) from classic
        /// on-prem SQL Server. Absent on on-prem.
        /// </summary>
        private const string IsXdbEnvVar = "IS_XDB";

        /// <summary>
        /// Environment variable holding the TDS loopback endpoint in
        /// "host,port" form.
        /// </summary>
        private const string LoopbackEndpointEnvVar = "SqlTdsLoopbackConnectionEndpoint";

        /// <summary>
        /// Environment variable holding the SNI loopback named-pipe
        /// name (with or without an <c>np:</c> prefix). Set by
        /// launchpad on XDB. Read to build the cert-based implied-auth
        /// XDB loopback.
        /// </summary>
        private const string LoopbackPipeEnvVar = "LoopbackConnectionPipe";

        /// <summary>
        /// Environment variable holding the SHA-1 thumbprint of the
        /// extensibility client certificate the loopback should
        /// present. Set by launchpad on XDB.
        /// </summary>
        private const string CertificateHashEnvVar = "ExtensibilityCertificateHash";

        /// <summary>
        /// Environment variable holding the name of the physical
        /// database the caller is executing against. Read as the
        /// default when no <c>dbName</c> is passed. Set by launchpad.
        /// </summary>
        private const string PhysicalDbNameEnvVar = "PhysicalDbName";

        /// <summary>
        /// Fallback database name used when no <c>dbName</c> is passed
        /// and <see cref="PhysicalDbNameEnvVar"/> is not set.
        /// </summary>
        private const string DefaultDatabaseName = "master";

        /// <summary>
        /// ODBC driver clause used for every loopback connection
        /// string.
        /// </summary>
        private const string DriverClause = "Driver={ODBC Driver 18 for SQL Server};";

        /// <summary>
        /// ODBC option that suppresses server-certificate validation.
        /// </summary>
        private const string TrustServerCertificateClause = "TrustServerCertificate=Yes;";

        /// <summary>
        /// Named-pipe prefix required by SNI to select the Named Pipes
        /// provider without auto-detection.
        /// </summary>
        private const string NamedPipePrefix = "np:";

        /// <summary>
        /// Application name recorded in the XDB implied-auth
        /// connection string. Lets server-side traces attribute
        /// loopback sessions built by this SDK helper.
        /// </summary>
        private const string DefaultApplicationName = "CSharpExtension";

        /// <summary>
        /// Characters that can alter ODBC connection-string parsing
        /// when inserted into an unquoted value.
        /// </summary>
        private static readonly char[] s_invalidConnectionStringValueCharacters =
        {
            ';',
            '{',
            '}',
            '=',
        };

        /// <summary>
        /// Returns <c>true</c> when the current process is an XDB
        /// (SQL DB); <c>false</c> for classic on-prem SQL Server.
        /// Determined from the <c>IS_XDB</c> environment variable.
        /// </summary>
        private static bool IsXdbHost()
        {
            string value = Environment.GetEnvironmentVariable(IsXdbEnvVar);
            return string.Equals(value, "TRUE", StringComparison.OrdinalIgnoreCase);
        }

        /// <summary>
        /// Builds a loopback ODBC connection string for the local SQL
        /// Server. When <paramref name="userName"/> is supplied, the
        /// connection uses SQL authentication; otherwise, it uses
        /// implied authentication as the original caller.
        /// The transport is selected between XDB and on-prem from the
        /// <c>IS_XDB</c> environment variable.
        /// </summary>
        /// <param name="dbName">
        /// Target database name. When null or empty the value of the
        /// <see cref="PhysicalDbNameEnvVar"/> environment variable is
        /// used; if that is also not set the connection string targets
        /// <see cref="DefaultDatabaseName"/>.
        /// </param>
        /// <param name="userName">
        /// Optional SQL-authentication user name. When null or empty,
        /// implied authentication is used.
        /// </param>
        /// <param name="password">
        /// SQL-authentication password. Required when
        /// <paramref name="userName"/> is supplied.
        /// </param>
        /// <returns>ODBC connection string for the loopback endpoint.</returns>
        /// <exception cref="ArgumentException">
        /// Thrown when <paramref name="userName"/> is supplied without
        /// a non-empty <paramref name="password"/>, or when a supplied
        /// database name, user name, or password contains an ODBC
        /// connection-string metacharacter.
        /// </exception>
        /// <exception cref="InvalidOperationException">
        /// Thrown when <c>IS_XDB=TRUE</c> but launchpad has not published
        /// valid environment variables required by the selected
        /// authentication mode, or when the launchpad-provided database
        /// name is not valid for an ODBC connection string.
        /// </exception>
        public static string BuildLoopbackConnectionString(
            string dbName = null,
            string userName = null,
            string password = null)
        {
            string targetDb = ResolveDatabaseName(dbName);

            if (!string.IsNullOrEmpty(userName))
            {
                if (string.IsNullOrEmpty(password))
                {
                    throw new ArgumentException(
                        "SQL authentication requires a non-empty password.",
                        nameof(password));
                }

                ValidateConnectionStringArgument(userName, nameof(userName));
                ValidateConnectionStringArgument(password, nameof(password));

                if (IsXdbHost())
                {
                    string endpoint = RequireEnvironmentVariable(LoopbackEndpointEnvVar);
                    return
                        $"{DriverClause}Server={endpoint};Database={targetDb};"
                        + $"UID={userName};PWD={password};";
                }

                return
                    $"{DriverClause}{TrustServerCertificateClause}"
                    + $"Server=localhost;Database={targetDb};"
                    + $"UID={userName};PWD={password};";
            }

            if (IsXdbHost())
            {
                string pipe = RequireEnvironmentVariable(LoopbackPipeEnvVar);
                string thumbprint = RequireEnvironmentVariable(CertificateHashEnvVar);
                return BuildXdbImpliedAuthConnectionString(pipe, thumbprint, targetDb);
            }

            return BuildOnPremImpliedAuthConnectionString(targetDb);
        }

        /// <summary>
        /// Builds the XDB (SQL DB) cert-based implied-auth loopback
        /// connection string.
        /// </summary>
        /// <param name="pipe">SNI loopback pipe name from launchpad.</param>
        /// <param name="thumbprint">
        /// Extensibility client certificate SHA-1 hash from launchpad.
        /// </param>
        /// <param name="targetDb">Resolved target database name.</param>
        /// <returns>ODBC connection string for the XDB loopback.</returns>
        private static string BuildXdbImpliedAuthConnectionString(
            string pipe, string thumbprint, string targetDb)
        {
            // Prefix server with "np:" so SNI selects the Named Pipes
            // provider without auto-detection. Preserve any prefix
            // already emitted by launchpad.
            string server = pipe;
            if (!server.StartsWith(NamedPipePrefix, StringComparison.OrdinalIgnoreCase))
            {
                server = NamedPipePrefix + server;
            }

            return
                $"{DriverClause}Server={server};Database={targetDb};"
                + $"Encrypt=yes;{TrustServerCertificateClause}"
                + $"ClientCertificate=sha1:{thumbprint};"
                + $"Connection Timeout=10;ConnectRetryCount=0;APP={DefaultApplicationName}";
        }

        /// <summary>
        /// Builds the on-prem trusted (Windows) loopback connection
        /// string to <c>localhost</c>.
        /// </summary>
        /// <param name="targetDb">Resolved target database name.</param>
        /// <returns>ODBC connection string for the on-prem loopback.</returns>
        private static string BuildOnPremImpliedAuthConnectionString(string targetDb)
        {
            return
                $"{DriverClause}{TrustServerCertificateClause}"
                + $"Server=localhost;Database={targetDb};Trusted_Connection=Yes;";
        }

        /// <summary>
        /// Returns the value to use for the ODBC <c>Database=</c>
        /// keyword. Prefers the caller-supplied
        /// <paramref name="dbName"/>, falls back to
        /// <see cref="PhysicalDbNameEnvVar"/>, and finally to
        /// <see cref="DefaultDatabaseName"/>.
        /// </summary>
        /// <param name="dbName">Caller-supplied database name.</param>
        /// <returns>Database name to embed in the connection string.</returns>
        private static string ResolveDatabaseName(string dbName)
        {
            if (!string.IsNullOrEmpty(dbName))
            {
                ValidateConnectionStringArgument(dbName, nameof(dbName));
                return dbName;
            }

            string fromEnv = Environment.GetEnvironmentVariable(PhysicalDbNameEnvVar);
            if (!string.IsNullOrEmpty(fromEnv))
            {
                return ValidateEnvironmentVariableValue(PhysicalDbNameEnvVar, fromEnv);
            }

            return DefaultDatabaseName;
        }

        /// <summary>
        /// Reads a required environment variable and throws
        /// <see cref="InvalidOperationException"/> when it is missing
        /// empty, or contains an ODBC connection-string metacharacter.
        /// Used for XDB loopback inputs whose absence or malformed value
        /// indicates a launchpad misconfiguration that must not be
        /// silently masked by falling back to on-prem.
        /// </summary>
        /// <param name="name">Environment variable name.</param>
        /// <returns>Non-empty value of the environment variable.</returns>
        private static string RequireEnvironmentVariable(string name)
        {
            string value = Environment.GetEnvironmentVariable(name);
            if (string.IsNullOrEmpty(value))
            {
                throw new InvalidOperationException(
                    $"XDB satellite is missing required environment variable '{name}'. "
                    + "This variable is normally published by launchpad; "
                    + "its absence indicates a launchpad misconfiguration.");
            }

            return ValidateEnvironmentVariableValue(name, value);
        }

        /// <summary>
        /// Rejects a caller-supplied value that could append or alter an
        /// ODBC connection-string keyword.
        /// </summary>
        /// <param name="value">Value to validate.</param>
        /// <param name="parameterName">Public API parameter name.</param>
        private static void ValidateConnectionStringArgument(string value, string parameterName)
        {
            if (ContainsConnectionStringMetacharacter(value))
            {
                throw new ArgumentException(
                    "The value contains a character that is not allowed in an ODBC connection string.",
                    parameterName);
            }
        }

        /// <summary>
        /// Rejects a launchpad-provided value that could append or alter
        /// an ODBC connection-string keyword.
        /// </summary>
        /// <param name="name">Environment variable name.</param>
        /// <param name="value">Environment variable value.</param>
        /// <returns>The validated value.</returns>
        private static string ValidateEnvironmentVariableValue(string name, string value)
        {
            if (ContainsConnectionStringMetacharacter(value))
            {
                throw new InvalidOperationException(
                    $"Environment variable '{name}' contains a character that is not allowed "
                    + "in an ODBC connection string.");
            }

            return value;
        }

        /// <summary>
        /// Determines whether a value contains an ODBC connection-string
        /// metacharacter.
        /// </summary>
        /// <param name="value">Value to inspect.</param>
        /// <returns><c>true</c> when the value contains a metacharacter.</returns>
        private static bool ContainsConnectionStringMetacharacter(string value)
        {
            return value.IndexOfAny(s_invalidConnectionStringValueCharacters) >= 0;
        }
    }
}
