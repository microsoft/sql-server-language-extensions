cmake_minimum_required(VERSION 3.20)

# Set the project name
project(tokenizers_cpp_onnx)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the tokenizers_cpp external project
add_subdirectory(externals/tokenizers-cpp)

# Define the shared library to be generated
add_library(tokenizers_cpp_onnx SHARED src/tokenizer_exports.cpp)

# Set the output name to "tokenizers_cpp" for the final DLL
set_target_properties(tokenizers_cpp_onnx
    PROPERTIES 
    OUTPUT_NAME "tokenizers_cpp"
)

# Link against tokenizers_cpp
target_link_libraries(tokenizers_cpp_onnx PRIVATE tokenizers_cpp)

# Specify include directories
target_include_directories(tokenizers_cpp_onnx PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/externals/tokenizers-cpp/include)