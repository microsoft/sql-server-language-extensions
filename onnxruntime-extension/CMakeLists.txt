cmake_minimum_required(VERSION 3.20)
project(onnxextension)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE ON)

# Define version of ML.OnnxRuntime as a constant
set(ONNXRUNTIME_VERSION 1.19.2)

# set the variable for src directory
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Source files
set(SOURCES
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/OnnxSession.cpp
    ${SRC_DIR}/util.cpp
    ${SRC_DIR}/OrtApis.cpp
)

# Create shared library (must come before target_include_directories)
add_library(onnxextension SHARED ${SOURCES})

# Include directories
target_include_directories(onnxextension PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/packages/Microsoft.ML.OnnxRuntime.${ONNXRUNTIME_VERSION}/build/native/include
    ${CMAKE_SOURCE_DIR}/packages/nlohmann.json.3.12.0/build/native/include
)

# Link directories
if(WIN32)
    link_directories(
        ${CMAKE_SOURCE_DIR}/packages/Microsoft.ML.OnnxRuntime.${ONNXRUNTIME_VERSION}/runtimes/win-x64/native
    )
elseif(UNIX)
    link_directories(
        ${CMAKE_SOURCE_DIR}/packages/Microsoft.ML.OnnxRuntime.${ONNXRUNTIME_VERSION}/runtimes/linux-x64/native
    )
endif()

# Copy DLL to output directory
add_custom_command(TARGET onnxextension POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/packages/Microsoft.ML.OnnxRuntime.${ONNXRUNTIME_VERSION}/runtimes/win-x64/native/onnxruntime.dll"
    $<TARGET_FILE_DIR:onnxextension>
)