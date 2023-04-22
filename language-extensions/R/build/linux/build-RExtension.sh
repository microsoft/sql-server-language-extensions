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

	# Output directory and output dll name
	#
	TARGET=${ENL_ROOT}/build-output/RExtension/target/${CMAKE_CONFIGURATION}
	# Create the output directories
	mkdir -p ${TARGET}

	if ! [[ -d ${REXTENSION_WORKING_DIR} ]]; then
		mkdir -p ${REXTENSION_WORKING_DIR}
	fi
	cd ${REXTENSION_WORKING_DIR}
	# Compile
	#
	cmake -DCMAKE_CONFIGURATION=${CMAKE_CONFIGURATION} \
		-DCMAKE_INSTALL_PREFIX:PATH=${REXTENSION_WORKING_DIR}/${CMAKE_CONFIGURATION} \
		-DR_HOME=${R_HOME} \
		-DR_INCLUDE_DIR=${R_INCLUDE} \
		-DR_LIB_DIR=${R_LIB_DIR} \
		-DEXTENSION_API_INCLUDE_DIR=${ENL_ROOT}/extension-host/include \
		-DREXTENSION_INCLUDE_DIR=${REXTENSION_HOME}/include \
		-DREXTENSION_WORKING_DIR=${REXTENSION_WORKING_DIR} \
		-DREXTENSION_SRC_DIR=${REXTENSION_SRC_DIR} \
		-DENL_ROOT=${ENL_ROOT} \
		-DPLATFORM=linux \
		${REXTENSION_SRC_DIR}
	cmake --build ${REXTENSION_WORKING_DIR} --config ${CMAKE_CONFIGURATION} --target install

	# Check the exit code of the compiler and exit appropriately so that build will fail.
	#
	check_exit_code "Success: Built libRExtension.so.1.2" "Error: Failed to build RExtension"

	# Move the generated libs to configuration folder
	#
	mkdir -p ${CMAKE_CONFIGURATION}
	mv -f libRExtension.so.1.2 ${CMAKE_CONFIGURATION}/

	cd ${CMAKE_CONFIGURATION}/
	# This will create the RExtension package with unsigned binaries, this is used for local development and non-release builds. Release
	# builds will call create-RExtension-zip.sh after the binaries have been signed and this will be included in the zip
	#
	zip ${TARGET}/R-lang-extension libRExtension.so.1.2

	check_exit_code "Success: Created R-lang-extension.zip" "Error: Failed to create zip for RExtension"
}

# Enlistment root and location of RExtension
#
SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
ENL_ROOT=${SCRIPTDIR}/../../../..
REXTENSION_HOME=${ENL_ROOT}/language-extensions/R

# Set environment variables required in Cmake
#
PACKAGES_ROOT=${ENL_ROOT}/packages
REXTENSION_SRC_DIR=${REXTENSION_HOME}/src/
REXTENSION_WORKING_DIR=${ENL_ROOT}/build-output/RExtension/linux/

DEFAULT_R_HOME=/usr/lib/R
DEFAULT_R_INCLUDE=/usr/share/R/include

# Find R_HOME from user, or set to default for building.
# Error code 1 is generic bash error.
#
if [ -z "${R_HOME}" ]; then
	if [ -d "${DEFAULT_R_HOME}" ]; then
		R_HOME=${DEFAULT_R_HOME}
	else
		echo "R_HOME is empty"
		exit 1
	fi
fi

# Find R_INCLUDE from user, or set to default for building.
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

R_LIB_DIR=${R_HOME}/lib/

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
