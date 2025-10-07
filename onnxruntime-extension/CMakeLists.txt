# -----------------------------------------------------------------------------
# CMake configuration for building the ONNX Runtime Extension shared library.
# Supports both Windows and Linux platforms with platform-specific include and
# link paths. Dependencies are expected to be restored via NuGet (Windows) or
# curl/manual download (Linux).
#
# Requirements:
# - CMake >= 3.20
# - ONNX Runtime version 1.19.2
# - nlohmann.json version 3.12.0
# -----------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.20)
project(onnxextension)

# Define version of ML.OnnxRuntime as a constant
set(ONNXRUNTIME_VERSION 1.19.2)
# Define version of nlohmann.json as a constant
set(NLOHMANN_JSON_VERSION 3.12.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE ON)

# set the variable for src directory
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Source files

if(WIN32)
    set(ONNXSESSION_SRC ${SRC_DIR}/windows/OnnxSession_windows.cpp)
    set(ORTAPIS_SRC ${SRC_DIR}/windows/OrtApis_windows.cpp)
elseif(UNIX)
    set(ONNXSESSION_SRC ${SRC_DIR}/linux/OnnxSession_linux.cpp)
    set(ORTAPIS_SRC ${SRC_DIR}/linux/OrtApis_linux.cpp)
endif()

set(SOURCES
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/OnnxSession.cpp
    ${ONNXSESSION_SRC}
    ${SRC_DIR}/util.cpp
    ${SRC_DIR}/OrtApis.cpp
    ${ORTAPIS_SRC}
)

# Create shared library (must come before target_include_directories)
add_library(onnxextension SHARED ${SOURCES})

# Include directories (platform-specific)
# Windows uses NuGet to restore packages, while Linux uses curl/manual download from GitHub.
# As a result, the ONNX Runtime and nlohmann.json package directory structures differ between platforms.
if(WIN32)
    target_include_directories(onnxextension PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/packages/Microsoft.ML.OnnxRuntime.${ONNXRUNTIME_VERSION}/build/native/include
        ${CMAKE_SOURCE_DIR}/packages/nlohmann.json.${NLOHMANN_JSON_VERSION}/build/native/include
    )
elseif(UNIX)
    target_include_directories(onnxextension PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/packages/Microsoft.ML.OnnxRuntime.${ONNXRUNTIME_VERSION}/include
        ${CMAKE_SOURCE_DIR}/packages/nlohmann.json.${NLOHMANN_JSON_VERSION}/include
    )
endif()

# Link directories (platform-specific)
# Windows uses NuGet to restore packages, while Linux uses curl/manual download from GitHub.
# As a result, the ONNX Runtime package directory structure differs between platforms.
if(WIN32)
    link_directories(
        ${CMAKE_SOURCE_DIR}/packages/Microsoft.ML.OnnxRuntime.${ONNXRUNTIME_VERSION}/runtimes/win-x64/native
    )
elseif(UNIX)
    link_directories(
        ${CMAKE_SOURCE_DIR}/packages/Microsoft.ML.OnnxRuntime.${ONNXRUNTIME_VERSION}/lib
    )
endif()

# Copy ONNX Runtime library to output directory (platform-specific)
if(WIN32)
    add_custom_command(TARGET onnxextension POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/packages/Microsoft.ML.OnnxRuntime.${ONNXRUNTIME_VERSION}/runtimes/win-x64/native/onnxruntime.dll"
        $<TARGET_FILE_DIR:onnxextension>
    )
elseif(UNIX)
    add_custom_command(TARGET onnxextension POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/packages/Microsoft.ML.OnnxRuntime.${ONNXRUNTIME_VERSION}/lib/libonnxruntime.so.${ONNXRUNTIME_VERSION}"
        $<TARGET_FILE_DIR:onnxextension>/libonnxruntime.so.${ONNXRUNTIME_VERSION}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/packages/Microsoft.ML.OnnxRuntime.${ONNXRUNTIME_VERSION}/lib/libonnxruntime.so.1"
        $<TARGET_FILE_DIR:onnxextension>/libonnxruntime.so.1
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/packages/Microsoft.ML.OnnxRuntime.${ONNXRUNTIME_VERSION}/lib/libonnxruntime_providers_shared.so"
        $<TARGET_FILE_DIR:onnxextension>/libonnxruntime_providers_shared.so
    )
endif()