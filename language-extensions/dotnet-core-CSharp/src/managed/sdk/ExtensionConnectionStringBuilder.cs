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
    /// The primary entry point,
    /// <see cref="BuildLoopbackConnectionString"/>, selects between the
    /// XDB (SQL DB) and classic on-prem loopback shapes based
    /// on the <c>IS_XDB</c> environment variable that launchpad sets
    /// on every XDB satellite process. Extension user code should call
    /// it without inspecting the environment itself.
    /// </para>
    /// <para>
    /// A separate <see cref="BuildSqlAuthLoopbackConnectionString"/>
    /// overload is provided for the small set of internal test
    /// scenarios that need explicit SQL-auth loopbacks; that path is
    /// NOT intended for extension user code.
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
        public const string IsXdbEnvVar = "IS_XDB";

        /// <summary>
        /// Environment variable holding the TDS loopback endpoint in
        /// "host,port" form. Set by launchpad on XDB. Read by the
        /// SQL-auth path when running on XDB.
        /// </summary>
        public const string LoopbackEndpointEnvVar = "SqlTdsLoopbackConnectionEndpoint";

        /// <summary>
        /// Environment variable holding the SNI loopback named-pipe
        /// name (with or without an <c>np:</c> prefix). Set by
        /// launchpad on XDB. Read to build the cert-based implied-auth
        /// XDB loopback.
        /// </summary>
        public const string LoopbackPipeEnvVar = "LoopbackConnectionPipe";

        /// <summary>
        /// Environment variable holding the SHA-1 thumbprint of the
        /// extensibility client certificate the loopback should
        /// present. Set by launchpad on XDB.
        /// </summary>
        public const string CertificateHashEnvVar = "ExtensibilityCertificateHash";

        /// <summary>
        /// Environment variable holding the name of the physical
        /// database the caller is executing against. Read as the
        /// default when no <c>dbName</c> is passed. Set by launchpad.
        /// </summary>
        public const string PhysicalDbNameEnvVar = "PhysicalDbName";

        /// <summary>
        /// Fallback database name used when no <c>dbName</c> is passed
        /// and <see cref="PhysicalDbNameEnvVar"/> is not set.
        /// </summary>
        public const string DefaultDatabaseName = "master";

        /// <summary>
        /// ODBC driver clause used for every loopback connection
        /// string.
        /// </summary>
        public const string DriverClause = "Driver={ODBC Driver 18 for SQL Server};";

        /// <summary>
        /// ODBC option that suppresses server-certificate validation.
        /// ODBC Driver 18 defaults <c>Encrypt=Yes</c>, so this is
        /// included on paths that connect to endpoints presenting a
        /// self-signed certificate (the on-prem <c>localhost</c>
        /// loopback and the XDB SNI named-pipe loopback).
        /// </summary>
        public const string TrustServerCertificateClause = "TrustServerCertificate=Yes;";

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
        /// Returns <c>true</c> when the current process is an XDB /
        /// WCOW satellite (SQL DB or SQL MI); <c>false</c> for classic
        /// on-prem SQL Server. Determined from the <c>IS_XDB</c>
        /// environment variable set by launchpad at satellite launch.
        /// </summary>
        public static bool IsXdbHost()
        {
            string value = Environment.GetEnvironmentVariable(IsXdbEnvVar);
            return string.Equals(value, "TRUE", StringComparison.OrdinalIgnoreCase);
        }

        /// <summary>
        /// Builds an implied-auth loopback ODBC connection string for
        /// the local SQL Server, selecting between the XDB (cert-based
        /// SNI named-pipe) and on-prem (trusted <c>localhost</c>)
        /// transports from the <c>IS_XDB</c> environment variable that
        /// launchpad sets on XDB satellites.
        /// 
        /// In both cases the resulting connection authenticates as the
        /// original caller, not as the satellite process identity.
        /// </summary>
        /// <param name="dbName">
        /// Target database name. When null or empty the value of the
        /// <see cref="PhysicalDbNameEnvVar"/> environment variable is
        /// used; if that is also not set the connection string targets
        /// <see cref="DefaultDatabaseName"/>.
        /// </param>
        /// <returns>ODBC connection string for the loopback endpoint.</returns>
        /// <exception cref="InvalidOperationException">
        /// Thrown when <c>IS_XDB=TRUE</c> but launchpad has not also
        /// published <see cref="LoopbackPipeEnvVar"/> and
        /// <see cref="CertificateHashEnvVar"/>. Silent fallback to
        /// on-prem in that case would connect as the wrong identity.
        /// </exception>
        public static string BuildLoopbackConnectionString(string dbName = null)
        {
            string targetDb = ResolveDatabaseName(dbName);

            if (IsXdbHost())
            {
                string pipe = RequireEnvironmentVariable(LoopbackPipeEnvVar);
                string thumbprint = RequireEnvironmentVariable(CertificateHashEnvVar);
                return BuildXdbImpliedAuthConnectionString(pipe, thumbprint, targetDb);
            }

            return BuildOnPremImpliedAuthConnectionString(targetDb);
        }

        /// <summary>
        /// Builds a SQL-authenticated ODBC loopback connection string,
        /// selecting the loopback transport from the <c>IS_XDB</c>
        /// environment variable. EFX implied auth is NOT used - the
        /// loopback authenticates with the supplied SQL login.
        /// Intended for internal test scenarios that need an explicit
        /// SQL login; extension user code should use
        /// <see cref="BuildLoopbackConnectionString"/> instead.

        /// </summary>
        /// <param name="dbName">
        /// Target database name. When null or empty the value of the
        /// <see cref="PhysicalDbNameEnvVar"/> environment variable is
        /// used; if that is also not set the connection string targets
        /// <see cref="DefaultDatabaseName"/>.
        /// </param>
        /// <param name="userName">SQL-auth user name. Required.</param>
        /// <param name="password">SQL-auth password.</param>
        /// <returns>ODBC connection string for the loopback endpoint.</returns>
        /// <exception cref="ArgumentException">
        /// Thrown when <paramref name="userName"/> is null or empty.
        /// </exception>
        /// <exception cref="InvalidOperationException">
        /// Thrown when <c>IS_XDB=TRUE</c> but
        /// <see cref="LoopbackEndpointEnvVar"/> is not set.
        /// </exception>
        public static string BuildSqlAuthLoopbackConnectionString(
            string dbName,
            string userName,
            string password)
        {
            if (string.IsNullOrEmpty(userName))
            {
                throw new ArgumentException(
                    "SQL authentication requires a non-empty user name.",
                    nameof(userName));
            }

            string targetDb = ResolveDatabaseName(dbName);

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
        /// or empty. Used for XDB loopback inputs whose absence
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

            return value;
        }
    }
}
