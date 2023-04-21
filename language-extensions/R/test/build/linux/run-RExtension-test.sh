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
	CMAKE_CONFIGURATION=$1
	if [ -z "${CMAKE_CONFIGURATION}" ]; then
		CMAKE_CONFIGURATION=release
	fi

	pushd ${REXTENSIONTEST_WORKING_DIR}/${CMAKE_CONFIGURATION}
	# Move the generated libs to configuration folder
	cp ${REXTENSION_WORKING_DIR}/${CMAKE_CONFIGURATION}/libRExtension.so.1.2 .
	./RExtension-test --gtest_output=xml:${ENL_ROOT}/out/TestReport_RExtension-test.xml

	# Check the exit code of the tests.
	check_exit_code "Success: Ran RExtension-test" "Error: RExtension-test failed"

	popd
}

# Enlistment root and location of RExtension-test
SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
export ENL_ROOT=${SCRIPTDIR}/../../../../..
REXTENSION_WORKING_DIR=${ENL_ROOT}/build-output/RExtension/linux
REXTENSIONTEST_WORKING_DIR=${ENL_ROOT}/build-output/RExtension-test/linux
PACKAGES_ROOT=${ENL_ROOT}/packages

DEFAULT_R_HOME=/usr/lib/R

# Find R_HOME from user, or set to default for testing.
# Error code 1 is generic bash error.
#
if [ -z "${R_HOME}" ]; then
	if [ -d "${DEFAULT_R_HOME}" ]; then
		export R_HOME=${DEFAULT_R_HOME}
	else
		echo "R_HOME is empty"
		exit 1
	fi
fi

# Test in release mode if nothing is specified
#
if [ "$1" == "" ]; then
	set -- release
fi

while [ "$1" != "" ]; do
	# Advance arg passed to build.cmd
	build $1
	shift
done;
