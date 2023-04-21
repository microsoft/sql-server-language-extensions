# R Language Extension

## Getting Started
Language Extensions is a feature of SQL Server used for executing external code. The relational data can be used in the external code using the extensibility framework.

For more information about SQL Server Language Extensions, refer to this [documentation](https://docs.microsoft.com/en-us/sql/language-extensions/language-extensions-overview?view=sql-server-ver15).

The RExtension version in this repository is compatible with SQL Server 2019 CU3 onwards. It integrates R in SQL Server and works with R >= v3.3, however it is strongly recommended to use the latest stable version. At the time of this writing, it is R v4.2.2.

To use this released R-language-extension.zip package, follow [this tutorial](https://docs.microsoft.com/en-us/sql/machine-learning/install/custom-runtime-r?view=sql-server-ver15). For any fixes or enhancements, you are welcome to modify, rebuild and use the binaries using the following instructions.

## Building

### Windows
There are two alternatives to building this project.
A.	Using the [**restore-packages.cmd**](build/windows/restore-packages.cmd) script

1.	Modify the following as appropriate
	- `R_VERSION` on line 15 in [**restore-packages.cmd**](build/windows/restore-packages.cmd)
	- `DEFAULT_R_HOME` on line 10 in [**build-RExtension.cmd**](build/windows/build-RExtension.cmd)

1.	Run [**restore-packages.cmd**](build/windows/restore-packages.cmd)

1. Continue on **Step 5** below.

B.	Installing needed packages from respective sources.

1. Install [rtools40-x86_64.exe](https://cran.r-project.org/bin/windows/Rtools/) to get the gcc v8.3.0 toolchain. Set RTOOLS40_HOME to point to this rtools40 installation path. Also install [Rtools35.exe](https://cran.r-project.org/bin/windows/Rtools/Rtools35.exe) to use MinGW Makefiles as the generator for cmake and mingw32-make as the build tool. Set RTOOLS35_HOME to point to this Rtools35 installation path.

1. Install [CMake for Windows](https://cmake.org/download/). Set CMAKE_ROOT to point to the cmake installation folder.

1. Install [R for Windows](https://cran.r-project.org/bin/windows/base/) and set R_HOME to point to your installation path.

1. Install the [Rcpp](https://cran.r-project.org/web/packages/Rcpp/index.html) and [RInside](https://cran.r-project.org/web/packages/RInside/index.html) packages into this R runtime. These are needed to seamlessly integrate and embed R in C++.

1. Run [**build-RExtension.cmd**](./build/windows/build-RExtension.cmd) which will generate: \
		- PATH\TO\ENLISTMENT\build-output\RExtension\windows\release\libRExtension.dll

1. Run [**create-RExtension-zip.cmd**](./build/windows/create-RExtension-zip.cmd) which will generate: \
		- PATH\TO\ENLISTMENT\build-output\RExtension\windows\release\packages\R-lang-extension.zip \
		This zip can be used in CREATE EXTERNAL LANGUAGE, as detailed in the tutorial in the Usage section below.

### Linux
There are two alternatives to building this project.

A.	Using the [**restore-packages.sh**](build/linux/restore-packages.sh) script

1.	Modify `DEFAULT_R_HOME` on line 29 in [**restore-packages.sh**](build/linux/restore-packages.sh) as appropriate.

1.	Run [**restore-packages.sh**](build/linux/restore-packages.sh)

1. Continue on **Step 4** below.

B.	Installing needed packages from respective sources.

1. Install all packages necessary for c++ compilation on Linux based on your distribution.
	```
	#bash e.g. for Ubuntu
	sudo apt-get -q -y install unixodbc-dev
	sudo apt-get install build-essential software-properties-common -y
	sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
	sudo apt-get update -y
	sudo apt-get install gcc-7 g++-7 -y
	sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 70 --slave /usr/bin/g++ g++ /usr/bin/g++-7
	```

1. Install [R for Linux](https://cran.r-project.org/bin/linux/) based on your distribution. Set R_HOME to point to your installation path, by default it is /usr/lib/R. Set R_INCLUDE_HOME to point to the R include files, by default it is /usr/share/R/include
	```
	#bash e.g. for Ubuntu
	sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys E298A3A825C0D65DFD57CBB651716619E084DAB9
	sudo add-apt-repository 'deb https://cloud.r-project.org/bin/linux/ubuntu xenial-cran40/'
	sudo apt-get update
	sudo apt-get -y install r-base-core
	```

1. Install the [Rcpp](https://cran.r-project.org/web/packages/Rcpp/index.html) and [RInside](https://cran.r-project.org/web/packages/RInside/index.html) packages into this R runtime. These are needed to seamlessly integrate and embed R in C++.

1. Run [**build-RExtension.sh**](./build/linux/build-RExtension.sh) which will generate: \
		- /PATH/TO/ENLISTMENT/build-output/RExtension/linux/release/libRExtension.so.1.2

1. Run [**create-R-extension-zip.sh**](./build/linux/create-RExtension-zip.sh) which will generate: \
		- /PATH/TO/ENLISTMENT/build-output/RExtension/linux/release/packages/R-lang-extension.zip \
		This zip can be used in CREATE EXTERNAL LANGUAGE, as detailed in the tutorial in the Usage section below.

## Testing (Optional)

### Windows
1. (Optional) To unit test this extension,
	+ Run [**build-googletest.cmd**](../test/googletest/build/windows/build-googletest.cmd) which will generate the gtest library essential to build the RExtension-test executable: \
		- PATH\TO\ENLISTMENT\build-output\googletest\windows\lib\libgtest.a
	+ Run [**build-RExtension-test.cmd**](./test/build/windows/build-RExtension-test.cmd) which will generate: \
		- PATH\TO\ENLISTMENT\build-output\RExtension-test\windows\release\RExtension-test.exe
	+ Run [**run-RExtension-test.cmd**](./test/build/windows/run-RExtension-test.cmd) to run all the unit tests.

### Linux
1. (Optional) To unit test this extension,
	+ Run [**build-googletest.sh**](../test/googletest/build/linux/build-googletest.sh) which will generate the gtest library essential to build the RExtension-test binary: \
		- /PATH/TO/ENLISTMENT/build-output/googletest/linux/lib/libgtest.a
	+ Run [**build-RExtension-test.sh**](./test/build/linux/build-RExtension-test.sh) which will generate: \
		- /PATH/TO/ENLISTMENT/build-output/RExtension-test/windows/release/RExtension-test.exe
	+ Run [**run-RExtension-test.sh**](./test/build/linux/run-RExtension-test.sh) to run all the unit tests.

## Usage
After downloading or building the R-lang-extension.zip, use [CREATE EXTERNAL LANGUAGE](https://docs.microsoft.com/en-us/sql/t-sql/statements/create-external-language-transact-sql?view=sql-server-ver15) to register the language with SQL Server 2019 CU3+.

This [tutorial](https://docs.microsoft.com/en-us/sql/machine-learning/install/custom-runtime-r?view=sql-server-ver15) will walk you through an end to end sample using the R language extension.
