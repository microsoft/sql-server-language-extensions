cmake_minimum_required (VERSION 3.5)

# this is what the final executable is going to be named
#
project(RExtension-test VERSION 1.0 LANGUAGES CXX)

# All string comparisons are CASE SENSITIVE in CMAKE. Make all strings lower before comparisons!
#
string(TOLOWER ${PLATFORM} PLATFORM)
string(TOLOWER ${CMAKE_CONFIGURATION} CMAKE_CONFIGURATION)

file(TO_CMAKE_PATH ${ENL_ROOT}/language-extensions/R/test REXTENSIONTEST_HOME)
file(TO_CMAKE_PATH ${REXTENSIONTEST_HOME}/src REXTENSIONTEST_SRC_DIR)

# C++ unit tests code; only these files are compiled-in
#
file(GLOB REXTENSIONTEST_SOURCE_FILES ${REXTENSIONTEST_SRC_DIR}/*.cpp ${REXTENSIONTEST_SRC_DIR}/${PLATFORM}/*.cpp)

add_executable(RExtension-test
  ${REXTENSIONTEST_SOURCE_FILES}
)

if(${PLATFORM} STREQUAL linux)
	target_compile_options(RExtension-test PRIVATE -Wall -Wextra -g -O2 -fPIC -Werror -std=c++14 -Wno-unused-parameter)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,'$ORIGIN:${CMAKE_INSTALL_PREFIX}' -Wl,--no-as-needed -Wl,--export-dynamic")

	# Find and add linkage to dl library for dynamic loading
	#
	set(USR_LIB_PATH "/usr/local/lib")
	find_library(DL dl ${USR_LIB_PATH})
	target_link_libraries(RExtension-test ${DL})

	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads REQUIRED)
	target_link_libraries(RExtension-test Threads::Threads)

elseif(${PLATFORM} STREQUAL windows)
	target_compile_options(RExtension-test PRIVATE -Wall -Wextra -g -O2 -std=c++14 -Wno-unused-parameter)

	# Set the DLLEXPORT variable to export symbols
	#
	add_definitions(-DWIN_EXPORT -D_WIN64 -D_WINDOWS)
	target_compile_definitions(RExtension-test PRIVATE WIN_EXPORT)
endif()

file(TO_CMAKE_PATH ${ENL_ROOT}/language-extensions/R/test/include/ REXTENSIONTEST_INCLUDE_DIR)
file(TO_CMAKE_PATH ${ENL_ROOT}/.build/RExtension-test/${PLATFORM}/ REXTENSIONTEST_WORKING_DIR)
file(TO_CMAKE_PATH ${REXTENSIONTEST_WORKING_DIR}/${CMAKE_CONFIGURATION} REXTENSIONTEST_INSTALL_DIR)

file(TO_CMAKE_PATH ${ENL_ROOT}/.build/googletest/${PLATFORM} GTEST_HOME)
file(TO_CMAKE_PATH ${GTEST_HOME}/googletest-src/googletest/include GTEST_INCLUDE_DIR)
file(TO_CMAKE_PATH ${GTEST_HOME}/lib/libgtest.a GTEST_LIB)

# This is not a standard include path so test projects need
# to add it explicitly
#
include_directories(
	"${GTEST_INCLUDE_DIR}"
	"${REXTENSIONTEST_INCLUDE_DIR}"
)

# The string comparison is case sensitive
#
if(${CMAKE_CONFIGURATION} STREQUAL debug)
	add_definitions(-D_DEBUG)
endif()

target_link_libraries(RExtension-test
  ${GTEST_LIB}
)

install(TARGETS RExtension-test DESTINATION ${REXTENSIONTEST_INSTALL_DIR})
