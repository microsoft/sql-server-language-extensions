# Introduction
This repo contains 3rd-party language extensions for SQL Server. Currently, there is only the Java Extension. This repo is built and released via CDPx pipeline and SQL Server consumes the output via a Nuget package.

# Getting Started
Overview:
The directory \extension-host\ contains the extension API, while \language-extensions\ contains all of the extensions (i.e. language-extensions\java).
All the scripts to trigger the builds of all extensions and projects are in \build\. The gtest testing framework and sample-test projects are in \test\.
Other linked repositories are in \submodules\.

# Build and Test
Each extension should contain its own building process.

Java Extension:
There are multiple ways to build the Java Extension.

Prerequisites:
For the first time building the Java Extension, run \build\restore-packages.cmd to download the nuget containing the JDK to build with. Alternatively, if the version of JDK is updated then re-run this command.

1) For developing open \language-extensions\java\build\windows\javaextension.vcxproj in Visual Studio to modify and/or build. This build will only produce the javaextension.dll

2) Run \build\build-java-extension.cmd, this will build everything for the Java Extension (javaextension.dll, mssql-java-lang-extension.jar) and it will create a zip of the java-lang-extension.zip package which contains the javaextension.dll/.pdb for local testing with TestShell.
	-By default build-java-extension.cmd builds debug, to build retail run build-java-extension.cmd -retail.

3) Run \build\build-java-package.cmd to build mssql-java-lang-extension.jar

Currently, there is no local testing for the Java extension in this repo. Full coverage and testing is done via TestShell and SQL labruns.

For local validation of changes, the recommended practice is to do the following steps:
1) Make your changes and run build-java-extension.cmd
2) Copy java-lang-extension.zip to one of the locations
	-Full Setup: SQL Server\binn\
	-XCopy: <build drop>\amd64\, TestShell setup() will copy from here to binn. If setup() is skipped, then copy to binn\.
3) Where possible, add unit tests using the gtest testing framework.

# Release Pipeline
The complete release pipeline does the following:
1) Builds Gtest debug\relwithdebinfo binaries
2) Builds Java Extension debug\release binaries
3) Builds mssql-java-lang-extension.jar
4) Signs the binaries
5) Builds the test executables
6) Runs the unit tests
7) Packages a corresponding retail/debug zip of the signed binaries
8) Creates/publishes a new nuget which contains the following:
	<package>/build/debug/javaextension.dll
	<package>/build/debug/mssql-java-lang-extension.jar
	<package>/build/debug/java-lang-extension.zip
	<package>/build/debug/symbols/javaextension.pdb


	<package>/build/release/javaextension.dll
	<package>/build/release/mssql-java-lang-extension.jar
	<package>/build/release/java-lang-extension.zip
	<package>/build/release/symbols/javaextension.pdb

To create a release do the following:
1) Run an official build from master, this will perform steps 1-4) mentioned above.
2) From the official build, create a new release which will perform step 5).
3) Update the nuget version numbers in DS_Main_Dev and run PVS before checking in.

# Contribute
Contact brnieb@microsoft.com, yaelh@microsoft.com, or sqlext@microsoft.com for questions and contributing.