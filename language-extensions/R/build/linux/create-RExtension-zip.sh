#!/bin/bash

function check_exit_code {
    EXIT_CODE=$?
    if [ ${EXIT_CODE} -eq 0 ]; then
        echo "Success: Created zip for $1 config"
    else
        echo "Error: Failed to create zip for $1 config"
        exit ${EXIT_CODE}
    fi
}

SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
ENL_ROOT=${SCRIPTDIR}/../../../..

GNU_BUILD_CONFIGURATION=debug
BUILD_OUTPUT=${ENL_ROOT}/build-output/RExtension/linux/${GNU_BUILD_CONFIGURATION}

mkdir -p ${BUILD_OUTPUT}/packages
cd ${BUILD_OUTPUT}
zip packages/R-lang-extension libRExtension.so.1.0
check_exit_code  ${GNU_BUILD_CONFIGURATION}

GNU_BUILD_CONFIGURATION=release
BUILD_OUTPUT=${ENL_ROOT}/build-output/RExtension/linux/${GNU_BUILD_CONFIGURATION}

mkdir -p ${BUILD_OUTPUT}/packages
cd ${BUILD_OUTPUT}
zip packages/R-lang-extension libRExtension.so.1.0
check_exit_code ${GNU_BUILD_CONFIGURATION}

exit $?
