# ONNX Runtime Extension

This directory contains extensions for integrating ONNX Runtime with SQL Server. It builds a shared library (onnxextension.dll) that dynamically loads onnxruntime.dll and tokenizers_cpp.dll to perform inference using ONNX models. This library enables the AIRuntimeHost process, which runs alongside the sqlservr process, to interface with ONNX Runtime. This extension supports only single thread execution as AIRuntimeHost processes one request at a time only.

## Folder Structure

```
onnxruntime-extension/
|-- CMakeLists.txt                        # CMake build configuration for the extension
|-- README.md                             # Project documentation
|-- build/
|   |-- windows/
|   |   |-- build-onnxruntime-extension.cmd   # Batch script to build ONNX Runtime extension for Windows
|   |   |-- build.ps1                        # PowerShell script for building the extension on Windows
|   |   |-- restore-packages.cmd             # Script to restore NuGet packages for the build
|-- cleanup.cmd                            # Script to clean up external modules and packages after build script is executed.
|-- include/
|   |-- OnnxSession.h                       # Header for ONNX session management
|   |-- OrtApis.h                           # Header for ONNX Runtime API wrappers
|   |-- sqlexternallanguage.h               # Header for SQL external language integration
|   |-- sqlexternallibrary.h                # Header for SQL external library integration
|   `-- util.h                              # Utility functions and definitions
|-- packages.config                         # NuGet package configuration file listing any dependencies
|-- scripts/
|   |-- CMakeLists_tokenizers_cpp.txt       # CMake config for building tokenizers_cpp
|   |-- StaticExports.cc                    # Static export definitions for tokenizers_cpp
|   |-- build_tokenizers_cpp.ps1            # PowerShell script to build tokenizers_cpp.dll
|   |-- convert_to_onnx_with_tokenizer_exported.py # Script to convert PyTorch models to ONNX
|-- src/
    |-- OnnxSession.cpp                     # Implementation of ONNX session management
    |-- main.cpp                            # Main entry point for the extension
    |-- util.cpp                            # Implementation of utility functions
```

## Prerequisites

- **CMake** 3.20 or higher
- **Visual Studio 2019** or later (with C++ toolchain)
- **Python** 3.8 or higher (required for model conversion scripts only)
- **Rust** (required only if you need to build `tokenizers_cpp.dll` for internal testing)
- **NuGet CLI** (for restoring packages)

## Getting Started

1. **Build the extension:**
    First, restore the NuGet package by running `build\windows\restore-packages.cmd` script.
    Execute the `build\windows\build-onnxruntime-extension.cmd Debug Release` to build for Debug and Release bits for windows x64 platform. This will build the onnxextension.dll file.
    __Note: Here we just need header files of ONNX Runtime and no static/shared library to build it. For tokenizers_cpp dependency no headers or any libraries are required for building.__

    After building, the onnxextension.dll will be located in the output directory (e.g., `build\x64\Release`). This DLL is intended to be loaded by the AIRuntimeHost process during SQL Server inference.

2. **Cleanup:**
    There is a cleanup.cmd script to remove external party module and packages. This is only required during development.

3. **Build tokenizers_cpp.dll for internal testing purposes:**
    `scripts` directory contains `build_tokenizers_cpp.ps1` which can be used to install rustup, git clone tokenizers_cpp from github and build the dll file. This is only required for generating the `tokenizers_cpp.dll` for internal testing and is not part of the core onnxextension implementation.

4. **Convert PyTorch models to ONNX format for SQL:**
    `scripts\convert_to_onnx_with_tokenizer_exported.py` contains sample python script to convert a pytorch model to onnx format which can then be used by onnxextension.dll. This will generate model files in specific directories. Again, this is not part of the core onnxextension implementation.

5. **Setup:**
    Place `onnxruntime.dll` and `tokenizers_cpp.dll` in the same directory and use this directory path in External Model DDL `LOCAL_RUNTIME_PATH` field.
    Use the directory containing model related files (including `tokenizer.json`) in External Model DDL `Location` field.
    This sets up the configuration for generating embeddings using `ai_generate_embeddings` built-in in Sql.