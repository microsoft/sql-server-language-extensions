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
		CMAKE_CONFIGURATION=debug
	fi

	pushd ${REXTENSIONTEST_WORKING_DIR}/${CMAKE_CONFIGURATION}
	# Move the generated libs to configuration folder
	cp ${REXTENSION_WORKING_DIR}/${CMAKE_CONFIGURATION}/libRextension.so.1.0 .
	cp /usr/src/gtest/*.so .
	export R_HOME=/opt/microsoft/ropen/3.5.2/lib64/R
	./Rextension-test --gtest_output=xml:${ENL_ROOT}/out/TestReport_Rextension-test.xml

	# Check the exit code of the tests.
	check_exit_code "Success: Ran Rextension-test" "Error: Rextension-test failed"

	popd
}

# Enlistment root and location of Rextension-test
SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
ENL_ROOT=${SCRIPTDIR}/../../../../..
REXTENSION_WORKING_DIR=${ENL_ROOT}/.build/R-extension/linux
REXTENSIONTEST_WORKING_DIR=${ENL_ROOT}/.build/Rextension-test/linux
PACKAGES_ROOT=${ENL_ROOT}/packages

while [ "$1" != "" ]; do
	# Advance arg passed to build.cmd
	build $1
	shift
done;
