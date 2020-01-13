cmake_minimum_required (VERSION 3.5)

# this is what the final executable is going to be named
project(Rextension-test VERSION 1.0 LANGUAGES CXX)

# All string comparisons are CASE SENSITIVE in CMAKE. Make all strings lower before comparisons!
#
string(TOLOWER ${CMAKE_CXX_COMPILER_ID} CMAKE_CXX_COMPILER_ID_STRING)
string(TOLOWER ${CMAKE_CONFIGURATION} CMAKE_CONFIGURATION)

# C++ unit tests code; only these files are compiled-in
set(REXTENSIONTEST_SOURCE_FILES
	main.cpp
	RextensionApiTest.cpp
)
if (${CMAKE_CXX_COMPILER_ID_STRING} STREQUAL gnu)
	set(REXTENSIONTEST_SOURCE_FILES
		${REXTENSIONTEST_SOURCE_FILES}
		Utilities_linux.cpp)
elseif(${CMAKE_CXX_COMPILER_ID_STRING} STREQUAL msvc)
	set(REXTENSIONTEST_SOURCE_FILES
		${REXTENSIONTEST_SOURCE_FILES}
		Utilities_win.cpp)
endif()

add_executable(Rextension-test
  ${REXTENSIONTEST_SOURCE_FILES}
)

if(${CMAKE_CXX_COMPILER_ID_STRING} STREQUAL gnu)
	# G++
	target_compile_options(Rextension-test PRIVATE -Wall -Wextra -g -O2 -fPIC -Werror -std=c++11)
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-rpath,'$ORIGIN' -Wl,--no-as-needed -Wl,--export-dynamic")
	set(USR_LIB_PATH "/usr/local/lib")
	set(PLATFORM "linux")
	file(TO_CMAKE_PATH /usr/include/gtest/ GTEST_INCLUDE_DIR)
	file(TO_CMAKE_PATH /usr/src/gtest/ GTEST_LIB_PATH)
	find_library(DL dl ${USR_LIB_PATH})
	target_link_libraries(Rextension-test
		${DL}
	)
elseif(${CMAKE_CXX_COMPILER_ID_STRING} STREQUAL msvc)
	# MSVC
	add_definitions(-DWIN_EXPORT -D_WIN64 -D_WINDOWS)
	target_compile_options(Rextension-test PRIVATE /EHsc /W2 /c /MDd)
	# Set the DLLEXPORT variable to export symbols
	target_compile_definitions(Rextension-test PRIVATE WIN_EXPORT)
	set(PLATFORM "windows")
	file(TO_CMAKE_PATH ${ENL_ROOT}\\packages\\Microsoft.googletest.v140.windesktop.msvcstl.dyn.rt-dyn.1.8.0 GTEST_HOME)
	file(TO_CMAKE_PATH ${GTEST_HOME}\\build\\native\\include GTEST_INCLUDE_DIR)
	file(TO_CMAKE_PATH ${GTEST_HOME}\\lib\\native\\v140\\windesktop\\msvcstl\\dyn\\rt-dyn\\x64\\${CMAKE_CONFIGURATION} GTEST_LIB_PATH)
endif()

file(TO_CMAKE_PATH ${ENL_ROOT}\\language-extensions\\R\\test\\include\\ REXTENSIONTEST_INCLUDE_DIR)
file(TO_CMAKE_PATH ${ENL_ROOT}\\.build\\Rextension-test\\${PLATFORM}\\ REXTENSIONTEST_WORKING_DIR)
file(TO_CMAKE_PATH ${REXTENSIONTEST_WORKING_DIR}\\${CMAKE_CONFIGURATION} REXTENSIONTEST_INSTALL_DIR)

# this is not a standard include path so test projects need
# to add it explicitly
include_directories(
	"${GTEST_INCLUDE_DIR}"
	"${REXTENSIONTEST_INCLUDE_DIR}"
)

# The string comparison is case sensitive
if(${CMAKE_CONFIGURATION} STREQUAL debug)
	add_definitions(-D_DEBUG)
endif()

find_library(GTEST gtest ${GTEST_LIB_PATH})

target_link_libraries(Rextension-test
	${GTEST}
)

install(TARGETS Rextension-test DESTINATION ${REXTENSIONTEST_INSTALL_DIR})
