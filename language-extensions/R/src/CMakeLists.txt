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
string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
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

# RHEL 9 ABI floor shims. Required because libstdc++.a and libgcc.a from the
# Ubuntu 24.04 toolchain reference glibc symbols newer than 2.34. See
# language-extensions/common/linux/AbiFloorCompat_linux.cpp.
#
if (${PLATFORM} STREQUAL linux)
	list(APPEND REXTENSION_SOURCE_FILES ${ENL_ROOT}/language-extensions/common/linux/AbiFloorCompat_linux.cpp)
endif()

# Create the target library
#
add_library(RExtension SHARED
	${REXTENSION_SOURCE_FILES}
)

# R links libstdc++ dynamically, so RHEL9's 3.4.29 copy will not provide
# std::ios_base_library_init(). Ask the shared compat unit to define it.
#
if (${PLATFORM} STREQUAL linux)
	target_compile_definitions(RExtension PRIVATE ABI_FLOOR_PROVIDE_IOS_BASE_LIBRARY_INIT)
endif()

set_target_properties(RExtension
	PROPERTIES
	VERSION ${REXTENSION_VERSION}
	SOVERSION ${REXTENSION_VERSION_MAJOR}
)

if (${PLATFORM} STREQUAL linux)
	target_compile_options(RExtension PRIVATE -Wall -Wextra -g -fPIC -Werror
		-std=c++17 -Wno-unused-parameter -Wno-cast-function-type -Wno-format-overflow 
		-Wno-deprecated-copy -Wno-sign-conversion -Wno-conversion -Wno-maybe-uninitialized)
	# RHEL9 ABI floor (GLIBCXX <= 3.4.29): GCC 13 emits a reference to
	# std::ios_base_library_init()@GLIBCXX_3.4.32 into any translation unit that
	# includes <iostream>. The RExtension sources no longer include it directly,
	# but the Rcpp and RInside headers do, so the reference cannot be removed at
	# the source level.
	#
	# libstdc++ is deliberately NOT linked statically here. libR.so already links
	# libstdc++.so.6, so a static copy would put two C++ runtimes in one process
	# and the R extension aborts (SIGABRT) part-way through a script execution.
	# -Wl,-z,nodelete does not help: the fault is duplication, not unload order.
	# The single offending symbol is instead defined locally in
	# language-extensions/common/linux/AbiFloorCompat_linux.cpp.
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--as-needed \
		-Wl,--export-dynamic -fopenmp -Wl,-rpath,'${ORIGIN}'")
	set(CMAKE_FIND_LIBRARY_SUFFIXES "${CMAKE_FIND_LIBRARY_SUFFIXES};.so.1;.so.3;.so.5")
	set(USR_LIB_PATH "/usr/lib64")

	# Find dependent libraries with fallback paths
	#
	find_library(PCRE_LIB NAMES pcre libpcre PATHS ${USR_LIB_PATH} /usr/lib /lib /usr/lib/x86_64-linux-gnu NO_DEFAULT_PATH)
	find_library(LZMA_LIB NAMES lzma liblzma PATHS ${USR_LIB_PATH} /usr/lib /lib /usr/lib/x86_64-linux-gnu NO_DEFAULT_PATH)
	find_library(BZ2_LIB NAMES bz2 libbz2 PATHS ${USR_LIB_PATH} /usr/lib /lib /usr/lib/x86_64-linux-gnu NO_DEFAULT_PATH)
	find_library(Z_LIB NAMES z libz PATHS ${USR_LIB_PATH} /usr/lib /lib /usr/lib/x86_64-linux-gnu NO_DEFAULT_PATH)
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

if (${CMAKE_BUILD_TYPE} STREQUAL debug)
	add_definitions(-D_DEBUG)
else()
	# Do optimizations provided by the -O2 flag only in release
	#
	target_compile_options(RExtension PRIVATE -O2)
endif()

if (${PLATFORM} STREQUAL windows)
	find_file(R_LIB R.dll PATHS ${R_LIB_DIR} NO_DEFAULT_PATH)
else()
	find_library(R_LIB R ${R_LIB_DIR})
endif()

target_link_libraries(RExtension
	${R_LIB}
	${RINSIDE_LIB}
	# stdc++fs needed for filesystem
	#
	stdc++fs
)

install(TARGETS RExtension DESTINATION "${REXTENSION_WORKING_DIR}" LIBRARY NAMELINK_ONLY)
