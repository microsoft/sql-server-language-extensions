//**************************************************************************************************
// ONNXRuntime-extension : A language extension implementing the SQL Server AIRuntime extension
// for ONNX Runtime.
// Copyright (C) 2025 Microsoft Corporation.
//
// @File: OnnxSession.cpp
//
// Purpose:
//  This file defines the OnnxSession class, which is responsible for managing the ONNX Runtime session.
//
//**************************************************************************************************




#include <iostream>
#include <vector>
#include <cassert>
#include <sstream>
#include <filesystem>

#ifdef _WIN32
// Windows headers
#include <windows.h>
#include <strsafe.h>
#else
// POSIX headers
#include <dlfcn.h>
#endif

#include <sql.h>
#include <sqlext.h>

#include "OnnxSession.h"
#include "util.h"
#include "OrtApis.h"

#ifndef _WIN32
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#endif

using namespace std;

// Default ONNX model file name
const string DEFAULT_ONNX_MODEL_FILE_NAME = "model.onnx";

// Default tokenizer JSON file name
const string DEFAULT_ONNX_TOKENIZER_FILE_NAME = "tokenizer.json";

// Default tokenizer DLL file name (Windows)
const string DEFAULT_ONNX_TOKENIZER_DLL_NAME_WINDOWS = "tokenizers_cpp.dll";
// Default tokenizer DLL file name (Linux)
const string DEFAULT_ONNX_TOKENIZER_DLL_NAME_LINUX = "libtokenizers_cpp.so";

// Input and output names for the ONNX model
const char *INPUT_NAMES[] = {"input_ids", "attention_mask"};
const char *OUTPUT_NAMES[] = {"token_embeddings", "sentence_embedding"};

//--------------------------------------------------------------------------------------------
// Name: OnnxSession::OnnxSession
//
// Description:
//   Constructor for the OnnxSession class. Initializes the ONNX Runtime environment and session
//   options, and loads the ONNX Runtime DLL from the specified local runtime path.
//
// Parameters:
//   LocalRuntimePath - Pointer to a CHAR buffer containing the path to the ONNX Runtime DLL.
//
OnnxSession::OnnxSession(const char *LocalRuntimePath)
{
	if (LocalRuntimePath == nullptr)
	{
		throw std::invalid_argument("LocalRuntimePath cannot be null");
	}

	// Set the local runtime path
	SetLocalRuntimePath((char *)LocalRuntimePath, strlen(LocalRuntimePath));

	// Load the ONNX Runtime DLL from the specified path
	int loadResult = LoadOnnxDll(GetLocalRuntimePath());

	// If loading failed, throw an exception with a detailed message
	if (loadResult != 0)
	{
		std::ostringstream oss;
		oss << "Failed to load ONNX Runtime DLL from path: \"" << LocalRuntimePath << "\".";
		throw std::runtime_error(oss.str());
	}

	// Create the ONNX Runtime environment and session options
	CheckStatus(g_apiFuncs.CreateEnv(ORT_LOGGING_LEVEL_WARNING, "airuntime_embedder", &m_env));
	CheckStatus(g_apiFuncs.CreateSessionOptions(&m_session_options));
	m_session = nullptr; // Initialize session pointer to nullptr
}

//--------------------------------------------------------------------------------------------
// Name: OnnxSession::~OnnxSession
//
// Description:
//   Destructor for the OnnxSession class. Releases all ONNX Runtime resources, including the
//   session, session options, environment, and tokenizer DLL.
OnnxSession::~OnnxSession()
{
	// Release the ONNX Runtime session if it exists
	if (m_session)
	{
		g_apiFuncs.ReleaseSession(m_session);
		m_session = nullptr;
	}

	// Release the session options if they exist
	if (m_session_options)
	{
		g_apiFuncs.ReleaseSessionOptions(m_session_options);
		m_session_options = nullptr;
	}

	// Release the ONNX Runtime environment if it exists
	if (m_env)
	{
		g_apiFuncs.ReleaseEnv(m_env);
		m_env = nullptr;
	}

		// Free the tokenizer DLL/shared library if it was loaded
	if (m_HMTokenizerDll != nullptr)
	{
#ifdef _WIN32
		FreeLibrary(m_HMTokenizerDll);
#else
		dlclose(m_HMTokenizerDll);
#endif
		m_HMTokenizerDll = nullptr;
	}

	// Unload the ONNX Runtime DLL
	UnloadOnnxDll();
}

