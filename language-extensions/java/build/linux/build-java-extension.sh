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

	# Output directory and output dll name
	#
	TARGET=${ENL_ROOT}/build-output/java-extension/target/${CMAKE_CONFIGURATION}
	TARGET_CLASSES=${TARGET}/classes
	OUTPUT_JAR=mssql-java-lang-extension-linux.jar

	# Create the output directories
	#
	mkdir -p ${TARGET}
	mkdir -p ${TARGET_CLASSES}

	# Compile java files into class files
	#
	pushd ${ENL_ROOT}/language-extensions/java/sdk/src/java/main/java/com/microsoft/sqlserver/javalangextension/
	ls -d "$PWD"/*.java > ${TARGET}/sources.txt
	${JAVA_HOME}/bin/javac  -d ${TARGET_CLASSES} @${TARGET}/sources.txt

	# Create the mssql-java-lang-extension-linux.jar file
	#
	${JAVA_HOME}/bin/jar cvf ${TARGET}/${OUTPUT_JAR} -C ${TARGET_CLASSES} .

	if ! [[ -d ${JAVAEXTENSION_WORKING_DIR} ]]; then
		mkdir -p ${JAVAEXTENSION_WORKING_DIR}
	fi

	cd ${JAVAEXTENSION_WORKING_DIR}

	# Compile
	#
	cmake -DPLATFORM=Linux \
		-DENL_ROOT=${ENL_ROOT} \
		-DCMAKE_BUILD_TYPE=${CMAKE_CONFIGURATION} \
		-DJAVA_HOME=${JAVA_HOME} \
		-DJAVAEXTENSION_WORKING_DIR=${JAVAEXTENSION_WORKING_DIR} \
		${JAVAEXTENSION_HOME}/src
	cmake --build ${JAVAEXTENSION_WORKING_DIR} --config ${CMAKE_CONFIGURATION} --target install

	# Check the exit code of the compiler and exit appropriately so that build will fail.
	#
	check_exit_code "Success: Built libJavaExtension.so.1.0" "Error: Failed to build java extension"

	# Move the generated libs to configuration folder
	#
	mkdir -p ${CMAKE_CONFIGURATION}
	mv libJavaExtension.so* ${CMAKE_CONFIGURATION}/

	cd ${CMAKE_CONFIGURATION}/

	# This will create the java extension package with unsigned binaries, this is used for local development and non-release builds. release
	# builds will call create-java-extension-zip.sh after the binaries have been signed and this will be included in the zip
	#
	zip ${TARGET}/java-lang-extension libJavaExtension.so.1.0

	check_exit_code "Success: Created java-lang-extension.zip" "Error: Failed to create zip for java extension"
}

# Enlistment root and location of javaextension
#
SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
ENL_ROOT=${SCRIPTDIR}/../../../..

# Set environment variables required in Cmake
#
JAVAEXTENSION_HOME=${ENL_ROOT}/language-extensions/java
JAVAEXTENSION_WORKING_DIR=${ENL_ROOT}/build-output/java-extension/linux
DEFAULT_JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64

# Find JAVA_HOME from user, or set to default for tests.
# Error code 1 is generic bash error.
# 
if [ -z "${JAVA_HOME}" ]; then
	if [ -d "${DEFAULT_JAVA_HOME}" ]; then 
		JAVA_HOME=${DEFAULT_JAVA_HOME}
	else
		echo "JAVA_HOME is empty"
		exit 1
	fi
fi

# Build in release mode if nothing is specified
#
if [ "$1" == "" ]; then
	set -- release
fi

while [ "$1" != "" ]; do
	# Advance arg passed to build.sh
	#
	build $1
	shift
done;
