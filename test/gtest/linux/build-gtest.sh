#!/bin/bash

SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
ENL_ROOT=${SCRIPTDIR}/../../../
PACKAGES_ROOT=${ENL_ROOT}/packages/

cd /usr/src/gtest
cmake -DBUILD_SHARED_LIBS=ON .
make

exit $?
