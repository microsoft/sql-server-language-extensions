//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: ExtensionConnectionStringBuilder.cs
//
// Purpose:
//  Public SDK helper that builds an ODBC connection string for the
//  local SQL Server loopback endpoint. A single entry point,
//  BuildOdbcLoopbackConnectionString, hides the platform-detection and
//  driver / auth composition so extension user code does not need to
//  reason about whether it is executing on XDB (SQL DB) or on
//  classic on-prem SQL Server.
//
//*********************************************************************
using System;
using System.Data.Common;

namespace Microsoft.SqlServer.CSharpExtension.SDK
{
    /// <summary>
    /// Builds ODBC connection strings for the local SQL Server loopback
    /// endpoint published to the external extension host process
    /// (exthost.exe).
    /// <para>
    /// The <see cref="BuildOdbcLoopbackConnectionString"/> entry point
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
        /// Environment variable set to <c>"TRUE"</c> for SQL Managed
        /// Instance on Windows containers (WCOW). ODBC loopback is not
        /// supported on this host type.
        /// </summary>
        private const string IsWcowEnvVar = "IS_WCOW";

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
        /// Environment variable holding the hash of the extensibility
        /// client certificate the loopback should present. Set by
        /// launchpad on XDB.
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
        private static readonly char[] s_invalidEnvironmentValueCharacters =
        {
            ';',
            '{',
            '}',
            '=',
        };

