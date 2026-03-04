cmake_minimum_required (VERSION 3.5)

# This is what the final executable is going to be named
#
project(dotnet-core-CSharp-extension-test VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# All string comparisons are CASE SENSITIVE in CMAKE. Make all strings lower before comparisons!
#
string(TOLOWER ${PLATFORM} PLATFORM)
string(TOLOWER ${CMAKE_CONFIGURATION} CMAKE_CONFIGURATION)

file(TO_CMAKE_PATH ${ENL_ROOT}/extension-host EXTENSION_API_HOME)
file(TO_CMAKE_PATH ${ENL_ROOT}/language-extensions/dotnet-core-CSharp DOTNETCORE_CSHARP_EXTENSION_HOME)
file(TO_CMAKE_PATH ${DOTNETCORE_CSHARP_EXTENSION_HOME}/test DOTNETCORE_CSHARP_EXTENSION_TEST_HOME)
file(TO_CMAKE_PATH ${DOTNETCORE_CSHARP_EXTENSION_TEST_HOME}/src/native DOTNETCORE_CSHARP_EXTENSION_TEST_SRC_DIR)

# C++ unit tests code; only these files are compiled-in
#
file(GLOB DOTNETCORE_CSHARP_EXTENSION_TEST_SOURCE_FILES
	${DOTNETCORE_CSHARP_EXTENSION_TEST_SRC_DIR}/*.cpp)

add_executable(dotnet-core-CSharp-extension-test
	${DOTNETCORE_CSHARP_EXTENSION_TEST_SOURCE_FILES}
)

file(TO_CMAKE_PATH ${ENL_ROOT}/language-extensions/dotnet-core-CSharp/test/include/ DOTNETCORE_CSHARP_EXTENSION_TEST_INCLUDE_DIR)
file(TO_CMAKE_PATH ${ENL_ROOT}/build-output/dotnet-core-CSharp-extension-test/${PLATFORM}/ DOTNETCORE_CSHARP_EXTENSION_TEST_WORKING_DIR)
file(TO_CMAKE_PATH ${DOTNETCORE_CSHARP_EXTENSION_TEST_WORKING_DIR}/${CMAKE_CONFIGURATION} DOTNETCORE_CSHARP_EXTENSION_TEST_INSTALL_DIR)

if(WIN32)
	# Set the DLLEXPORT variable to export symbols
	#
	add_definitions(-DWIN_EXPORT -D_WIN64 -D_WINDOWS)
	target_compile_definitions(dotnet-core-CSharp-extension-test PRIVATE WIN_EXPORT)

	file(TO_CMAKE_PATH ${ENL_ROOT}/packages/Microsoft.googletest.v140.windesktop.msvcstl.dyn.rt-dyn.1.8.1.3 GTEST_HOME)
	file(TO_CMAKE_PATH ${GTEST_HOME}/build/native/include GTEST_INCLUDE_DIR)
	file(TO_CMAKE_PATH ${GTEST_HOME}/lib/native/v140/windesktop/msvcstl/dyn/rt-dyn/x64/${CMAKE_CONFIGURATION} GTEST_LIB_PATH)

	# MDd is for debug DLL and MD is for release DLL
	#
	if (${CMAKE_CONFIGURATION} STREQUAL debug)
		set(COMPILE_OPTIONS ${COMPILE_OPTIONS} /MDd)
		find_library(GTEST_LIB gtestd ${GTEST_LIB_PATH})
		set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG} /Od)
	else()
		set(COMPILE_OPTIONS ${COMPILE_OPTIONS} /MD /O2)
		find_library(GTEST_LIB gtest ${GTEST_LIB_PATH})
	endif()
else()
	# On Linux, use the googletest built from source (installed into build-output/googletest/linux/)
	#
	file(TO_CMAKE_PATH ${ENL_ROOT}/build-output/googletest/linux GTEST_BUILD_ROOT)
	file(TO_CMAKE_PATH ${GTEST_BUILD_ROOT}/lib GTEST_LIB_PATH)
	file(TO_CMAKE_PATH ${GTEST_BUILD_ROOT}/include GTEST_INCLUDE_DIR)

	if (${CMAKE_CONFIGURATION} STREQUAL debug)
		find_library(GTEST_LIB gtestd ${GTEST_LIB_PATH})
	else()
		find_library(GTEST_LIB gtest ${GTEST_LIB_PATH})
	endif()

	# Use -fshort-wchar so wchar_t is 2 bytes, matching SQL Server's UTF-16
	# and the managed C# extension (consistent with R/Python/Java extensions).
	#
	target_compile_options(dotnet-core-CSharp-extension-test PRIVATE -fshort-wchar)

	# Link with dl for dlopen/dlsym and pthread for gtest
	#
	find_package(Threads REQUIRED)
	target_link_libraries(dotnet-core-CSharp-extension-test Threads::Threads ${CMAKE_DL_LIBS})
endif()

# This is not a standard include path so test projects need
# to add it explicitly
#
include_directories(
	"${DOTNETCORE_CSHARP_EXTENSION_TEST_INCLUDE_DIR}"
	"${EXTENSION_API_HOME}/include"
	"${DOTNETCORE_CSHARP_EXTENSION_HOME}/include"
	"${GTEST_INCLUDE_DIR}"
)

# This string comparison is case sensitive
#
if(${CMAKE_CONFIGURATION} STREQUAL debug)
	add_definitions(-D_DEBUG)
endif()

target_link_libraries(dotnet-core-CSharp-extension-test
	${GTEST_LIB}
)

install(TARGETS dotnet-core-CSharp-extension-test DESTINATION ${DOTNETCORE_CSHARP_EXTENSION_TEST_INSTALL_DIR})