//--------------------------------------------------------------------------------------------
// Name: SetModelLocation
//
// Description:
//   Sets the model location for the ONNX Runtime session.
//   Copies the input WCHAR buffer into an internal unique_ptr for safe memory management.
//   Method internally converts WCHAR*(modelLocation) to UTF-8 std::string and stores it internally in m_modelLocation.
//
// Parameters:
//   modelLocation   - Pointer to the WCHAR buffer containing the model location path.
//   cbModelLocation - Size of the buffer in bytes.
//
void OnnxSession::SetModelLocation(WCHAR *modelLocation, size_t cbModelLocation)
{
	// Validate input arguments
	if (modelLocation == nullptr || cbModelLocation == 0)
	{
		throw std::invalid_argument("SetModelLocation: modelLocation is null or size is zero");
	}
	if (cbModelLocation % sizeof(WCHAR) != 0)
	{
		throw std::invalid_argument("SetModelLocation: cbModelLocation is not a multiple of WCHAR size");
	}
	size_t sizeInWchar = cbModelLocation / sizeof(WCHAR);
	if (sizeInWchar == 0)
	{
		throw std::invalid_argument("SetModelLocation: sizeInWchar is zero");
	}

	// Convert WCHAR* to UTF-8 std::string
	std::string utf8Path = WCHAR_to_utf8_string(modelLocation, sizeInWchar);

	// Allocate a new char buffer and copy the string, ensuring null-termination
	m_modelLocation = ConvertAndAllocateUtf8Buffer(utf8Path);
}

//--------------------------------------------------------------------------------------------
// Name: SetEmbeddingParam
//
// Description:
//   Sets the embedding parameter for the ONNX Runtime session.
//   Copies the input char buffer into an internal unique_ptr for safe memory management.
//   Function ensures string is null terminated and stored it in m_embedding_param
//
// Parameters:
//   param        - Pointer to the char buffer containing the embedding parameter.
//   sizeInChars  - Number of characters in the buffer.
//
void OnnxSession::SetEmbeddingParam(char *param, size_t sizeInChars)
{
	// Validate input arguments
	if (param == nullptr || sizeInChars == 0)
	{
		throw std::invalid_argument("SetEmbeddingParam: param is null or size is zero");
	}

	// Allocate a new CHAR buffer and copy the string, ensuring null-termination
	std::string paramStr(param, sizeInChars);
	m_embedding_param = ConvertAndAllocateUtf8Buffer(paramStr);
}

//--------------------------------------------------------------------------------------------
// Name: SetDefaultParams
//
// Description:
//   Sets the default parameters for the ONNX Runtime session.
//   Copies the input WCHAR buffer into an internal char unique_ptr array for safe memory management.
//
// Note:
//   This function assumes that any char* input is UTF-8 encoded. Ensure that all char* strings
//   passed to this function are properly encoded as UTF-8.
//
// Parameters:
//   params       - Pointer to the WCHAR buffer containing the default parameters.
//   sizeInBytes  - Size of the buffer in bytes.
//
void OnnxSession::SetDefaultParams(WCHAR *params, size_t sizeInBytes)
{
	// Validate input arguments
	if ((params == nullptr && sizeInBytes != 0) || (sizeInBytes % sizeof(WCHAR) != 0))
	{
		throw std::invalid_argument("SetDefaultParams: params is null but size is not zero, or size is not a multiple of WCHAR");
	}

	if (sizeInBytes == 0)
	{
		// If input is empty, allocate a single char for the null terminator
		m_defaultParams = std::make_unique<char[]>(1);
		m_defaultParams[0] = '\0';
	}
	else
	{
		// Convert WCHAR* to UTF-8 std::string
		size_t sizeInWchar = sizeInBytes / sizeof(WCHAR);
		std::string utf8Params = WCHAR_to_utf8_string(params, sizeInWchar);

		// Allocate a new char buffer and copy the string, ensuring null-termination
		m_defaultParams = ConvertAndAllocateUtf8Buffer(utf8Params);
	}
}