        /// <summary>
        /// Returns <c>true</c> when the current process is SQL DB and
        /// <c>false</c> for classic on-prem SQL Server. SQL Managed
        /// Instance is rejected because ODBC loopback is unsupported.
        /// Any present host marker must have the value <c>TRUE</c>.
        /// </summary>
        private static bool IsXdbHost()
        {
            if (GetStrictEnvironmentFlag(IsWcowEnvVar))
            {
                throw new InvalidOperationException(
                    "SQL Managed Instance (WCOW) is not supported by ODBC loopback connections.");
            }

            return GetStrictEnvironmentFlag(IsXdbEnvVar);
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
        /// Optional SQL-authentication user name. Must be supplied together
        /// with <paramref name="password"/>. When both are null or empty,
        /// implied authentication is used.
        /// </param>
        /// <param name="password">
        /// Optional SQL-authentication password. Must be supplied together
        /// with <paramref name="userName"/>.
        /// </param>
        /// <param name="applicationName">
        /// Optional application name recorded for XDB implied-auth
        /// connections. When null or empty, <c>CSharpExtension</c> is used.
        /// </param>
        /// <returns>ODBC connection string for the loopback endpoint.</returns>
        /// <exception cref="ArgumentException">
        /// Thrown when exactly one of <paramref name="userName"/> and
        /// <paramref name="password"/> is supplied.
        /// </exception>
        /// <exception cref="InvalidOperationException">
        /// Thrown for SQL Managed Instance, an invalid host marker, or when
        /// an XDB satellite lacks a valid environment variable required by
        /// the selected authentication mode.
        /// </exception>
        public static string BuildOdbcLoopbackConnectionString(
            string dbName = null,
            string userName = null,
            string password = null,
            string applicationName = null)
        {
            string targetDb = ResolveDatabaseName(dbName);
            bool hasUserName = !string.IsNullOrEmpty(userName);
            bool hasPassword = !string.IsNullOrEmpty(password);

            if (hasUserName != hasPassword)
            {
                string missingParameterName = hasUserName ? nameof(password) : nameof(userName);
                throw new ArgumentException(
                    "SQL authentication requires a non-empty user name and password.",
                    missingParameterName);
            }

            string resolvedApplicationName = string.IsNullOrEmpty(applicationName)
                ? DefaultApplicationName
                : applicationName;

            if (hasUserName)
            {
                if (IsXdbHost())
                {
                    string endpoint = GetRequiredEnvironmentVariable(LoopbackEndpointEnvVar);
                    return BuildSqlAuthConnectionString(
                        endpoint,
                        targetDb,
                        userName,
                        password,
                        trustServerCertificate: false);
                }

                return BuildSqlAuthConnectionString(
                    "localhost",
                    targetDb,
                    userName,
                    password,
                    trustServerCertificate: true);
            }

            if (IsXdbHost())
            {
                string pipe = GetRequiredEnvironmentVariable(LoopbackPipeEnvVar);
                string certificateHash = GetRequiredEnvironmentVariable(CertificateHashEnvVar);
                return BuildXdbImpliedAuthConnectionString(
                    pipe,
                    certificateHash,
                    targetDb,
                    resolvedApplicationName);
            }

            return BuildOnPremImpliedAuthConnectionString(targetDb);
        }

        /// <summary>
        /// Builds the XDB (SQL DB) cert-based implied-auth loopback
        /// connection string.
        /// </summary>
        /// <param name="pipe">SNI loopback pipe name from launchpad.</param>
        /// <param name="certificateHash">
        /// Extensibility client certificate hash from launchpad.
        /// </param>
        /// <param name="targetDb">Resolved target database name.</param>
        /// <param name="applicationName">Application name recorded for the connection.</param>
        /// <returns>ODBC connection string for the XDB loopback.</returns>
        private static string BuildXdbImpliedAuthConnectionString(
            string pipe, string certificateHash, string targetDb, string applicationName)
        {
            // Prefix server with "np:" so SNI selects the Named Pipes
            // provider without auto-detection. Preserve any prefix
            // already emitted by launchpad.
            string server = pipe;
            if (!server.StartsWith(NamedPipePrefix, StringComparison.OrdinalIgnoreCase))
            {
                server = NamedPipePrefix + server;
            }

            DbConnectionStringBuilder builder = CreateConnectionStringBuilder(server, targetDb);
            builder["Encrypt"] = "yes";
            builder["TrustServerCertificate"] = "Yes";
            builder["ClientCertificate"] = "sha1:" + certificateHash;
            builder["Connection Timeout"] = 10;
            builder["ConnectRetryCount"] = 0;
            builder["APP"] = applicationName;
            return ToOdbcConnectionString(builder);
        }

        /// <summary>
        /// Builds the on-prem trusted (Windows) loopback connection
        /// string to <c>localhost</c>.
        /// </summary>
        /// <param name="targetDb">Resolved target database name.</param>
        /// <returns>ODBC connection string for the on-prem loopback.</returns>
        private static string BuildOnPremImpliedAuthConnectionString(string targetDb)
        {
            DbConnectionStringBuilder builder = CreateConnectionStringBuilder("localhost", targetDb);
            builder["TrustServerCertificate"] = "Yes";
            builder["Trusted_Connection"] = "Yes";
            return ToOdbcConnectionString(builder);
        }

        /// <summary>
        /// Builds a SQL-authenticated connection string.
        /// </summary>
        /// <param name="server">Loopback server endpoint.</param>
        /// <param name="targetDb">Resolved target database name.</param>
        /// <param name="userName">SQL-authentication user name.</param>
        /// <param name="password">SQL-authentication password.</param>
        /// <param name="trustServerCertificate">
        /// Whether to suppress server-certificate validation.
        /// </param>
        /// <returns>ODBC SQL-authentication connection string.</returns>
        private static string BuildSqlAuthConnectionString(
            string server,
            string targetDb,
            string userName,
            string password,
            bool trustServerCertificate)
        {
            DbConnectionStringBuilder builder = CreateConnectionStringBuilder(server, targetDb);
            if (trustServerCertificate)
            {
                builder["TrustServerCertificate"] = "Yes";
            }

            builder["UID"] = userName;
            builder["PWD"] = password;
            return ToOdbcConnectionString(builder);
        }

        /// <summary>
        /// Creates a connection-string builder with the loopback endpoint
        /// and target database.
        /// </summary>
        /// <param name="server">Loopback server endpoint.</param>
        /// <param name="targetDb">Resolved target database name.</param>
        /// <returns>Connection-string builder with common values.</returns>
        private static DbConnectionStringBuilder CreateConnectionStringBuilder(
            string server,
            string targetDb)
        {
            return new DbConnectionStringBuilder
            {
                ["Server"] = server,
                ["Database"] = targetDb,
            };
        }

        /// <summary>
        /// Prepends the fixed ODBC driver clause to escaped key/value pairs.
        /// </summary>
        /// <param name="builder">Builder containing non-driver values.</param>
        /// <returns>Complete ODBC connection string.</returns>
        private static string ToOdbcConnectionString(DbConnectionStringBuilder builder)
        {
            return DriverClause + builder.ConnectionString + ";";
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
                return dbName;
            }

            string fromEnv = Environment.GetEnvironmentVariable(PhysicalDbNameEnvVar);
            if (!string.IsNullOrEmpty(fromEnv))
            {
                return fromEnv;
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
        private static string GetRequiredEnvironmentVariable(string name)
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
        /// Reads an optional boolean environment flag. Missing or empty
        /// values mean <c>false</c>; <c>TRUE</c> means <c>true</c>.
        /// Any other present value indicates host misconfiguration.
        /// </summary>
        /// <param name="name">Environment variable name.</param>
        /// <returns>Parsed environment flag.</returns>
        private static bool GetStrictEnvironmentFlag(string name)
        {
            string value = Environment.GetEnvironmentVariable(name);
            if (string.IsNullOrEmpty(value))
            {
                return false;
            }

            if (string.Equals(value, "TRUE", StringComparison.OrdinalIgnoreCase))
            {
                return true;
            }

            throw new InvalidOperationException(
                $"Environment variable '{name}' must be either absent or set to 'TRUE'.");
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
            if (value.IndexOfAny(s_invalidEnvironmentValueCharacters) >= 0)
            {
                throw new InvalidOperationException(
                    $"Environment variable '{name}' contains a character that is not allowed "
                    + "in an ODBC connection string.");
            }

            return value;
        }
    }
}
