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
	if [ -z "${CMAKE_CONFIGURATION}" ]; then
		CMAKE_CONFIGURATION=release
	fi

	pushd ${PYTHONEXTENSIONTEST_WORKING_DIR}/${CMAKE_CONFIGURATION}
	
	# Move the generated libs to configuration folder
	#
	cp ${PYTHONEXTENSION_WORKING_DIR}/${CMAKE_CONFIGURATION}/libPythonExtension.so.1.2 .
	cp /usr/src/gtest/*.so .
	
	ENL_ROOT=${ENL_ROOT} ./pythonextension-test --gtest_output=xml:${ENL_ROOT}/out/TestReport_PythonExtension-test.xml

	# Check the exit code of the tests.
	#
	check_exit_code "Success: Ran pythonextension-test" "Error: pythonextension-test failed"

	popd
}

# Enlistment root and location of pythonextension-test
#
SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
ENL_ROOT=${SCRIPTDIR}/../../../../..
PYTHONEXTENSION_WORKING_DIR=${ENL_ROOT}/build-output/pythonextension/linux
PYTHONEXTENSIONTEST_WORKING_DIR=${ENL_ROOT}/build-output/pythonextension-test/linux
PACKAGES_ROOT=${ENL_ROOT}/packages
DEFAULT_PYTHONHOME=/usr

# Find PYTHONHOME from user, or set to default for tests.
# Error code 1 is generic bash error.
# 
if [ -z "${PYTHONHOME}" ]; then
	if [ -x "${DEFAULT_PYTHONHOME}" ]; then 
		PYTHONHOME=${DEFAULT_PYTHONHOME}
	else
		echo "PYTHONHOME is empty"
		exit 1
	fi
fi

echo "Python home is ${PYTHONHOME}"

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
