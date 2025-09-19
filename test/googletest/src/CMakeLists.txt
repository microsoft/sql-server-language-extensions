# Downloads and unpacks googletest at configure time.  Based on the instructions
# at https://github.com/google/googletest/tree/master/googletest#incorporating-into-an-existing-cmake-project

cmake_minimum_required (VERSION 3.5)

project(googletest)

set(CMAKE_CXX_STANDARD 17)

# Populate the CMakeLists.txt.in with the actual values from build
#
configure_file(${PLATFORM}/CMakeLists.txt.in googletest-download/CMakeLists.txt)

# Generate the make files
#
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" . -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
	RESULT_VARIABLE result
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)
if(result)
	message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()

# Build googletest
#

if (${PLATFORM} STREQUAL linux)
	target_compile_options(gtest PRIVATE -w)
endif()

execute_process(COMMAND "${CMAKE_COMMAND}" --build .
	RESULT_VARIABLE result
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
	message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
#
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to build. This defines
# the gtest and gtest_main targets.
#
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
				 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build)
