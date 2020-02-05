cmake_minimum_required (VERSION 3.5)

# this is what the final executable is going to be named
#
project(RExtension-test VERSION 1.0 LANGUAGES CXX)

# All string comparisons are CASE SENSITIVE in CMAKE. Make all strings lower before comparisons!
#
string(TOLOWER ${CMAKE_CXX_COMPILER_ID} CMAKE_CXX_COMPILER_ID_STRING)
string(TOLOWER ${CMAKE_CONFIGURATION} CMAKE_CONFIGURATION)

file(TO_CMAKE_PATH ${ENL_ROOT}/language-extensions/R/test REXTENSIONTEST_HOME)
file(TO_CMAKE_PATH ${REXTENSIONTEST_HOME}/src REXTENSIONTEST_SRC_DIR)

# C++ unit tests code; only these files are compiled-in
#
file(GLOB REXTENSIONTEST_SOURCE_FILES ${REXTENSIONTEST_SRC_DIR}/*.cpp ${REXTENSIONTEST_SRC_DIR}/${PLATFORM}/*.cpp)

add_executable(RExtension-test
  ${REXTENSIONTEST_SOURCE_FILES}
)

if(${CMAKE_CXX_COMPILER_ID_STRING} STREQUAL gnu)
	# G++
	#
	target_compile_options(RExtension-test PRIVATE -Wall -Wextra -g -O2 -fPIC -Werror -std=c++11)

	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,'$ORIGIN' -Wl,--no-as-needed -Wl,--export-dynamic")
	set(USR_LIB_PATH "/usr/local/lib")
	set(PLATFORM "linux")

	find_library(DL dl ${USR_LIB_PATH})
	target_link_libraries(RExtension-test ${DL})

	file(TO_CMAKE_PATH ${ENL_ROOT}/.build/googletest/${PLATFORM} GTEST_HOME)
	file(TO_CMAKE_PATH ${GTEST_HOME}/googletest-src/googletest/include GTEST_INCLUDE_DIR)
	file(TO_CMAKE_PATH ${GTEST_HOME}/lib/libgtest.a GTEST)

	# Need to set pthread flag on so that linker can find reference to pthread functions
	#
	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads REQUIRED)
	target_link_libraries(RExtension-test Threads::Threads)

elseif(${CMAKE_CXX_COMPILER_ID_STRING} STREQUAL msvc)
	# MSVC
	#
	add_definitions(-DWIN_EXPORT -D_WIN64 -D_WINDOWS)
	target_compile_options(RExtension-test PRIVATE /EHsc /W2 /c /MDd)
	# Set the DLLEXPORT variable to export symbols
	target_compile_definitions(RExtension-test PRIVATE WIN_EXPORT)
	set(PLATFORM "windows")
	file(TO_CMAKE_PATH ${ENL_ROOT}/packages/Microsoft.googletest.v140.windesktop.msvcstl.dyn.rt-dyn.1.8.0 GTEST_HOME)
	file(TO_CMAKE_PATH ${GTEST_HOME}/build/native/include GTEST_INCLUDE_DIR)
	file(TO_CMAKE_PATH ${GTEST_HOME}/lib/native/v140/windesktop/msvcstl/dyn/rt-dyn/x64/${CMAKE_CONFIGURATION} GTEST_LIB_PATH)
	find_library(GTEST gtest ${GTEST_LIB_PATH})
endif()

file(TO_CMAKE_PATH ${ENL_ROOT}/language-extensions/R/test/include/ REXTENSIONTEST_INCLUDE_DIR)
file(TO_CMAKE_PATH ${ENL_ROOT}/.build/RExtension-test/${PLATFORM}/ REXTENSIONTEST_WORKING_DIR)
file(TO_CMAKE_PATH ${REXTENSIONTEST_WORKING_DIR}/${CMAKE_CONFIGURATION} REXTENSIONTEST_INSTALL_DIR)

# this is not a standard include path so test projects need
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
	${GTEST}
)

install(TARGETS RExtension-test DESTINATION ${REXTENSIONTEST_INSTALL_DIR})
