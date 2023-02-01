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

	rm -rf ${PYTHONEXTENSIONTEST_WORKING_DIR}/${CMAKE_CONFIGURATION}
	mkdir -p ${PYTHONEXTENSIONTEST_WORKING_DIR}/${CMAKE_CONFIGURATION}

	pushd ${PYTHONEXTENSIONTEST_WORKING_DIR}

	# Compile
	#
	cmake -DCMAKE_INSTALL_PREFIX:PATH=${PYTHONEXTENSIONTEST_WORKING_DIR}/${CMAKE_CONFIGURATION} \
		-DPLATFORM=linux \
		-DCMAKE_CONFIGURATION=${CMAKE_CONFIGURATION} \
		-DENL_ROOT=${ENL_ROOT} \
		-DINCLUDE_ROOT=${INCLUDE_ROOT} \
		-DPYTHONHOME=${PYTHONHOME} \
		-DBOOST_PYTHON_ROOT=${BOOST_PYTHON_ROOT} \
		${PYTHONEXTENSIONTEST_SRC_DIR}
	cmake --build ${PYTHONEXTENSIONTEST_WORKING_DIR} --config ${CMAKE_CONFIGURATION} --target install

	# Check the exit code of the compiler and exit appropriately so that build will fail.
	#
	check_exit_code "Success: Built pythonextension-test" "Error: Failed to build pythonextension-test"

	# Move the generated libs to configuration folder
	#
	mv pythonextension-test ${CMAKE_CONFIGURATION}/

	popd
}

# Enlistment root and location of pythonextension-test
#
SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
ENL_ROOT=${SCRIPTDIR}/../../../../..
PYTHONEXTENSIONTEST_HOME=${ENL_ROOT}/language-extensions/python/test/

# Set environment variables required in Cmake
#
PYTHONEXTENSIONTEST_SRC_DIR=${PYTHONEXTENSIONTEST_HOME}/src
PYTHONEXTENSIONTEST_WORKING_DIR=${ENL_ROOT}/build-output/pythonextension-test/linux

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
		echo "PYTHONHOME is empty but needs to be set to build the python extension test"
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
		echo "BOOST_ROOT is empty but needs to be set to build the python extension test"
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
		echo "BOOST_PYTHON_ROOT is empty but needs to be set to build the python extension test"
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
