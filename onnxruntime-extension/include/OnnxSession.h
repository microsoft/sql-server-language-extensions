//**************************************************************************************************
// ONNXRuntime-extension : A language extension implementing the SQL Server AIRuntime extension
// for ONNX Runtime.
// Copyright (C) 2025 Microsoft Corporation.
//
// @File: OnnxSession.h
//
// Purpose:
//  This file defines the OnnxSession class, which is responsible for managing the ONNX Runtime
//  session. It includes methods for loading the model, generating embeddings, and handling input
//  and output parameters. The class also provides getter and setter methods for various parameters
//  related to the ONNX session. This file also includes the necessary headers and function
//  signatures for the exported ONNX Runtime API. Therefore, this file is self-contained and using
//  it does not require linking to the ONNX Runtime library.
//
//**************************************************************************************************

#pragma once

// Define the function signature for tokenization
#ifdef _WIN32
typedef void(__cdecl *PFN_LoadBlobJsonAndEncode)(const std::string &, const std::string &, std::vector<int32_t> &);
#else
// On Linux, omit __cdecl
typedef void (*PFN_LoadBlobJsonAndEncode)(const std::string &, const std::string &, std::vector<int32_t> &);
#endif

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

// ONNX Runtime headers (adjust path based on your setup)
#include <onnxruntime_c_api.h>

// This namespace defines constant keys used for accessing default parameters related to the ONNX
// session. These keys are typically used to retrieve file names for the tokenizer JSON, tokenizer
// DLL, and the ONNX model from a configuration or parameter set.
// Using char* (UTF-8 encoded strings) for cross-platform compatibility.
// This avoids platform-specific issues with WCHAR/wchar_t and ensures consistent behavior.
namespace OnnxDefaultConstants
{
	// Key for the tokenizer JSON file name parameter
	constexpr const char *TOKENIZER_JSON_FILE_NAME = "ONNX_TOKENIZER_JSON_FILE_NAME";

	// Key for the tokenizer DLL file name parameter
	constexpr const char *TOKENIZER_DLL_FILE_NAME = "ONNX_TOKENIZER_DLL_FILE_NAME";

	// Key for the ONNX model file name parameter
	constexpr const char *ONNX_MODEL_FILE_NAME = "ONNX_MODEL_FILE_NAME";
}

//--------------------------------------------------------------------------------------------------
// Name: OnnxSession
//
// Description:
//  The OnnxSession class is responsible for managing the ONNX Runtime session, loading the ONNX model,
//  and generating embeddings from input text. It provides methods to set and get various parameters
//  related to the ONNX session, such as model location, embedding parameters, and default parameters.
//  The class also handles tokenization and embedding generation using the ONNX model.
class OnnxSession
{
public:
	// Constructor
	OnnxSession(const char *LocalRuntimePath);

	// Destructor
	~OnnxSession();

	// Method to load the ONNX model and set up the session
	bool SetupOnnxSession();

	/**
	 * @brief Generates embeddings from the current embedding parameter.
	 *
	 * This method uses the previously set embedding parameter (via SetEmbeddingParam) and runs the ONNX model to generate vector embeddings.
	 * The result is stored internally and can be retrieved with GetOutputStr().
	 *
	 * @return true if embedding generation succeeded, false otherwise.
	 *
	 * @note The embedding parameter must be set and must be UTF-8 encoded.
	 */
	bool GenerateSingleEmbedding();

	// Method to get the model location
	const char *GetModelLocation()
	{
		return m_modelLocation.get();
	}

	/**
	 * @brief Sets the model location for the ONNX Runtime session.
	 * @param modelLocation Pointer to the WCHAR buffer containing the model location path. Must be UTF-16 (Windows) or UTF-32 (Linux).
	 * @param cbModelLocation Size of the buffer in bytes.
	 * @note The input WCHAR* modelLocation is converted to utf-8 char array for cross platform compatibility and stored in m_modelLocation.
	 */
	void SetModelLocation(WCHAR *modelLocation, size_t cbModelLocation);

	// Getter for m_embedding_param
	const char *GetEmbeddingParam()
	{
		return m_embedding_param.get();
	}

	/**
	 * @brief Sets the embedding parameter for the ONNX Runtime session.
	 *
	 * @param param Pointer to a UTF-8 encoded char buffer containing the embedding parameter.
	 * @param sizeInChars Number of characters in the buffer.
	 *
	 * @note
	 *   The input string must be UTF-8 encoded.
	 *   Unlike SetDefaultParams, which accepts WCHAR* input, this function uses char* because the JSON parser (nlohmann)
	 *   already requires conversion of original WCHAR* input from SQL to UTF-8, and so we can use this UTF-8 setting for setting m_embedding_param.
	 */
	void SetEmbeddingParam(char *param, size_t sizeInChars);

