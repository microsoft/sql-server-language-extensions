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
#include <string>
#include <vector>
#include <cassert>
#include <sstream>
#include <filesystem>

// Windows headers
#include <windows.h>
#include <strsafe.h>

// SQL headers (must come after strsafe.h)
#include <sql.h>
#include <sqlext.h>

// ONNX Runtime headers
#include "OnnxSession.h"
#include "util.h"
#include "OrtApis.h"

using namespace std;

// Default ONNX model file name
const wstring DEFAULT_ONNX_MODEL_FILE_NAME = L"model.onnx";

// Default tokenizer JSON file name
const wstring DEFAULT_ONNX_TOKENIZER_FILE_NAME = L"tokenizer.json";

// Default tokenizer DLL file name (Windows)
const wstring DEFAULT_ONNX_TOKENIZER_DLL_NAME = L"tokenizers_cpp.dll";

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
OnnxSession::OnnxSession(const CHAR *LocalRuntimePath)
{
	if (LocalRuntimePath == nullptr)
	{
		throw std::invalid_argument("LocalRuntimePath cannot be null");
	}

	// Set the local runtime path (converts CHAR* to WCHAR* and stores internally)
	SetLocalRuntimePath((CHAR *)LocalRuntimePath, strlen(LocalRuntimePath));

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
//
OnnxSession::~OnnxSession()
{
	// Release the ONNX Runtime session if it exists
	if (m_session) {
		g_apiFuncs.ReleaseSession(m_session);
		m_session = nullptr;
	}

	// Release the session options if they exist
	if (m_session_options) {
		g_apiFuncs.ReleaseSessionOptions(m_session_options);
		m_session_options = nullptr;
	}

	// Release the ONNX Runtime environment if it exists
	if (m_env) {
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

	// Copy the input buffer into a temporary wstring
	std::wstring temp(modelLocation, sizeInWchar);

	// Allocate a new WCHAR buffer and copy the string, ensuring null-termination
	auto newPtr = std::make_unique<WCHAR[]>(sizeInWchar + 1);
	wmemcpy_s(newPtr.get(), sizeInWchar + 1, temp.c_str(), sizeInWchar);
	newPtr[sizeInWchar] = L'\0';

	// Store the buffer in the class member for automatic memory management
	m_modelLocation = std::move(newPtr);
}

//--------------------------------------------------------------------------------------------
// Name: SetEmbeddingParam
//
// Description:
//   Sets the embedding parameter for the ONNX Runtime session.
//   Copies the input CHAR buffer into an internal unique_ptr for safe memory management.
//
// Parameters:
//   param        - Pointer to the CHAR buffer containing the embedding parameter.
//   sizeInChars  - Number of characters in the buffer.
//
void OnnxSession::SetEmbeddingParam(CHAR *param, size_t sizeInChars)
{
	// Validate input arguments
	if (param == nullptr || sizeInChars == 0)
	{
		throw std::invalid_argument("SetEmbeddingParam: param is null or size is zero");
	}

	// Allocate a new CHAR buffer and copy the string, ensuring null-termination
	auto newPtr = std::make_unique<CHAR[]>(sizeInChars + 1);
	memcpy_s(newPtr.get(), (sizeInChars + 1) * sizeof(CHAR), param, sizeInChars * sizeof(CHAR));
	newPtr[sizeInChars] = '\0';

	// Store the buffer in the class member for automatic memory management
	m_embedding_param = std::move(newPtr);
}

//--------------------------------------------------------------------------------------------
// Name: SetDefaultParams
//
// Description:
//   Sets the default parameters for the ONNX Runtime session.
//   Copies the input WCHAR buffer into an internal unique_ptr for safe memory management.
//
// Note:
//   This function assumes that any CHAR* input is UTF-8 encoded. Ensure that all CHAR* strings
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

	std::unique_ptr<WCHAR[]> newPtr;

	if (sizeInBytes == 0)
	{
		// If input is empty, allocate a single WCHAR for the null terminator
		newPtr = std::make_unique<WCHAR[]>(1);
		newPtr[0] = L'\0';
	}
	else
	{
		// Copy the input buffer and ensure null-termination
		size_t sizeInWchar = sizeInBytes / sizeof(WCHAR);
		newPtr = std::make_unique<WCHAR[]>(sizeInWchar + 1);
		memset(newPtr.get(), 0, (sizeInWchar + 1) * sizeof(WCHAR));
		memcpy_s(newPtr.get(), (sizeInWchar + 1) * sizeof(WCHAR), params, sizeInBytes);
		newPtr[sizeInWchar] = L'\0';
	}
	m_defaultParams = std::move(newPtr);
}

//--------------------------------------------------------------------------------------------
// Name: SetLocalRuntimePath
//
// Description:
//   Sets the local runtime path for the ONNX Runtime session.
//   Converts the input CHAR buffer to WCHAR and stores it in an internal unique_ptr.
//
// Parameters:
//   localRuntimePath - Pointer to the CHAR buffer containing the runtime path.
//   sizeInChar       - Number of characters in the buffer.
//
void OnnxSession::SetLocalRuntimePath(CHAR *localRuntimePath, size_t sizeInChar)
{
	// Validate input arguments
	if (localRuntimePath == nullptr || sizeInChar == 0)
	{
		throw std::invalid_argument("SetLocalRuntimePath: localRuntimePath is null or size is zero");
	}

	// Convert the CHAR buffer to a wide string (WCHAR)
	std::wstring wpath = String_to_Wstring(localRuntimePath, sizeInChar);
	if (wpath.empty())
	{
		throw std::invalid_argument("SetLocalRuntimePath: converted path is empty");
	}

	// Allocate a new WCHAR buffer and copy the string, ensuring null-termination
	auto newPtr = std::make_unique<WCHAR[]>(wpath.size() + 1);
	wcscpy_s(newPtr.get(), wpath.size() + 1, wpath.c_str());

	// Store the buffer in the class member for automatic memory management
	m_localRuntimePath = std::move(newPtr);
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
//   TRUE if the session is successfully set up, FALSE otherwise.
//
BOOL OnnxSession::SetupOnnxSession()
{
	// If the session is already initialized, return success.
	if (m_session != nullptr)
	{
		return TRUE;
	}

	// Ensure required members are set.
	assert(m_modelLocation);
	assert(m_env);
	assert(m_session_options);

	// 1. Determine model, tokenizer, and tokenizer DLL file paths.

	// Get model file name from default parameters, or use default.
	wstring modelFileName = OnnxRuntimeExtensionHelpers::ExtractJsonKey(m_defaultParams.get(), DEFAULT_PARAMETERS_KEYS.ONNX_MODEL_FILE_NAME);
	if (!modelFileName.empty())
	{
		m_onnxModelFilePath = (std::filesystem::path(m_modelLocation.get()) / modelFileName).wstring();
	}
	else
	{
		m_onnxModelFilePath = (std::filesystem::path(m_modelLocation.get()) / DEFAULT_ONNX_MODEL_FILE_NAME).wstring();
	}

	// Get tokenizer JSON file name from default parameters, or use default.
	wstring tokenizerFileName = OnnxRuntimeExtensionHelpers::ExtractJsonKey(m_defaultParams.get(), DEFAULT_PARAMETERS_KEYS.TOKENIZER_JSON_FILE_NAME);
	if (!tokenizerFileName.empty())
	{
		m_onnxTokenizerFilePath = (std::filesystem::path(m_modelLocation.get()) / tokenizerFileName).wstring();
	}
	else
	{
		m_onnxTokenizerFilePath = (std::filesystem::path(m_modelLocation.get()) / DEFAULT_ONNX_TOKENIZER_FILE_NAME).wstring();
	}

	// Get tokenizer DLL file name from default parameters, or use default.
	wstring tokenizerDllFileName = OnnxRuntimeExtensionHelpers::ExtractJsonKey(m_defaultParams.get(), DEFAULT_PARAMETERS_KEYS.TOKENIZER_DLL_FILE_NAME);
	if (!tokenizerDllFileName.empty())
	{
		m_onnxTokenizerDLLFilePath = (std::filesystem::path(GetLocalRuntimePath()) / tokenizerDllFileName).wstring();
	}
	else
	{
		m_onnxTokenizerDLLFilePath = (std::filesystem::path(GetLocalRuntimePath()) / DEFAULT_ONNX_TOKENIZER_DLL_NAME).wstring();
	}

	// Check if the model file exists.
	if (!OnnxRuntimeExtensionHelpers::DoesFileExist(m_onnxModelFilePath))
	{
		LogMessage(L"Model file not found: " + m_onnxModelFilePath, LogLevel::CRITICAL_ERROR);
		return FALSE;
	}

	// Check if the tokenizer JSON file exists.
	if (!OnnxRuntimeExtensionHelpers::DoesFileExist(m_onnxTokenizerFilePath))
	{
		LogMessage(L"Tokenizer file not found: " + m_onnxTokenizerFilePath, LogLevel::CRITICAL_ERROR);
		return FALSE;
	}

	// Check if the tokenizer DLL file exists.
	if (!OnnxRuntimeExtensionHelpers::DoesFileExist(m_onnxTokenizerDLLFilePath))
	{
		LogMessage(L"Tokenizer DLL file not found: " + m_onnxTokenizerDLLFilePath, LogLevel::CRITICAL_ERROR);
		return FALSE;
	}

	// 2. Configure ONNX Runtime session options.
#ifdef _DEBUG
	// Debug: lower optimization, single thread, verbose logging.
	CheckStatus(g_apiFuncs.SetGraphOptimizationLevel(m_session_options, ORT_ENABLE_BASIC));
	CheckStatus(g_apiFuncs.SetIntraOpNumThreads(m_session_options, 1));
	CheckStatus(g_apiFuncs.SetExecutionMode(m_session_options, ORT_SEQUENTIAL));
	CheckStatus(g_apiFuncs.SetLogSeverityLevel(m_session_options, ORT_LOGGING_LEVEL_VERBOSE));
#else
	// Release: full optimization, multi-threaded, warning logging.
	CheckStatus(g_apiFuncs.SetGraphOptimizationLevel(m_session_options, ORT_ENABLE_ALL));
	CheckStatus(g_apiFuncs.SetIntraOpNumThreads(m_session_options, 4));
	CheckStatus(g_apiFuncs.SetExecutionMode(m_session_options, ORT_PARALLEL));
	CheckStatus(g_apiFuncs.SetLogSeverityLevel(m_session_options, ORT_LOGGING_LEVEL_WARNING));
#endif

	// Try to use CUDA (GPU) execution provider if available.
	bool use_gpu = false;
	if (g_apiFuncs.AppendExecutionProviderCUDA) {
		OrtCUDAProviderOptions cuda_options;
		cuda_options.device_id = 0;
		if (g_apiFuncs.AppendExecutionProviderCUDA(m_session_options, &cuda_options) == nullptr) {
			use_gpu = true;
			LogMessage(L"Using CUDA execution provider", LogLevel::INFO);
		}
	}

	// Fallback to CPU execution provider if CUDA is not used.
	if (!use_gpu) {
		if (g_apiFuncs.AppendExecutionProviderCPU(m_session_options, 0) == nullptr) {
			LogMessage(L"Using CPU execution provider", LogLevel::INFO);
		} else {
			return FALSE;
		}
	}

	// 3. Load the ONNX model into a session.
	CheckStatus(g_apiFuncs.CreateSession(m_env, m_onnxModelFilePath.c_str(), m_session_options, &m_session));

#ifdef _WIN32
	// 4. Load tokenizer DLL (Windows).
	m_HMTokenizerDll = LoadLibraryExW(m_onnxTokenizerDLLFilePath.c_str(), NULL, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
	if (m_HMTokenizerDll == NULL)
	{
		LogMessage(L"Failed to load the tokenizer DLL: " + m_onnxTokenizerDLLFilePath + L". Error: " + std::to_wstring(GetLastError()), LogLevel::CRITICAL_ERROR);
		return FALSE;
	}

	// Get the function pointer for LoadBlobJsonAndEncode from the DLL.
	m_pFnLoadBlobJsonAndEncode =
		reinterpret_cast<PFN_LoadBlobJsonAndEncode>(GetProcAddress(m_HMTokenizerDll, "LoadBlobJsonAndEncode"));

	if (!m_pFnLoadBlobJsonAndEncode)
	{
		LogMessage(L"Failed to get function address \"void LoadBlobJsonAndEncode(const std::string &, const std::string &, std::vector<int32_t> &)\"", LogLevel::CRITICAL_ERROR);
		return FALSE;
	}
#else
	// 4. Load tokenizer shared library (Linux).
	void *handle = dlopen("tokenizers_cpp.so", RTLD_LAZY);
	if (!handle)
	{
		LogMessage(L"Failed to load the shared library: " + std::wstring(dlerror()), LogLevel::CRITICAL_ERROR);
		return FALSE;
	}

	m_pFnLoadBlobJsonAndEncode = (PFN_LoadBlobJsonAndEncode)dlsym(handle, "LoadBlobJsonAndEncode");

	const char *error = dlerror();
	if (error != NULL)
	{
		LogMessage(L"Failed to get function address \"void LoadBlobJsonAndEncode(const std::string &, const std::string &, std::vector<int32_t> &)\": " + std::wstring(error), LogLevel::CRITICAL_ERROR);
		dlclose(handle);
		return FALSE;
	}
#endif

	return TRUE;
}

// OnnxSession::GenerateSingleEmbedding
//
// Sets up the model path and tokenizer.json path, and generates embeddings.
//
// Returns:
//   TRUE on success, FALSE on failure
BOOL OnnxSession::GenerateSingleEmbedding()
{
	BOOL isSuccess = FALSE;
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
//   TRUE on success, FALSE on failure
//
// Note: This function expects the model.onnx and tokenizer.json files to be present in the same directory.

BOOL OnnxSession::TokenizeAndGenerateEmbeddings(
	_In_ string userInputString,
	_Out_ wstring &output_str,
	_In_opt_ BOOL fVerboseOutput /* = false by default */)
{
	// Define custom deleters for ONNX Runtime objects to ensure proper resource management
	auto memory_info_deleter = [](OrtMemoryInfo* p) { if (p) g_ortApi->ReleaseMemoryInfo(p); };
	auto shape_info_deleter = [](OrtTensorTypeAndShapeInfo* p) { if (p) g_ortApi->ReleaseTensorTypeAndShapeInfo(p); };
	auto value_deleter = [](OrtValue* p) { if (p) g_ortApi->ReleaseValue(p); };

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
		OrtMemoryInfo* raw_memory_info = nullptr;
		CheckStatus(g_ortApi->CreateCpuMemoryInfo(OrtArenaAllocator, OrtMemTypeDefault, &raw_memory_info));
		memory_info.reset(raw_memory_info);

		// Load the tokenizer.json file as a string blob
		string blob = OnnxRuntimeExtensionHelpers::LoadBytesFromFile(m_onnxTokenizerFilePath);

		// Tokenize the input string using the tokenizer DLL function
		vector<int> token_ids;
		if (blob.empty())
		{
			LogMessage(L"Tokenizer JSON blob is empty. Cannot tokenize input.", LogLevel::CRITICAL_ERROR);
			return FALSE;
		}
		if (m_pFnLoadBlobJsonAndEncode == nullptr)
		{
			LogMessage(L"Tokenizer function pointer is null. Cannot tokenize input.", LogLevel::CRITICAL_ERROR);
			return FALSE;
		}
		m_pFnLoadBlobJsonAndEncode(blob, userInputString, token_ids);

#ifdef _DEBUG
		// Log the encoded token IDs if verbose output is enabled
		if (fVerboseOutput)
		{
			std::wstringstream ss;
			ss << L"Encoded token IDs: [";
			for (size_t i = 0; i < token_ids.size(); ++i)
			{
				ss << token_ids[i];
				if (i != token_ids.size() - 1)
					ss << L", ";
			}
			ss << L"]";
			LogMessage(ss.str(), LogLevel::INFO);
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
			m_session,                // Session handle
			nullptr,                  // Optional run options
			INPUT_NAMES,              // Input node names
			inputs,                   // Input tensors
			input_ids_num_dims,       // Number of inputs
			OUTPUT_NAMES,             // Output node names
			1,                        // Number of outputs
			&raw_output_tensor));     // Output tensor pointer
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
			LogMessage(L"Output tensor does not have at least 3 dimensions.", LogLevel::CRITICAL_ERROR);
			return FALSE;
		}

		// Extract the embedding dimension count from the output tensor shape
		const int64_t embedding_dim_count = dims[2];

		// Log the embedding vector if verbose output is enabled
		if (fVerboseOutput)
		{
			std::wstringstream ss;
			ss << L"Vector embedding of dimensions " << embedding_dim_count << L" : [";
			for (size_t i = 0; i < embedding_dim_count; ++i)
			{
				if (i > 0) ss << L", ";
				ss << output_data[i];
			}
			ss << L"]";
			LogMessage(ss.str(), LogLevel::INFO);
		}

		// Convert the float array output_data to a wide string output_str
		output_str = OnnxRuntimeExtensionHelpers::ConvertFloatsToWideString(output_data, static_cast<int>(embedding_dim_count));
	}
	catch (const std::exception &e)
	{
		// Log any standard exceptions as critical errors
		LogMessage(e.what(), LogLevel::CRITICAL_ERROR);
		return FALSE;
	}
	catch (...)
	{
		// Log any unknown exceptions as critical errors
		LogMessage("ONNX Runtime error.", LogLevel::CRITICAL_ERROR);
		return FALSE;
	}

	return TRUE;
}
