# Java Language Extension 

## Getting Started
Language Extensions is a feature of SQL Server used for executing external code. The relational data can be used in the external code using the extensibility framework.

For more information about SQL Server Language Extensions, refer to this [documentation](https://docs.microsoft.com/en-us/sql/language-extensions/language-extensions-overview?view=sql-server-ver15).

The Java extension version in this repository is compatible with SQL Server 2019 CU3 onwards. 

This [tutorial](https://docs.microsoft.com/en-us/sql/language-extensions/tutorials/search-for-string-using-regular-expressions-in-java?view=sqlallproducts-allversions) will walk you through an end to end sample using the Java language extension for SQL Server. Sample code files for the tutorial can also be found under samples in this repository. [Sample code files](./samples/regex) are also available in this repository.



## Things to Know

* The Java components in this project are built on Java 8
* The [**Microsoft Extensibility SDK for Java**](sdk) is also installed as part of SQL Server 2019 on both Windows as Linux
  * Default installation path on Windows: [instance installation home directory]\MSSQL\Binn\mssql-java-lang-extension.jar
  * Default installation path on Linux: /opt/mssql/lib/mssql-java-lang-extension.jar 
  
## Building

### Windows
A.	Using the [**restore-packages.cmd**](build/windows/restore-packages.cmd) script

1.	Modify the following as appropriate
	- `JAVA_VERSION` on line 5 in [**restore-packages.cmd**](build/windows/restore-packages.cmd)
	- `DEFAULT_JAVA_HOME` on line 11 in [**build-RExtension.cmd**](build/windows/build-java-extension.cmd)

1.	Run [**restore-packages.cmd**](build/windows/restore-packages.cmd)

1. Continue on **Step 4** below.

B.	Installing needed packages from respective sources.

1. Install [CMake for Windows](https://cmake.org/download/) and Java ([file](https://www.azul.com/downloads/zulu-community/?architecture=x86-64-bit&package=jdk) and [instructions](https://docs.azul.com/zulu/zuludocs/ZuluUserGuide/InstallingZulu/InstallWindowsUsingZuluZIPFile.htm))

1. Install C++ Tools for CMake from the [Build Tools for Visual Studio 2017](https://my.visualstudio.com/Downloads?q=visual%20studio%202017&wt.mc_id=o~msft~vscom~older-downloads). 
		Download the Visual Studio 2017 Build Tools installer and check the **Visual C++ build tools** option under Workloads. In the sidebar on the right, make sure **Visual C++ tools for CMake** is checked, then install.

1. Set CMAKE_ROOT and JAVA_HOME pointing to the respective folders.

1. Run [**build-java-extension.cmd**](build/windows/build-java-extension.cmd) which will generate two main files: \
		- PATH\TO\ENLISTMENT\build-output\java-extension\windows\release\release\java-extension.dll \
		- PATH\TO\ENLISTMENT\build-output\java-extension\target\release\mssql-java-lang-extension.jar

1. Run [**create-java-extension-zip.cmd**](build/windows/create-java-extension-zip.cmd) which will generate: \
		- PATH\TO\ENLISTMENT\build-output\java-extension\windows\release\release\packages\java-lang-extension.zip \
		This zip can be used in CREATE EXTERNAL LANGUAGE, as detailed in the tutorial in the Usage section below.

### Linux
There are two alternatives to building this project.

A.	Using the [**restore-packages.sh**](build/linux/restore-packages.sh) script

1.	Modify `openjdk-17-jdk` on line 14 in [**restore-packages.sh**](build/linux/restore-packages.sh) as appropriate.

1.	Run [**restore-packages.sh**](build/linux/restore-packages.sh)

1. Continue on **Step 3** below.

1. Install [CMake for Linux](https://cmake.org/download/) and [Java](https://docs.azul.com/zulu/zuludocs/ZuluUserGuide/InstallingZulu/InstallOnLinuxUsingAPTRepository.htm)

1. Set JAVA_HOME pointing to the Java folder.

1. Run [**build-java-extension.sh**](build/linux/build-java-extension.sh) which will generate two main files: \
		- PATH/TO/ENLISTMENT/build-output/java-extension/linux/release/libJavaExtension.so.1.0 \
		- PATH/TO/ENLISTMENT/build-output/java-extension/target/release/mssql-java-lang-extension-linux.jar

1. Run [**create-java-extension-zip.sh**](build/linux/create-java-extension-zip.sh) which will generate: \
		- PATH/TO/ENLISTMENT/build-output/java-extension/linux/release/packages/java-lang-extension.zip \
		This zip can be used in CREATE EXTERNAL LANGUAGE, as detailed in the tutorial in the Usage section below.

## Usage
After creating the Java extension zip, use [CREATE EXTERNAL LANGUAGE](https://docs.microsoft.com/en-us/sql/t-sql/statements/create-external-language-transact-sql?view=sql-server-ver15) to create the language on the SQL Server. 

This [tutorial](https://docs.microsoft.com/en-us/sql/language-extensions/tutorials/search-for-string-using-regular-expressions-in-java?view=sqlallproducts-allversions) will walk you through an end to end sample using the Java language extension. 
