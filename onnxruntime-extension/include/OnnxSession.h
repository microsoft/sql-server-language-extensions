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
typedef void (__cdecl *PFN_LoadBlobJsonAndEncode)(const std::string &, const std::string &, std::vector<int32_t> &);

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

// ONNX Runtime headers (adjust path based on your setup)
#include <onnxruntime_c_api.h>

// This structure defines constant keys used for accessing default parameters related to the ONNX
// session. These keys are typically used to retrieve file names for the tokenizer JSON, tokenizer
// DLL, and the ONNX model from a configuration or parameter set.
struct DEFAULT_PARAMETERS_KEYS
{
	// Key for the tokenizer JSON file name parameter
	const WCHAR *TOKENIZER_JSON_FILE_NAME = L"ONNX_TOKENIZER_JSON_FILE_NAME";

	// Key for the tokenizer DLL file name parameter
	const WCHAR *TOKENIZER_DLL_FILE_NAME = L"ONNX_TOKENIZER_DLL_FILE_NAME";

	// Key for the ONNX model file name parameter
	const WCHAR *ONNX_MODEL_FILE_NAME = L"ONNX_MODEL_FILE_NAME";
};

// Global constant instance of DEFAULT_PARAMETERS_KEYS for use throughout the codebase.
const struct DEFAULT_PARAMETERS_KEYS DEFAULT_PARAMETERS_KEYS;

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
	OnnxSession(const CHAR *LocalRuntimePath);

	// Destructor
	~OnnxSession();

	// Method to load the ONNX model and set up the session
	BOOL SetupOnnxSession();

	// Method to generate embeddings from input text
	// Returns a boolean indicating success or failure.
	BOOL GenerateSingleEmbedding();

	// Method to get the model location
	const WCHAR *GetModelLocation()
	{
		return m_modelLocation.get();
	}

	// Method to set the model location
	void SetModelLocation(WCHAR *modelLocation, size_t cbModelLocation);

	// Getter for m_embedding_param
	const CHAR *GetEmbeddingParam()
	{
		return m_embedding_param.get();
	}

	// Setter for m_embedding_param
	// This function assumes that any CHAR* input is UTF-8 encoded. Ensure that all CHAR* strings
	// passed to this function are properly encoded as UTF-8.
	void SetEmbeddingParam(CHAR *param, size_t sizeInChars);

	// Method to clear the embedding parameter
	// This is required as embedding parameters can change between calls to GenerateSingleEmbedding.
	// It ensures that the previous embedding parameter is cleared before setting a new one.
	void ClearEmbeddingParam()
	{
		m_embedding_param.reset();
	}

	// Getter for DefaultParams
	const WCHAR *GetDefaultParams()
	{
		return m_defaultParams.get();
	}

	// Setter for DefaultParams
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
	const WCHAR *GetLocalRuntimePath()
	{
		return m_localRuntimePath.get();
	}

	void SetLocalRuntimePath(CHAR *localRuntimePath, size_t sizeInChar);

public:
	// Module handle for the tokenizer DLL
	HMODULE m_HMTokenizerDll = nullptr;

private:
	// Path to the ONNX model file
	// This should be set to the path of your ONNX model file.
	// For example: L"C:\\path\\to\\your\\model.onnx"
	std::unique_ptr<WCHAR[]> m_modelLocation;

	// Function pointer for loading the tokenizer and encoding the input text
	PFN_LoadBlobJsonAndEncode m_pFnLoadBlobJsonAndEncode = nullptr;

	// ONNX Runtime environment
	OrtEnv *m_env = nullptr;

	// ONNX Runtime session
	OrtSession *m_session = nullptr;

	// ONNX Runtime session options
	OrtSessionOptions *m_session_options = nullptr;

	// Input string for which vector embeddings will be generated
	// Tokenizers usually work on a non-wide string input only and hence this is a CHAR pointer.
	std::unique_ptr<CHAR[]> m_embedding_param;

	// Default parameters for the model
	// This should be set to the default parameters for your model.
	// For example: L"{\"VOCAB_FILE_PATH\": \"C:\\path\\to\\your\\vocab.txt\"}"
	std::unique_ptr<WCHAR[]> m_defaultParams;

	// Local runtime path
	std::unique_ptr<WCHAR[]> m_localRuntimePath;

	// Store output embedding as a vector of wide strings
	std::vector<std::wstring> m_output_str;

	// Path to the ONNX model file
	std::wstring m_onnxModelFilePath;

	// Path to the tokenizer.json file
	std::wstring m_onnxTokenizerFilePath;

	// Path to the tokenizer DLL file
	std::wstring m_onnxTokenizerDLLFilePath;

	// Method to tokenize input text and generate embeddings
	BOOL TokenizeAndGenerateEmbeddings(
		_In_ std::string userInputString,
		_Out_ std::wstring &output_str,
		_In_opt_ BOOL fVerboseOutput = false);
};