//--------------------------------------------------------------------------------------------
// Name: SetLocalRuntimePath
//
// Description:
//   Sets the local runtime path for the ONNX Runtime session.
//   Stores localRuntimePath to an internal char unique_ptr array named m_localRuntimePath.
//
// Parameters:
//   localRuntimePath - Pointer to the char buffer containing the runtime path.
//   sizeInChar       - Number of characters in the buffer.
//
void OnnxSession::SetLocalRuntimePath(char *localRuntimePath, size_t sizeInChar)
{
	// Validate input arguments
	if (localRuntimePath == nullptr || sizeInChar == 0)
	{
		throw std::invalid_argument("SetLocalRuntimePath: localRuntimePath is null or size is zero");
	}

	// Convert the CHAR buffer (UTF-8) to std::string
	std::string utf8Path(localRuntimePath, sizeInChar);
	if (utf8Path.empty())
	{
		throw std::invalid_argument("SetLocalRuntimePath: converted path is empty");
	}

	// Allocate a new char buffer and copy the string, ensuring null-termination
	m_localRuntimePath = ConvertAndAllocateUtf8Buffer(utf8Path);
}

//--------------------------------------------------------------------------------------------
// Name: OnnxSession::ResolvePaths
//
// Description:
//   Determines and validates the file paths required for the ONNX session, including:
//     - The ONNX model file
//     - The tokenizer JSON file
//     - The tokenizer DLL/shared library
//   This method extracts file names from the default parameters (if provided), constructs full paths
//   using the model location and runtime path, and checks that all required files exist. If any file
//   is missing, it logs a critical error and returns false.
//
// Returns:
//   true if all required files exist and paths are set, false otherwise.
//
bool OnnxSession::ResolvePaths()
{
	std::string modelFileName = OnnxRuntimeExtensionHelpers::ExtractJsonKey(m_defaultParams.get(), OnnxDefaultConstants::ONNX_MODEL_FILE_NAME);
	if (!modelFileName.empty())
		m_onnxModelFilePath = (std::filesystem::path(m_modelLocation.get()) / modelFileName).string();
	else
		m_onnxModelFilePath = (std::filesystem::path(m_modelLocation.get()) / DEFAULT_ONNX_MODEL_FILE_NAME).string();

	std::string tokenizerFileName = OnnxRuntimeExtensionHelpers::ExtractJsonKey(m_defaultParams.get(), OnnxDefaultConstants::TOKENIZER_JSON_FILE_NAME);
	if (!tokenizerFileName.empty())
		m_onnxTokenizerFilePath = (std::filesystem::path(m_modelLocation.get()) / tokenizerFileName).string();
	else
		m_onnxTokenizerFilePath = (std::filesystem::path(m_modelLocation.get()) / DEFAULT_ONNX_TOKENIZER_FILE_NAME).string();

	std::string tokenizerDllFileName = OnnxRuntimeExtensionHelpers::ExtractJsonKey(m_defaultParams.get(), OnnxDefaultConstants::TOKENIZER_DLL_FILE_NAME);
#ifdef _WIN32
	const std::string &defaultTokenizerDllName = DEFAULT_ONNX_TOKENIZER_DLL_NAME_WINDOWS;
#else
	const std::string &defaultTokenizerDllName = DEFAULT_ONNX_TOKENIZER_DLL_NAME_LINUX;
#endif
	if (!tokenizerDllFileName.empty())
		m_onnxTokenizerDLLFilePath = (std::filesystem::path(GetLocalRuntimePath()) / tokenizerDllFileName).string();
	else
		m_onnxTokenizerDLLFilePath = (std::filesystem::path(GetLocalRuntimePath()) / defaultTokenizerDllName).string();

	if (!OnnxRuntimeExtensionHelpers::DoesFileExist(m_onnxModelFilePath))
	{
		LogMessage("Model file not found: " + m_onnxModelFilePath, LogLevel::CRITICAL_ERROR);
		return false;
	}
	if (!OnnxRuntimeExtensionHelpers::DoesFileExist(m_onnxTokenizerFilePath))
	{
		LogMessage("Tokenizer file not found: " + m_onnxTokenizerFilePath, LogLevel::CRITICAL_ERROR);
		return false;
	}
	if (!OnnxRuntimeExtensionHelpers::DoesFileExist(m_onnxTokenizerDLLFilePath))
	{
		LogMessage("Tokenizer DLL file not found: " + m_onnxTokenizerDLLFilePath, LogLevel::CRITICAL_ERROR);
		return false;
	}
	return true;
}

