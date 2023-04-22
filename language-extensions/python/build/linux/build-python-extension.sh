#!/bin/bash

function check_exit_code {
	EXIT_CODE=$?
	if [ ${EXIT_CODE} -eq 0 ]; then
		echo $1
	else
		echo $2
		exit ${EXIT_CODE}
	fi
}

function build {
	# Set cmake config to first arg
	#
	CMAKE_CONFIGURATION=$1

	# Output directory and output lib name
	#
	TARGET=${ENL_ROOT}/build-output/pythonextension/target/${CMAKE_CONFIGURATION}

	# Create the output directories
	#
	mkdir -p ${TARGET}

	if ! [[ -d ${PYTHONEXTENSION_WORKING_DIR} ]]; then
		mkdir -p ${PYTHONEXTENSION_WORKING_DIR}
	fi

	cd ${PYTHONEXTENSION_WORKING_DIR}

	# Compile
	#
	cmake -DPLATFORM=Linux \
		-DENL_ROOT=${ENL_ROOT} \
		-DCMAKE_BUILD_TYPE=${CMAKE_CONFIGURATION} \
		-DPYTHONHOME=${PYTHONHOME} \
		-DBOOST_PYTHON_ROOT=${BOOST_PYTHON_ROOT} \
		-DINCLUDE_ROOT=${INCLUDE_ROOT} \
		${PYTHONEXTENSION_HOME}/src
	cmake --build ${PYTHONEXTENSION_WORKING_DIR} --config ${CMAKE_CONFIGURATION} --target install

	# Check the exit code of the compiler and exit appropriately so that build will fail.
	#
	check_exit_code "Success: Built libPythonExtension.so.1.2" "Error: Failed to build python extension"

	# Move the generated libs to configuration folder
	#
	mkdir -p ${CMAKE_CONFIGURATION}
	mv libPythonExtension.so* ${CMAKE_CONFIGURATION}/

	cd ${CMAKE_CONFIGURATION}/

	# This will create the python extension package with unsigned binaries, this is used for local development and non-release builds. release
	# builds will call create-python-extension-zip.sh after the binaries have been signed and this will be included in the zip
	#
	zip ${TARGET}/python-lang-extension libPythonExtension.so.1.2

	check_exit_code "Success: Created python-lang-extension.zip" "Error: Failed to create zip for python extension"
}

# Enlistment root and location of pythonextension
#
SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
ENL_ROOT=${SCRIPTDIR}/../../../..

# Set environment variables required in Cmake
#
PYTHONEXTENSION_HOME=${ENL_ROOT}/language-extensions/python
PYTHONEXTENSION_WORKING_DIR=${ENL_ROOT}/build-output/pythonextension/linux

INCLUDE_ROOT=/usr/include

DEFAULT_PYTHONHOME=/usr
DEFAULT_BOOST_ROOT=/usr/lib/boost_1_79_0

# Find PYTHONHOME from user, or set to default for tests.
# Error code 1 is generic bash error.
#
if [ -z "${PYTHONHOME}" ]; then
	if [ -x "${DEFAULT_PYTHONHOME}" ]; then
		PYTHONHOME=${DEFAULT_PYTHONHOME}
	else
		echo "PYTHONHOME is empty but needs to be set to build the python extension"
		exit 1
	fi
fi

# Find BOOST_ROOT from user, or set to default for tests.
# Error code 1 is generic bash error.
#
if [ -z "${BOOST_ROOT}" ]; then
	if [ -x "${DEFAULT_BOOST_ROOT}" ]; then
		BOOST_ROOT=${DEFAULT_BOOST_ROOT}
	else
		echo "BOOST_ROOT is empty but needs to be set to build the python extension"
		exit 1
	fi
fi

DEFAULT_BOOST_PYTHON_ROOT=${BOOST_ROOT}/stage/lib

# Find BOOST_PYTHON_ROOT from user, or set to default for tests.
# Error code 1 is generic bash error.
#
if [ -z "${BOOST_PYTHON_ROOT}" ]; then
	if [ -x "${DEFAULT_BOOST_PYTHON_ROOT}" ]; then
		BOOST_PYTHON_ROOT=${DEFAULT_BOOST_PYTHON_ROOT}
	else
		echo "BOOST_PYTHON_ROOT is empty but needs to be set to build the python extension"
		exit 1
	fi
fi

# Build in release mode if nothing is specified
#
if [ "$1" == "" ]; then
	set -- release
fi

while [ "$1" != "" ]; do
	# Advance arg passed to build.cmd
	#
	build $1
	shift
done;
