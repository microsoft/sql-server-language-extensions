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


function build {
	BUILD_CONFIGURATION=$1
	
	SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
	ENL_ROOT=${SCRIPTDIR}/../../../..

	BUILD_OUTPUT=${ENL_ROOT}/build-output/java-extension/linux/${BUILD_CONFIGURATION}

	mkdir -p ${BUILD_OUTPUT}/packages
	cd ${BUILD_OUTPUT}
	zip packages/java-lang-extension-linux libJavaExtension.so.1.0
	check_exit_code  ${BUILD_CONFIGURATION}
}

# Build in release mode if nothing is specified
#
if [ "$1" == "" ]; then
	set -- release
fi

while [ "$1" != "" ]; do
	# Advance arg passed to this script
	#
	build $1
	shift
done;

exit $?
