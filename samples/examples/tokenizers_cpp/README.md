# SQL Server Language Extensions - Tokenizers

This project provides a C++ tokenizer library (`tokenizers_cpp.dll`) that can be used with SQL Server Language Extensions. It includes implementations of various tokenizers including HuggingFace tokenizers and SentencePiece tokenization.
This is a wrapper on top of [tokenizer_cpp project](https://github.com/mlc-ai/tokenizers-cpp) and provides standardized interface for the SQL Server Language Extensions.

## Overview

This library serves as a bridge between SQL Server and various tokenization implementations, allowing efficient text processing and tokenization within SQL Server operations. It's particularly useful for:

- Natural Language Processing (NLP) tasks in SQL Server
- Text preprocessing for machine learning models
- Standardized tokenization across different SQL Server instances

## Project Structure

```
├── build.ps1                # Main build script for Windows
├── CMakeLists.txt          # Main CMake configuration
├── src/
│   └── tokenizer_exports.cpp  # Main tokenizer exports
├── externals/
    └── tokenizers-cpp/      # tokenizer sub-module from HuggingFace
```

## Prerequisites

- Windows operating system
- PowerShell
- CMake (3.2x or higher)
- Rust toolchain (will be automatically installed by the build script if missing)
- Visual Studio 2022 or higher with C++ development tools

## Building the Project

1. Clone the repository with submodules:
   ```powershell
   git clone --recursive <repository-url>
   ```

2. Run the build script:
   ```powershell
   .\build.ps1
   ```

   By default, the script builds both Debug and Release configurations. You can specify a specific configuration:
   ```powershell
   .\build.ps1 -BuildConfig "Release"  # For Release only
   .\build.ps1 -BuildConfig "Debug"    # For Debug only
   ```

The build process will:
- Check and install Rust if needed
- Initialize and update git submodules
- Create a build directory
- Configure and build the project using CMake
- Generate the tokenizers_cpp.dll library

## Build Output

After a successful build, you can find the compiled library at:
- Release: `build/Release/tokenizers_cpp.dll`
- Debug: `build/Debug/tokenizers_cpp.dll`

## Usage

The output binaries supports below tokenizer implementations:

1. HuggingFace Tokenizer (`huggingface_tokenizer.cc`)
2. SentencePiece Tokenizer (`sentencepiece_tokenizer.cc`)

The output binaries needs to be placed in the same directory as Runtimes directory and to be supplied as LOCAL_RUNTIME_PATH in the external model T-SQL query.

Example:
```sql
CREATE EXTERNAL MODEL myLocalOnnxModel
WITH (
	LOCATION = 'C:/Models/sentence-transformers_all-MiniLM-L6-v2',
	API_FORMAT = 'ONNX Runtime',
	MODEL_TYPE = EMBEDDINGS,
	MODEL = 'allMiniLM',
	PARAMETERS = '{"ONNX_TOKENIZER_DLL_FILE_NAME":"tokenizers_cpp.dll" }', -- optional param to explictly mention dll name
	LOCAL_RUNTIME_PATH = 'C:/Runtimes'
);
```
where `C:/Runtimes` directory contains both the onnxruntime.dll and tokenizers_cpp.dll. 

## Troubleshooting

If you encounter build issues:
1. Ensure all prerequisites are installed
2. Check that submodules are properly initialized
3. Verify Visual Studio installation and C++ components
4. Check the build logs in the `build` directory