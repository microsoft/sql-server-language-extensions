# Java Language Extension 

## Getting Started
Language Extensions is a feature of SQL Server used for executing external code. The relational data can be used in the external code using the extensibility framework.

For more information about SQL Server Language Extensions on Microsoft Learn, refer to this [article](https://docs.microsoft.com/sql/language-extensions/language-extensions-overview).

## Compatibility Matrix

The Java extension version in this repository is compatible with the following versions of SQL: 

| SQL Server Version            | Java Language Extension Support |
|-------------------------------|---------------------------------|
| SQL Server 2019 (GDR - CU2)   | :x:                             |
| SQL Server 2019 (CU3 and onwards) | :white_check_mark:              |
| (Azure) SQL Managed Instance  | :white_check_mark:              |
| SQL Server 2022               | :white_check_mark:              |

## What you will achieve by the end of this tutorial

This [tutorial](https://docs.microsoft.com/en-us/sql/language-extensions/tutorials/search-for-string-using-regular-expressions-in-java?view=sqlallproducts-allversions) will walk you through an end to end sample using the Java language extension for SQL Server on both Windows and Linux.

Sample code files for the tutorial can also be found under samples in this repository. [Sample code files](./samples/regex) are also available in this repository.

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

#### Restore package dependencies

1. Run the Language Extension repository's package restore script [**restore-packages.sh**](../../restore-packages.sh) located at the **root** of this repository.

    ```sh
    ../../restore-package.sh
    ```
   1. (*Optional*) By default, these instructions use `openjdk-17-jdk`. If you would like to use a different version of the JDK, install that version separately and ensure your `JAVA_HOME` environment variable is set to point to your desired JDK.
1. Run the Java Extension's package restore script from `/build/linux`: [**restore-packages.sh**](build/linux/restore-packages.sh).
    ```sh
    ./build/linux/restore-packages.sh
    ```
1. Set the environment variable `JAVA_HOME` to point to your desired JDK version.
    1. Identify installed versions of Java:

        ```sh
        sudo update-alternatives --config java
        ```
    1. Set the `JAVA_HOME` environment variable by copying the location of the JDK (Remove path contents including /bin and after)
        ```sh
		## 1. Copy the jdk version MINUS all content including /bin and after
		sudo nano /etc/environment
		## 2. On a new line in the environment file, add the following:
		JAVA_HOME="/usr/lib/jvm/java-17-openjdk-amd64/"
		## 3. Save (write out using ctrl+o) and then hit enter to use the default resolved file name.
		## 4. Reload your shell environment so the new environment variable is available.
		source /etc/environment
		## 5. Confirm that the JAVA_HOME environment variable is set, the following command should print out the value you set in your etc/environment file.
		echo $JAVA_HOME
        ```

#### Build Java Extension

1. Run [**build-java-extension.sh**](build/linux/build-java-extension.sh) which will generate two main files:
    ```sh
    ./build-java-extension.sh
	```
    1. **libJavaExtension.so.1.0** 
        - `<REPOSITORY_ROOT>/build-output/java-extension/linux/release/libJavaExtension.so.1.0`
	1. **mssql-java-lang-extension.jar** 
	    - `<REPOSITORY_ROOT>/build-output/java-extension/target/release/mssql-java-lang-extension.jar`

1. Run [**create-java-extension-zip.sh**](build/linux/create-java-extension-zip.sh) to create the zip file you will use with `CREATE EXTERNAL LANGUAGE`, as detailed in the next section.
    ```sh
	./create-java-extension-zip.sh
	```
   - **java-lang-extension.zip** 
     - `<REPOSITORY_ROOT>/build-output/java-extension/linux/release/packages/java-lang-extension.zip`

## Create External Language

After creating the Java extension zip, use [CREATE EXTERNAL LANGUAGE](https://docs.microsoft.com/en-us/sql/t-sql/statements/create-external-language-transact-sql?view=sql-server-ver15) to create the language on the SQL Server.
```tsql
CREATE EXTERNAL LANGUAGE ()
```

## Follow up tutorial

This [tutorial](https://docs.microsoft.com/en-us/sql/language-extensions/tutorials/search-for-string-using-regular-expressions-in-java?view=sqlallproducts-allversions) will walk you through an end to end sample using the Java language extension. 
