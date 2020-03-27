cmake_minimum_required (VERSION 3.5)

# this is what the final executable is going to be named
project(pythonextension-test VERSION 1.0 LANGUAGES CXX)

# All string comparisons are CASE SENSITIVE in CMAKE. Make all strings lower before comparisons!
#
string(TOLOWER ${CMAKE_CONFIGURATION} CMAKE_CONFIGURATION)

file(TO_CMAKE_PATH ${ENL_ROOT}/extension-host EXTENSION_API_HOME)
file(TO_CMAKE_PATH ${ENL_ROOT}/language-extensions/python PYTHONEXTENSION_HOME)
file(TO_CMAKE_PATH ${ENL_ROOT}/language-extensions/python/test PYTHONEXTENSION_TEST_HOME)
file(TO_CMAKE_PATH ${PYTHONEXTENSION_TEST_HOME}/src PYTHONEXTENSION_TEST_SRC_DIR)

file(TO_CMAKE_PATH ${ENL_ROOT}/build-output/pythonextension/${PLATFORM} PYTHONEXTENSION_WORKING_DIR)
file(TO_CMAKE_PATH ${ENL_ROOT}/build-output/pythonextension-test/${PLATFORM} PYTHONEXTENSION_TEST_WORKING_DIR)
file(TO_CMAKE_PATH ${PYTHONEXTENSION_TEST_WORKING_DIR}/${CMAKE_CONFIGURATION} PYTHONEXTENSION_TEST_INSTALL_DIR)

find_library(PYTHONEXTENSION_LIB PythonExtension ${PYTHONEXTENSION_WORKING_DIR}/${CMAKE_CONFIGURATION})

# C++ unit tests code; only these files are compiled-in
#
file(GLOB PYTHONEXTENSION_TEST_SOURCE_FILES ${PYTHONEXTENSION_TEST_SRC_DIR}/*.cpp ${PYTHONEXTENSION_TEST_SRC_DIR}/${PLATFORM}/*.cpp)

add_executable(pythonextension-test
  ${PYTHONEXTENSION_TEST_SOURCE_FILES}
)

if (${PLATFORM} STREQUAL linux)
	target_compile_options(pythonextension-test PRIVATE -Wall -Wextra -g -O2 -Werror -fPIC -std=c++11)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,'$ORIGIN' -Wl,--no-as-needed -Wl,--export-dynamic")
	
	set(USR_LIB_PATH "/usr/local/lib")

	file(TO_CMAKE_PATH ${ENL_ROOT}/build-output/googletest/${PLATFORM} GTEST_HOME)
	file(TO_CMAKE_PATH ${GTEST_HOME}/googletest-src/googletest/include GTEST_INCLUDE_DIR)
	file(TO_CMAKE_PATH ${GTEST_HOME}/lib/libgtest.a GTEST_LIB)

	find_library(DL dl ${USR_LIB_PATH})
	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads REQUIRED)
	target_link_libraries(pythonextension-test
		${DL}
		Threads::Threads
	)

	file(TO_CMAKE_PATH ${INCLUDE_ROOT}/python3.7 PYTHON_INCLUDE)
	file(TO_CMAKE_PATH ${INCLUDE_ROOT}/boost BOOST_INCLUDE)
	
	set(ADDITIONAL_INCLUDES ${BOOST_INCLUDE} ${PYTHON_INCLUDE})
	
elseif(${PLATFORM} STREQUAL windows)

	add_definitions(-DWIN_EXPORT -D_WIN64 -D_WINDOWS -D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
	
	# Compile options /W2 for severe warnings
	#
	set(COMPILE_OPTIONS /W2)
	
	find_library(PYTHON_LIB python37 ${PYTHONHOME}/libs)
	
	file(TO_CMAKE_PATH ${ENL_ROOT}/packages/Microsoft.googletest.v140.windesktop.msvcstl.dyn.rt-dyn.1.8.0 GTEST_HOME)
	file(TO_CMAKE_PATH ${GTEST_HOME}/build/native/include GTEST_INCLUDE_DIR)
	file(TO_CMAKE_PATH ${GTEST_HOME}/lib/native/v140/windesktop/msvcstl/dyn/rt-dyn/x64/${CMAKE_CONFIGURATION} GTEST_LIB_PATH)
	find_library(GTEST_LIB gtest ${GTEST_LIB_PATH})

	# Build static lib to test with 
	#
	if (${CMAKE_CONFIGURATION} STREQUAL debug)
		set(COMPILE_OPTIONS ${COMPILE_OPTIONS} /MTd)
	else()
		set(COMPILE_OPTIONS ${COMPILE_OPTIONS} /MT)
	endif()

	# Set the DLLEXPORT variable to export symbols
	#
	target_compile_definitions(pythonextension-test PRIVATE ${COMPILE_OPTIONS})

	file(TO_CMAKE_PATH ${PYTHONHOME}/include PYTHON_INCLUDE)
	file(TO_CMAKE_PATH ${BOOST_ROOT}/include BOOST_INCLUDE)

	set(ADDITIONAL_INCLUDES ${BOOST_INCLUDE} ${PYTHON_INCLUDE})
endif()

# this is not a standard include path so test projects need
# to add it explicitly
#
include_directories(
	"${GTEST_INCLUDE_DIR}"
	"${EXTENSION_API_HOME}/include"
	"${PYTHONEXTENSION_HOME}/include"
	"${PYTHONEXTENSION_TEST_HOME}/include"
)

# The string comparison is case sensitive
#
if (${CMAKE_CONFIGURATION} STREQUAL debug)
	add_definitions(-DDEBUG)
else()
	add_definitions(-DNDEBUG)
endif()

target_link_libraries(pythonextension-test
	${GTEST_LIB}
	${PYTHONEXTENSION_LIB}
)

target_include_directories(pythonextension-test
	PRIVATE ${ADDITIONAL_INCLUDES} 
)

install(TARGETS pythonextension-test DESTINATION ${PYTHONEXTENSION_TEST_INSTALL_DIR})
