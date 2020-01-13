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
		CMAKE_CONFIGURATION=debug
	fi

	# Output directory and output dll name
	#
	TARGET=${ENL_ROOT}/.build/R-extension/target/${CMAKE_CONFIGURATION}
	# Create the output directories
	mkdir -p ${TARGET}

	if ! [[ -d ${REXTENSION_WORKING_DIR} ]]; then
		mkdir -p ${REXTENSION_WORKING_DIR}
	fi
	cd ${REXTENSION_WORKING_DIR}
	# Compile
	#
	cmake -DCMAKE_CONFIGURATION=${CMAKE_CONFIGURATION} \
		-DR_INCLUDE_DIR=${R_HOME}/include \
		-DCMAKE_INSTALL_PREFIX:PATH=${REXTENSION_WORKING_DIR}/${CMAKE_CONFIGURATION} \
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
	check_exit_code "Success: Built libRextension.so.1.0" "Error: Failed to build R extension"

	# Move the generated libs to configuration folder
	#
	mkdir -p ${CMAKE_CONFIGURATION}
	mv -f libRextension.so* ${CMAKE_CONFIGURATION}/

	cd ${CMAKE_CONFIGURATION}/
	# This will create the R extension package with unsigned binaries, this is used for local development and non-release builds. Release
	# builds will call create-R-extension-zip.sh after the binaries have been signed and this will be included in the zip
	#
	zip ${TARGET}/R-lang-extension libRextension.so.1.0

	check_exit_code "Success: Created R-lang-extension.zip" "Error: Failed to create zip for R extension"
}

# Enlistment root and location of Rextension
#
SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
ENL_ROOT=${SCRIPTDIR}/../../../..
REXTENSION_HOME=${ENL_ROOT}/language-extensions/R

# Set environment variables required in Cmake
#
PACKAGES_ROOT=${ENL_ROOT}/packages
R_HOME=/opt/microsoft/ropen/3.5.2/lib64/R
REXTENSION_SRC_DIR=${REXTENSION_HOME}/src/
R_LIB_DIR=${R_HOME}/lib/
REXTENSION_WORKING_DIR=${ENL_ROOT}/.build/R-extension/linux/

while [ "$1" != "" ]; do
	# Advance arg passed to build.cmd
	#
	build $1
	shift
done;
