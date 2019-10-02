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

    # Output directory and output dll name
    TARGET=${EnlRoot}.build/R-extension/target/${CMAKE_CONFIGURATION}/
    # Create the output directories
    mkdir -p ${TARGET}

    if ! [[ -d ${REXTENSION_WORKING_DIR} ]]; then
        mkdir -p ${REXTENSION_WORKING_DIR}
    fi
    cd ${REXTENSION_WORKING_DIR}
    # Compile
    cmake -DCMAKE_BUILD_TYPE=${CMAKE_CONFIGURATION} \
        -DR_INCLUDE_DIR=${R_HOME}include \
        -DR_LIB_DIR=${R_LIB_DIR} \
        -DREXTENSION_INCLUDE_DIR=${REXTENSION_HOME}include \
        -DREXTENSION_WORKING_DIR=${REXTENSION_WORKING_DIR} \
        -DREXTENSION_SRC_DIR=${REXTENSION_SRC_DIR} \
        ${REXTENSION_SRC_DIR}
    cmake --build ${REXTENSION_WORKING_DIR} --config ${CMAKE_CONFIGURATION} --target install

    # Check the exit code of the compiler and exit appropriately so that build will fail.
    check_exit_code "Success: Built libRextension.so.1.0" "Error: Failed to build R extension"

    # Move the generated libs to configuration folder
    mkdir -p ${CMAKE_CONFIGURATION}
    mv libRextension.so* ${CMAKE_CONFIGURATION}/

    cd ${CMAKE_CONFIGURATION}/
    # This will create the R extension package with unsigned binaries, this is used for local development and non-release builds. Release
    # builds will call create-R-extension-zip.sh after the binaries have been signed and this will be included in the zip
    zip ${TARGET}R-lang-extension libRextension.so.1.0

    check_exit_code "Success: Created R-lang-extension.zip" "Error: Failed to create zip for R extension"
}

# Enlistment root and location of Rextension
SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
EnlRoot=${SCRIPTDIR}/../../../../
REXTENSION_HOME=${EnlRoot}language-extensions/R/

# Set environment variables required in Cmake
PACKAGES_ROOT=${EnlRoot}packages/
R_HOME=${PACKAGES_ROOT}External-R.MRO-3.5.2.R.3.5.2.229/Linux/lib64/R/
REXTENSION_SRC_DIR=${REXTENSION_HOME}src/
R_LIB_DIR=${R_HOME}lib/
REXTENSION_WORKING_DIR=${EnlRoot}.build/R-extension/linux/

while [ "$1" != "" ]; do
    # Advance arg passed to build.cmd
    build $1
    shift
done;
