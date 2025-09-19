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

# Enlistment root and location of googletest
#
SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
ENL_ROOT=${SCRIPTDIR}/../../../..
PACKAGES_ROOT=${ENL_ROOT}/packages
GTEST_HOME=${ENL_ROOT}/test/googletest

# Set environment variables required in cmake
#
BUILD_OUTPUT=${ENL_ROOT}/build-output/googletest/linux

# Generate the build output directory
#
rm -rf ${BUILD_OUTPUT}
mkdir -p ${BUILD_OUTPUT}
pushd ${BUILD_OUTPUT}

# Compile
#
cmake -DCMAKE_INSTALL_PREFIX:PATH=${BUILD_OUTPUT} \
	-DPLATFORM=linux \
	${GTEST_HOME}/src
cmake --build ${BUILD_OUTPUT} --config ${CMAKE_CONFIGURATION} --target install

# Check the exit code of the compiler and exit appropriately so that build will fail.
#
check_exit_code "Success: Built googletest" "Error: Failed to build Rextension-test"

popd

exit $?