//--------------------------------------------------------------------------------------------
// Name: SetupOnnxSession
//
// Description:
//   Initializes the ONNX Runtime session by:
//   1. Determining the correct model, tokenizer, and tokenizer DLL file paths.
//   2. Configuring session options (optimization, threading, execution providers).
//   3. Loading the ONNX model into a session.
//   4. Loading the tokenizer DLL and setting up the function pointer for tokenization.
//
// Returns:
//   true if the session is successfully set up, false otherwise.
//
bool OnnxSession::SetupOnnxSession()
{
	// If the session is already initialized, return success.
	if (m_session != nullptr)
	{
		return true;
	}

	// Ensure required members are set.
	assert(m_modelLocation);
	assert(m_env);
	assert(m_session_options);

	// Step 1: Resolve all required file paths for the ONNX session.
	// This step determines and validates the paths for the ONNX model file, tokenizer file, and tokenizer DLL/shared library.
	// It ensures all necessary files exist and properly set.
	if (!ResolvePaths())
		return false;

	// Step 2: Configure ONNX Runtime session options.
	// This step sets optimization level, threading, execution mode, and logging severity.
	// Returns false if configuration fails.
	if (!ConfigureSessionOptions())
		return false;

	// Step 3: Load the ONNX model into the session.
	// This step loads the ONNX model from the resolved file path into the ONNX Runtime session.
	// On Windows, the path is converted to a wide string; on other platforms, the UTF-8 path is used directly.
	// Returns false if loading fails.
	if (!LoadModel())
		return false;

	// Step 4: Load the tokenizer library and function pointer.
	// This step loads the tokenizer DLL/shared library from the resolved path and retrieves the function pointer
	// for tokenization (LoadBlobJsonAndEncode). Returns false if loading fails or the function is not found.
	if (!LoadTokenizer())
		return false;

	return true;
}

// OnnxSession::GenerateSingleEmbedding
//
// Sets up the model path and tokenizer.json path, and generates embeddings.
//
// Returns:
//   true on success, false on failure
bool OnnxSession::GenerateSingleEmbedding()
{
	// Check that m_embedding_param is set
	if (!m_embedding_param || !m_embedding_param.get() || m_embedding_param.get()[0] == '\0')
	{
		LogMessage("GenerateSingleEmbedding: embedding_param is not set or empty.", LogLevel::CRITICAL_ERROR);
		return false;
	}

	bool isSuccess = false;
	wstring result;

	isSuccess = TokenizeAndGenerateEmbeddings(
		std::string(m_embedding_param.get()),
		result
#ifdef _DEBUG
		,
		true // Set verbose output to true for debugging
#endif
	);

	// Convert wstring result to JSON format
	wstring json_result = L"[" + result + L"]"; // Use wide string literal (L) for concatenation

	// Push the result to m_output_str
	m_output_str.clear();
	m_output_str.push_back(std::move(json_result));

	return isSuccess;
}

// OnnxSession::TokenizeAndGenerateEmbeddings
//
// Tokenizes the input string and runs the ONNX model to generate embeddings.
//
// Parameters:
// - userInputString: The input text to be tokenized and embedded
// - output_str: The output wide-char string to store the generated embeddings
// - fVerboseOutput: Optional flag for verbose output (default is false)
//
// Returns:
//   true on success, false on failure
//
// Note: This function expects the model.onnx and tokenizer.json files to be present in the same directory.

