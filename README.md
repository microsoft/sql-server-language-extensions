# Introduction
This repo contains 3rd-party language extensions for SQL Server. This repo is built and released via Onebranch pipeline and SQL Server consumes the output via a Nuget package.

# Getting Started
Overview:
This repo does not contain any source code but only pipelines. The source code is in the the [open source repository](https://github.com/microsoft/sql-server-language-extensions) where the SQL Server language extensions is hosted. This repo contains the open source repository as a git submodule. The code is tested in the pipeline and in TestShellTests through an artifact.

Whenever there is a new change in the open source repo, this repo needs to update the submodule information to target the newest commit id of the open source repo.

# Build
Each extension should contain its own building process.
All the scripts to trigger the build of an extension for a specific platform should be located in its respective build\<platform> folders 
(e.g. sql-server-language-extensions\language-extensions\java\build\windows).

Java Extension:
There are multiple ways to build the Java Extension.

Prerequisites:
For the first time building the Java Extension, run \sql-server-language-extensions\language-extensions\java\build\windows\restore-packages.cmd to download the nuget containing the JDK to build with. Alternatively, if the version of JDK is updated then re-run this command.

1) For developing open \sql-server-language-extensions\language-extensions\java\build\windows\javaextension.vcxproj in Visual Studio to modify and/or build. This build will only produce the javaextension.dll

2) Run  \sql-server-language-extensions\language-extensions\java\build\windows\build-java-extension.cmd, this will build everything for the Java Extension (javaextension.dll, mssql-java-lang-extension.jar) and it will create a zip of the java-lang-extension.zip package which contains the javaextension.dll/.pdb for local testing with TestShell.
	-By default build-java-extension.cmd builds debug, to build retail run build-java-extension.cmd -retail.

3) Run  \sql-server-language-extensions\language-extensions\java\build\windows\build-java-package.cmd to build mssql-java-lang-extension.jar

# Test
Each extension is responsible for its own unit testing, which should be located in its respective test folder. e.g.  \sql-server-language-extensions\language-extensions\java\test

Currently, there is no local testing for the Java extension in this repo. Full coverage and testing is done via TestShell and SQL labruns.
For local validation of changes, the recommended practice is to do the following steps:
1) Make your changes and run build-java-extension.cmd
2) Copy java-lang-extension.zip to one of the locations
	-Full Setup: SQL Server\binn\
	-XCopy: <build drop>\amd64\, TestShell setup() will copy from here to binn. If setup() is skipped, then copy to binn\.
3) Where possible, add unit tests using the testing framework of your choice.

googletest is provided as one of the test frameworks. It is available in two forms
1) As a prebuilt version added as a nuget package restored in packages\Microsoft.googletest.v140.windesktop.msvcstl.dyn.rt-dyn.1.8.0
This form is only available on Windows. Sample test using this googletest framework is in \test\ (i.e. \test\googletest\sample-test).
2) Latest version downloaded and built in this pipeline using the mingw_64 compiler on Windows and gnu compiler on Linux.

# Release Pipeline
The complete release pipeline does the following:
1) Builds Java Extension debug\release binaries
2) Builds mssql-java-lang-extension.jar
3) Signs the binaries
4) Builds the test executables
5) Runs the unit tests
6) Packages a corresponding retail/debug zip of the signed binaries
7) Creates/publishes a new nuget which contains the following:
	- `<package>/build/debug/java-lang-extension-linux.zip`
	- `<package>/build/debug/java-lang-extension.zip`
	- `<package>/build/debug/mssql-java-lang-extension.jar`
	- `<package>/build/release/java-lang-extension-linux.zip`
	- `<package>/build/release/java-lang-extension.zip`
	- `<package>/build/release/mssql-java-lang-extension.jar`
	- `<package>/build/release/symbols/JavaExtension.pdb`

## Pre-Release Steps
Before starting the release process, update the git submodule inside the repo to ensure all components are up-to-date:

1) Open a terminal or command prompt.
2) Navigate to the root directory of this repository.
3) Run `git submodule update --init --recursive` to initialize and update the submodule.
4) If there are changes, commit them: `git commit -am "Update submodules" && git push`.

## Release Process

### 1. Initiate Official Builds:
- **Access Build Pipelines:** Use the Azure DevOps link to access the official build pipelines: [Official Build Pipelines](https://msazure.visualstudio.com/One/_build?definitionScope=%5COneBranch%5CData-SQL-Language-Extensions).
![Pipelines](/pics/pipelines.png)
- **Schedule Builds:** Schedule two separate official builds, one for Windows and one for Linux, to ensure platform compatibility.
![Pipeline](/pics/pipeline.png)
- **Monitor Builds:** Keep an eye on the build progress and address any issues that arise promptly.

### 2. Creating a New Release:
- **Navigate to Release Management:** After successful build completion, go to: [Release Management](https://msazure.visualstudio.com/One/_release?_a=releases&view=mine&definitionId=47018).
- **Select Artifacts:** Choose the artifacts from the completed Windows and Linux builds to create a new release.
- **Complete Pipeline Steps:** Follow through the pipeline steps including staging deployments, testing, and final publishing, waiting for the process to complete.
![Release](/pics/release.png)

### 3. NuGet Package Management:
- **Verify NuGet Package:** Check the new NuGet package version in the SqlUnified feed: [SqlUnified NuGet Feed](https://msdata.visualstudio.com/Database%20Systems/_artifacts/feed/SqlUnified/NuGet/data-sql-language-extensions). Save from upstream if the version is not updated.
![feed](/pics/feed.png)

### 4. Updating NuGet Version Numbers:
- **Update `package.config`:** Modify the [`package.config`](https://msdata.visualstudio.com/Database%20Systems/_git/DsMainDev?path=%2FSql%2FNtdbms%2Fextensibility%2Fdata-sql-language-extensions%2Fexternals%2Fpackages.config&version=GBmaster&_a=contents) to reference the new package version.
- **Update Project File if Necessary:** If there are changes to the output files or the structure of the NuGet package, also update the [`data-sql-language-extensions.vcxproj`](https://msdata.visualstudio.com/Database%20Systems/_git/DsMainDev?path=%2FSql%2FNtdbms%2Fextensibility%2Fdata-sql-language-extensions%2Fdata-sql-language-extensions.vcxproj&version=GBmaster&_a=contents).

### 5. Running PVS (Private Validation Service):
- **Execute PVS:** Run the PVS to ensure changes are valid, addressing any detected issues.

### 6. Check-in and Merge:
- **Finalize Changes:** After successful validation, check in your changes to the DsMainDev master branch.

### 7. Publishing Releases on GitHub:
- **Download the NuGet Package:** Start by downloading the NuGet package from the feed. Change the file extension from `.nupkg` to `.zip` for extraction.
- **Extract the Package:** Extract the ZIP file contents, preparing the language extensions for the next step.
![nuget](/pics/nuget.png)
- **Create a New Release on GitHub:**
  - Navigate to the [Create Release](https://github.com/microsoft/sql-server-language-extensions/releases/new) page.
  - Fill in the release details, including version tagging, title, and a descriptive changelog.
- **Upload Language Extensions:**
  - Find the attachment section in the release creation page to upload the language extensions.
- **Refer to Sample Releases:** For more information on release structure, refer to the [sample releases](https://github.com/microsoft/sql-server-language-extensions/releases).

# Contribute
Contact ExtensibilityCommitters@service.microsoft.com for questions and contributing.
