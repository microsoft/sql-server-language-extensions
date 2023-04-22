# CMakeLists.txt

cmake_minimum_required (VERSION 3.5)

set(REXTENSION_VERSION_MAJOR "1")
set(REXTENSION_VERSION_MINOR "2")
set(REXTENSION_VERSION ${REXTENSION_VERSION_MAJOR}.${REXTENSION_VERSION_MINOR})

# this is what the final library is going to be named
#
project(RExtension VERSION ${REXTENSION_VERSION} LANGUAGES CXX)

# All string comparisons are CASE SENSITIVE in CMAKE. Make all strings lower before comparisons!
#
string(TOLOWER ${CMAKE_CONFIGURATION} CMAKE_CONFIGURATION)
string(TOLOWER ${PLATFORM} PLATFORM)

# To build shared libraries in Windows, we set CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS to TRUE.
# See https://cmake.org/cmake/help/v3.4/variable/CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS.html
# See https://blog.kitware.com/create-dlls-on-windows-without-declspec-using-new-cmake-export-all-feature/
#
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

file(TO_CMAKE_PATH ${ENL_ROOT}/language-extensions/R REXTENSION_HOME)
file(TO_CMAKE_PATH ${REXTENSION_HOME}/src REXTENSION_SRC_DIR)

# C++ code; only these files are compiled-in
#
file(GLOB REXTENSION_SOURCE_FILES
	${REXTENSION_SRC_DIR}/*.cpp
	${REXTENSION_HOME}/common/src/*.cpp
	${REXTENSION_SRC_DIR}/${PLATFORM}/*.cpp)

# Create the target library
#
add_library(RExtension SHARED
	${REXTENSION_SOURCE_FILES}
)

set_target_properties(RExtension
	PROPERTIES
	VERSION ${REXTENSION_VERSION}
	SOVERSION ${REXTENSION_VERSION_MAJOR}
)

if (${PLATFORM} STREQUAL linux)
	target_compile_options(RExtension PRIVATE -Wall -Wextra -g -fPIC -Werror
		-std=c++17 -Wno-unused-parameter)
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--as-needed \
		-Wl,--export-dynamic -fopenmp -Wl,-rpath,'${ORIGIN}'")
	set(CMAKE_FIND_LIBRARY_SUFFIXES "${CMAKE_FIND_LIBRARY_SUFFIXES};.so.1;.so.3;.so.5")
	set(USR_LIB_PATH "/usr/lib/x86_64-linux-gnu")

	# Find dependent libraries
	#
	find_library(PCRE_LIB pcre ${USR_LIB_PATH})
	find_library(LZMA_LIB lzma ${USR_LIB_PATH})
	find_library(BZ2_LIB bz2 ${USR_LIB_PATH})
	find_library(Z_LIB z ${USR_LIB_PATH})
	find_library(RT_LIB rt ${USR_LIB_PATH})
	find_library(DL_LIB dl ${USR_LIB_PATH})
	find_library(M_LIB m ${USR_LIB_PATH})
	target_link_libraries(RExtension ${PCRE_LIB} ${LZMA_LIB} ${BZ2_LIB} ${Z_LIB}
		${RT_LIB} ${DL_LIB} ${M_LIB}
	)

	# Define base paths
	#
	file(TO_CMAKE_PATH ${R_HOME}/lib R_LIB_DIR)
	file(TO_CMAKE_PATH ${R_HOME}/library/RInside RINSIDE_HOME)
	file(TO_CMAKE_PATH ${RINSIDE_HOME}/lib/libRInside.a RINSIDE_LIB)

elseif(${PLATFORM} STREQUAL windows)
	target_compile_options(RExtension PRIVATE -Wall -Wextra -g -std=c++17
		-Wno-unused-parameter -Wno-cast-function-type)
	add_definitions(-DWIN_EXPORT -D_WIN64 -D_WINDOWS)
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--as-needed \
		-Wl,--export-all-symbols -fopenmp -Wl,-rpath,'${ORIGIN}'")

	# Set the DLLEXPORT variable to export symbols
	#
	target_compile_definitions(RExtension PRIVATE WIN_EXPORT)

	# Define base paths
	#
	file(TO_CMAKE_PATH ${R_HOME}/bin/x64 R_LIB_DIR)
	file(TO_CMAKE_PATH ${ENL_ROOT}/build-output/RExtension/windows REXTENSION_WORKING_DIR)
	file(TO_CMAKE_PATH ${R_HOME}/library/RInside RINSIDE_HOME)
	file(TO_CMAKE_PATH ${RINSIDE_HOME}/lib/x64/libRInside.a RINSIDE_LIB)
	file(TO_CMAKE_PATH ${R_HOME}/include R_INCLUDE_DIR)
endif()

file(TO_CMAKE_PATH ${ENL_ROOT}/build-output/RExtension/${PLATFORM} REXTENSION_WORKING_DIR)
file(TO_CMAKE_PATH ${R_HOME}/library/Rcpp/include RCPP_INCLUDE_DIR)
file(TO_CMAKE_PATH ${RINSIDE_HOME}/include RINSIDE_INCLUDE_DIR)
file(TO_CMAKE_PATH ${REXTENSION_HOME}/include REXTENSION_INCLUDE_DIR)
file(TO_CMAKE_PATH ${ENL_ROOT}/extension-host/include EXTENSION_API_INCLUDE_DIR)
file(TO_CMAKE_PATH ${REXTENSION_HOME}/common/include REXTENSION_COMMON_INCLUDE)

target_include_directories(RExtension
	PRIVATE ${REXTENSION_INCLUDE_DIR} ${R_INCLUDE_DIR}
	${EXTENSION_API_INCLUDE_DIR} ${RCPP_INCLUDE_DIR} ${RINSIDE_INCLUDE_DIR}
	${REXTENSION_COMMON_INCLUDE}
)

if (${CMAKE_CONFIGURATION} STREQUAL debug)
	add_definitions(-D_DEBUG)
else()
	# Do optimizations provided by the -O2 flag only in release
	#
	target_compile_options(RExtension PRIVATE -O2)
endif()

find_library(R_LIB R ${R_LIB_DIR})

target_link_libraries(RExtension
	${R_LIB}
	${RINSIDE_LIB}
	# stdc++fs needed for filesystem
	#
	stdc++fs
)

install(TARGETS RExtension DESTINATION "${REXTENSION_WORKING_DIR}" LIBRARY NAMELINK_ONLY)