bool OnnxSession::TokenizeAndGenerateEmbeddings(
	_In_ string userInputString,
	_Out_ wstring &output_str,
	_In_opt_ bool fVerboseOutput /* = false by default */)
{
	// Define custom deleters for ONNX Runtime objects to ensure proper resource management
	auto memory_info_deleter = [](OrtMemoryInfo *p)
	{ if (p) g_ortApi->ReleaseMemoryInfo(p); };
	auto shape_info_deleter = [](OrtTensorTypeAndShapeInfo *p)
	{ if (p) g_ortApi->ReleaseTensorTypeAndShapeInfo(p); };
	auto value_deleter = [](OrtValue *p)
	{ if (p) g_ortApi->ReleaseValue(p); };

	// Smart pointers for ONNX Runtime objects with custom deleters
	std::unique_ptr<OrtMemoryInfo, decltype(memory_info_deleter)> memory_info(nullptr, memory_info_deleter);
	std::unique_ptr<OrtTensorTypeAndShapeInfo, decltype(shape_info_deleter)> shape_info(nullptr, shape_info_deleter);
	std::unique_ptr<OrtValue, decltype(value_deleter)> output_tensor(nullptr, value_deleter);
	std::unique_ptr<OrtValue, decltype(value_deleter)> input_ids_tensor(nullptr, value_deleter);
	std::unique_ptr<OrtValue, decltype(value_deleter)> attention_mask_tensor(nullptr, value_deleter);

	// Pointer to hold the output data from the ONNX model inference
	float *output_data = nullptr;

	try
	{
		// Create memory info for CPU allocator (OrtArenaAllocator, default memory type)
		OrtMemoryInfo *raw_memory_info = nullptr;
		CheckStatus(g_ortApi->CreateCpuMemoryInfo(OrtArenaAllocator, OrtMemTypeDefault, &raw_memory_info));
		memory_info.reset(raw_memory_info);

		// Load the tokenizer.json file as a string blob
		string blob = OnnxRuntimeExtensionHelpers::LoadBytesFromFile(m_onnxTokenizerFilePath);

		// Tokenize the input string using the tokenizer DLL function
		vector<int> token_ids;
		if (blob.empty())
		{
			LogMessage("Tokenizer JSON blob is empty. Cannot tokenize input.", LogLevel::CRITICAL_ERROR);
			return false;
		}
		if (m_pFnLoadBlobJsonAndEncode == nullptr)
		{
			LogMessage("Tokenizer function pointer is null. Cannot tokenize input.", LogLevel::CRITICAL_ERROR);
			return false;
		}
		m_pFnLoadBlobJsonAndEncode(blob, userInputString, token_ids);

		// Check if tokenizer failed to generate any tokens
		if (token_ids.empty())
		{
			LogMessage("Tokenizer did not generate any tokens for the input.", LogLevel::CRITICAL_ERROR);
			return false;
		}

#ifdef _DEBUG
		// Log the encoded token IDs if verbose output is enabled
		if (fVerboseOutput)
		{
			std::wstring text = L"Encoded token IDs: ";
			LogVerboseVector(text, token_ids, LogLevel::INFO);
		}
#endif

		// Convert token IDs to int64_t for ONNX input
		vector<int64_t> input_ids;
		input_ids.reserve(token_ids.size());
		for (int id : token_ids)
		{
			input_ids.push_back(static_cast<int64_t>(id));
		}

		// Create the attention mask: all ones, same length as input_ids
		vector<int64_t> attention_mask(input_ids.size(), 1);

		// Define input tensor dimensions: [batch_size, sequence_length]
		const int64_t batch_size = 1;
		const int64_t sequence_length = static_cast<int64_t>(input_ids.size());
		int64_t input_dims[] = {batch_size, sequence_length};
		const int input_ids_num_dims = sizeof(input_dims) / sizeof(input_dims[0]);

		// Create input_ids tensor
		OrtValue *raw_input_ids_tensor = nullptr;
		CheckStatus(g_apiFuncs.CreateTensorWithDataAsOrtValue(
			memory_info.get(),
			input_ids.data(),
			input_ids.size() * sizeof(int64_t),
			input_dims,
			input_ids_num_dims,
			ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64,
			&raw_input_ids_tensor));
		input_ids_tensor.reset(raw_input_ids_tensor);

		// Create attention_mask tensor
		OrtValue *raw_attention_mask_tensor = nullptr;
		CheckStatus(g_apiFuncs.CreateTensorWithDataAsOrtValue(
			memory_info.get(),
			attention_mask.data(),
			attention_mask.size() * sizeof(int64_t),
			input_dims,
			input_ids_num_dims,
			ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64,
			&raw_attention_mask_tensor));
		attention_mask_tensor.reset(raw_attention_mask_tensor);

		// Prepare input tensor array for inference
		OrtValue *inputs[] = {input_ids_tensor.get(), attention_mask_tensor.get()};

		// Run inference on the ONNX model
		OrtValue *raw_output_tensor = nullptr;
		CheckStatus(g_apiFuncs.Run(
			m_session,			  // Session handle
			nullptr,			  // Optional run options
			INPUT_NAMES,		  // Input node names
			inputs,				  // Input tensors
			input_ids_num_dims,	  // Number of inputs
			OUTPUT_NAMES,		  // Output node names
			1,					  // Number of outputs
			&raw_output_tensor)); // Output tensor pointer
		output_tensor.reset(raw_output_tensor);

		// Extract the output data (embeddings) from the output tensor
		CheckStatus(g_ortApi->GetTensorMutableData(
			output_tensor.get(),
			reinterpret_cast<void **>(&output_data)));

		// Retrieve the shape information of the output tensor
		OrtTensorTypeAndShapeInfo *raw_shape_info = nullptr;
		CheckStatus(g_ortApi->GetTensorTypeAndShape(
			output_tensor.get(),
			&raw_shape_info));
		shape_info.reset(raw_shape_info);

		// Get the number of dimensions in the output tensor
		size_t output_tensor_dim_count = 0;
		CheckStatus(g_ortApi->GetDimensionsCount(
			shape_info.get(),
			&output_tensor_dim_count));

		// Get the actual dimensions of the output tensor
		std::vector<int64_t> dims(output_tensor_dim_count);
		CheckStatus(g_ortApi->GetDimensions(
			shape_info.get(),
			dims.data(),
			output_tensor_dim_count));

		// The embedding dimension is typically the last dimension of the output tensor
		// Assuming the output shape is [batch_size, sequence_length, embedding_dim_count]
		if (dims.size() < 3)
		{
			LogMessage("Output tensor does not have at least 3 dimensions.", LogLevel::CRITICAL_ERROR);
			return false;
		}

		// Extract the embedding dimension count from the output tensor shape
		const int64_t embedding_dim_count = dims[2];

		// Log the embedding vector if verbose output is enabled
		if (fVerboseOutput)
		{
			// Wrap output_data in a vector<float> for logging
			std::vector<float> embedding_vec(output_data, output_data + embedding_dim_count);
			std::wstring text = L"Vector embedding of dimensions " + std::to_wstring(embedding_dim_count) + L" : ";
			LogVerboseVector(text, embedding_vec, LogLevel::INFO);
		}

		// Convert the float array output_data to a wide string output_str
		output_str = OnnxRuntimeExtensionHelpers::ConvertFloatsToWideString(output_data, static_cast<int>(embedding_dim_count));
	}
	catch (const std::exception &e)
	{
		// Log any standard exceptions as critical errors
		LogMessage(e.what(), LogLevel::CRITICAL_ERROR);
		return false;
	}
	catch (...)
	{
		// Log any unknown exceptions as critical errors
		LogMessage("ONNX Runtime error.", LogLevel::CRITICAL_ERROR);
		return false;
	}

	return true;
}
