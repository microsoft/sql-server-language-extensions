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

	rm -rf ${REXTENSIONTEST_WORKING_DIR}/${CMAKE_CONFIGURATION}
	mkdir -p ${REXTENSIONTEST_WORKING_DIR}/${CMAKE_CONFIGURATION}

	pushd ${REXTENSIONTEST_WORKING_DIR}

	# Compile
	#
	cmake -G "Ninja Multi-Config" \
		-DCMAKE_INSTALL_PREFIX:PATH=${REXTENSIONTEST_WORKING_DIR}/${CMAKE_CONFIGURATION} \
		-DCMAKE_CONFIGURATION=${CMAKE_CONFIGURATION} \
		-DENL_ROOT=${ENL_ROOT} \
		-DCMAKE_BUILD_TYPE=${CMAKE_CONFIGURATION} \
		-DPLATFORM=linux \
		-DR_HOME=${R_HOME} \
		-DR_INCLUDE_DIR=${R_INCLUDE} \
		${REXTENSIONTEST_SRC_DIR}
	cmake --build ${REXTENSIONTEST_WORKING_DIR} --target install

	# Check the exit code of the compiler and exit appropriately so that build will fail.
	#
	check_exit_code "Success: Built RExtension-test" "Error: Failed to build RExtension-test"

	# Move the generated libs to configuration folder
	mv RExtension-test ${CMAKE_CONFIGURATION}/

	popd
}

# Enlistment root and location of RExtension-test
#
SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
ENL_ROOT=${SCRIPTDIR}/../../../../..
REXTENSIONTEST_HOME=${ENL_ROOT}/language-extensions/R/test/

# Set environment variables required in Cmake
#
PACKAGES_ROOT=${ENL_ROOT}/packages
REXTENSIONTEST_SRC_DIR=${REXTENSIONTEST_HOME}/src
REXTENSIONTEST_WORKING_DIR=${ENL_ROOT}/build-output/RExtension-test/linux

DEFAULT_R_HOME=/usr/lib/R
DEFAULT_R_INCLUDE=/usr/share/R/include

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

# Find R_INCLUDE from user, or set to default for building the test.
# Error code 1 is generic bash error.
#
if [ -z "${R_INCLUDE}" ]; then
	if [ -d "${DEFAULT_R_INCLUDE}" ]; then
		export R_INCLUDE=${DEFAULT_R_INCLUDE}
	else
		echo "R_INCLUDE is empty"
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