	// Method to clear the embedding parameter
	// This is required as embedding parameters can change between calls to GenerateSingleEmbedding.
	// It ensures that the previous embedding parameter is cleared before setting a new one.
	void ClearEmbeddingParam()
	{
		m_embedding_param.reset();
	}

	// Getter for DefaultParams
	const char *GetDefaultParams()
	{
		return m_defaultParams.get();
	}

	/**
	 * @brief Sets the default parameters for the ONNX Runtime session.
	 * @param params Pointer to the WCHAR buffer containing the default parameters. Must be UTF-16 (Windows) or UTF-32 (Linux).
	 * @param sizeInBytes Size of the buffer in bytes.
	 * @note The input WCHAR* param is converted to utf-8 char array for cross platform compatibility and stored in m_defaultParams.
	 */
	void SetDefaultParams(WCHAR *params, size_t sizeInBytes);

	// Getter and setter for output_str
	std::vector<std::wstring> GetOutputStr()
	{
		return m_output_str;
	}

	void SetOutputStr(std::vector<std::wstring> output)
	{
		m_output_str = output;
	}

	// Getter and setter for local runtime path
	const char *GetLocalRuntimePath()
	{
		return m_localRuntimePath.get();
	}

	/**
	 * @brief Sets the local runtime path for the ONNX Runtime session.
	 * @param localRuntimePath Pointer to the char buffer containing the runtime path. Must be UTF-8 encoded.
	 * @param sizeInChar Number of characters in the buffer.
	 * @note The input string must be UTF-8 encoded.
	 */
	void SetLocalRuntimePath(char *localRuntimePath, size_t sizeInChar);

	
#ifdef _WIN32
	// Module handle for the tokenizer DLL
	HMODULE m_HMTokenizerDll = nullptr;
#else
	void* m_HMTokenizerDll = nullptr;
#endif

private:
	// Path to the ONNX model file
	// This should be set to the path of your ONNX model file.
	// For example: L"C:\\path\\to\\your\\model.onnx"
	std::unique_ptr<char[]> m_modelLocation;

	// Function pointer for loading the tokenizer and encoding the input text
#ifdef _WIN32
	PFN_LoadBlobJsonAndEncode m_pFnLoadBlobJsonAndEncode = nullptr;
#else
	PFN_LoadBlobJsonAndEncode m_pFnLoadBlobJsonAndEncode = nullptr;
#endif

	// ONNX Runtime environment
	OrtEnv *m_env = nullptr;

	// ONNX Runtime session
	OrtSession *m_session = nullptr;

	// ONNX Runtime session options
	OrtSessionOptions *m_session_options = nullptr;

	// Input string for which vector embeddings will be generated
	// Tokenizers usually work on a non-wide string input only and hence this is a char pointer.
	std::unique_ptr<char[]> m_embedding_param;

	// Default parameters for the model
	// This should be set to the default parameters for your model.
	// For example: L"{\"VOCAB_FILE_PATH\": \"C:\\path\\to\\your\\vocab.txt\"}"
	std::unique_ptr<char[]> m_defaultParams;

	// Local runtime path
	std::unique_ptr<char[]> m_localRuntimePath;

	// Store output embedding as a vector of wide strings
	std::vector<std::wstring> m_output_str;

	// Path to the ONNX model file
	std::string m_onnxModelFilePath;

	// Path to the tokenizer.json file
	std::string m_onnxTokenizerFilePath;

	// Path to the tokenizer DLL file
	std::string m_onnxTokenizerDLLFilePath;

	/**
	 * @brief Tokenizes the input string and runs the ONNX model to generate embeddings.
	 *
	 * @param userInputString Input text to be tokenized and embedded. Must be UTF-8 encoded.
	 * @param output_str Output wide string to store the generated embeddings (as a JSON array or similar format).
	 * @param fVerboseOutput Optional flag for verbose output/logging (default: false).
	 *
	 * @return true on success, false on failure.
	 *
	 * @note The input string must be UTF-8 encoded. The output is a wide string which is later returned to SQL via GetOutputParam() based on platform.
	 */
	bool TokenizeAndGenerateEmbeddings(
		_In_ std::string userInputString,
		_Out_ std::wstring &output_str,
		_In_opt_ bool fVerboseOutput = false);

	// Helpers for SetupOnnxSession steps:
	// Resolves and validates all required file paths for the ONNX session, including the model, tokenizer JSON, and tokenizer DLL/shared library.
	bool ResolvePaths();
	// Configures ONNX Runtime session options such as optimization level, threading, and execution providers (CPU/GPU).
	bool ConfigureSessionOptions();
	// Loads the ONNX model file into a session, preparing it for inference.
	bool LoadModel();
	// Loads the tokenizer DLL/shared library and retrieves the function pointer for tokenization.
	bool LoadTokenizer();
};