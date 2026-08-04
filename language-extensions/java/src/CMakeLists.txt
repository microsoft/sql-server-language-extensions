# CMakeLists.txt

cmake_minimum_required (VERSION 3.5)

set(JAVAEXTENSION_VERSION_MAJOR "1")
set(JAVAEXTENSION_VERSION_MINOR "0")
set(JAVAEXTENSION_VERSION ${JAVAEXTENSION_VERSION_MAJOR}.${JAVAEXTENSION_VERSION_MINOR})

# this is what the final library is going to be named
#
project(JavaExtension VERSION ${JAVAEXTENSION_VERSION} LANGUAGES CXX)

# All string comparisons are CASE SENSITIVE in CMAKE. Make all strings lower before comparisons!
#
string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
string(TOLOWER ${PLATFORM} PLATFORM)

# To build shared libraries in Windows, we set CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS to TRUE.
# See https://cmake.org/cmake/help/v3.4/variable/CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS.html
# See https://blog.kitware.com/create-dlls-on-windows-without-declspec-using-new-cmake-export-all-feature/
#
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

file(TO_CMAKE_PATH ${ENL_ROOT}/language-extensions/java JAVAEXTENSION_HOME)
file(TO_CMAKE_PATH ${JAVAEXTENSION_HOME}/src JAVAEXTENSION_SRC_DIR)
file(TO_CMAKE_PATH ${JAVAEXTENSION_HOME}/include JAVAEXTENSION_INCLUDE_DIR)
file(TO_CMAKE_PATH ${ENL_ROOT}/extension-host/include EXTENSION_API_INCLUDE_DIR)

# C++ code; only these files are compiled-in
#
# C++ code; only these files are compiled-in
#
file(GLOB JAVAEXTENSION_SOURCE_FILES ${JAVAEXTENSION_SRC_DIR}/*.cpp ${JAVAEXTENSION_SRC_DIR}/${PLATFORM}/*.cpp)

# RHEL 9 ABI floor shims. Required because libstdc++.a from the Ubuntu 24.04
# toolchain references glibc symbols newer than 2.34. See
# language-extensions/common/linux/AbiFloorCompat_linux.cpp.
#
if (${PLATFORM} STREQUAL linux)
	list(APPEND JAVAEXTENSION_SOURCE_FILES ${ENL_ROOT}/language-extensions/common/linux/AbiFloorCompat_linux.cpp)
endif()

# Create the target library
#
add_library(JavaExtension SHARED
	${JAVAEXTENSION_SOURCE_FILES}
)

set_target_properties(JavaExtension
	PROPERTIES
	VERSION ${JAVAEXTENSION_VERSION}
	SOVERSION ${JAVAEXTENSION_VERSION_MAJOR}
)

if (${PLATFORM} STREQUAL linux)
	if(EXISTS "${JAVA_HOME}/lib/server")
		file(TO_CMAKE_PATH ${JAVA_HOME}/lib/server JRE_ROOT)
	else()
		file(TO_CMAKE_PATH ${JAVA_HOME}/jre/lib/amd64/server JRE_ROOT)
	endif()
	file(TO_CMAKE_PATH ${JAVA_HOME}/include JDK_INCLUDE)
	file(TO_CMAKE_PATH ${JDK_INCLUDE}/linux JDK_INCLUDE_LINUX)
		
	set(CXXFLAGS
			-O2 
			-std=c++11 
			-DSTRSAFE_LOCALE_FUNCTIONS 
			-D_M_AMD64 
			-fshort-wchar 
			-fpermissive 
			-fPIC 
			-Woverflow 
			-ggdb
			-Wno-unused-parameter)

	target_compile_options(JavaExtension PRIVATE ${CXXFLAGS})
	
	# libgcc is deliberately NOT linked statically. -static-libgcc pulls libgcc_eh.a's
	# unwinder into this .so, where its _dl_find_object reference binds to a hidden,
	# non-preemptible stub. GCC 13's _Unwind_Find_FDE has no runtime fallback when it
	# is built against _dl_find_object: the dl_iterate_phdr path is #else'd out, so a
	# stub that cannot answer the query makes it return NULL for every PC and the
	# first throw hits _URC_FATAL_PHASE1_ERROR -> std::terminate(). Linking libgcc
	# dynamically keeps the real unwinder in the host libgcc_s.so.1. RHEL 9's
	# libgcc_s.so.1 exports only GCC_* symbol versions, so the glibc floor is
	# unaffected.
	#
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -shared -static-libstdc++ -Wl,--exclude-libs,ALL -Wl,-rpath,${JRE_ROOT}:/opt/mssql-extensibility/lib")
	target_link_libraries(JavaExtension dl)
	set(ADDITIONAL_INCLUDES ${JDK_INCLUDE} ${JDK_INCLUDE_LINUX})
	
elseif(${PLATFORM} STREQUAL windows)
	add_definitions(-DWIN_EXPORT -D_WIN64 -D_WINDOWS)
	target_compile_options(JavaExtension PRIVATE /EHsc /MTd /W2 /c)

	# Set the DLLEXPORT variable to export symbols
	#
	target_compile_definitions(JavaExtension PRIVATE WIN_EXPORT)
	
	if (${CMAKE_BUILD_TYPE} STREQUAL debug)
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")
		set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /DEBUG:FULL /OPT:REF /OPT:ICF")
	else()
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
		set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG:FULL /OPT:REF /OPT:ICF")
	endif()
	file(TO_CMAKE_PATH ${JAVA_HOME}\\include JAVA_INCLUDE)
	file(TO_CMAKE_PATH ${JAVA_INCLUDE}\\win32 JAVA_INCLUDE_WIN32)
	
	set(ADDITIONAL_INCLUDES ${JAVA_INCLUDE} ${JAVA_INCLUDE_WIN32})
endif()

if (${CMAKE_BUILD_TYPE} STREQUAL debug)
	add_definitions(-DDEBUG)
else()
	add_definitions(-DNDEBUG)
endif()

target_include_directories(JavaExtension
	PRIVATE ${EXTENSION_API_INCLUDE_DIR} ${JAVAEXTENSION_INCLUDE_DIR} ${ADDITIONAL_INCLUDES} 
)

install(TARGETS JavaExtension DESTINATION "${JAVAEXTENSION_WORKING_DIR}" LIBRARY NAMELINK_ONLY)
