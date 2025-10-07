// Linux-specific implementation for OnnxSession
#include <iostream>
#include <vector>
#include <cassert>
#include <sstream>
#include <filesystem>
// posix headers
#include <dlfcn.h>

#include <sql.h>
#include <sqlext.h>

#include "OnnxSession.h"
#include "util.h"
#include "OrtApis.h"

//--------------------------------------------------------------------------------------------
// Name: OnnxSession::ConfigureSessionOptions
//
// Description:
//   Configures the ONNX Runtime session options for the current session.
//   Attempts to enable the CUDA execution provider if available, otherwise falls back to CPU provider.
//   Logs which provider is used. Returns false if no provider can be set.
//
// Returns:
//   true if session options are configured and a provider is set, false otherwise.
//
bool OnnxSession::ConfigureSessionOptions()
{
#ifdef _DEBUG
	CheckStatus(g_apiFuncs.SetGraphOptimizationLevel(m_session_options, ORT_ENABLE_BASIC));
	CheckStatus(g_apiFuncs.SetIntraOpNumThreads(m_session_options, 1));
	CheckStatus(g_apiFuncs.SetExecutionMode(m_session_options, ORT_SEQUENTIAL));
	CheckStatus(g_apiFuncs.SetLogSeverityLevel(m_session_options, ORT_LOGGING_LEVEL_VERBOSE));
#else
	CheckStatus(g_apiFuncs.SetGraphOptimizationLevel(m_session_options, ORT_ENABLE_ALL));
	CheckStatus(g_apiFuncs.SetIntraOpNumThreads(m_session_options, 4));
	CheckStatus(g_apiFuncs.SetExecutionMode(m_session_options, ORT_PARALLEL));
	CheckStatus(g_apiFuncs.SetLogSeverityLevel(m_session_options, ORT_LOGGING_LEVEL_WARNING));
#endif
	bool use_gpu = false;
	if (g_apiFuncs.AppendExecutionProviderCUDA)
	{
		OrtCUDAProviderOptions cuda_options;
		cuda_options.device_id = 0;
		if (g_apiFuncs.AppendExecutionProviderCUDA(m_session_options, &cuda_options) == nullptr)
		{
			use_gpu = true;
			LogMessage(L"Using CUDA execution provider", LogLevel::INFO);
		}
		else
		{
			LogMessage(L"Failed to append CUDA execution provider", LogLevel::WARNING);
		}
	}
	if (!use_gpu)
	{
		if (g_apiFuncs.AppendExecutionProviderCPU(m_session_options, 0) == nullptr)
		{
			LogMessage(L"Using CPU execution provider", LogLevel::INFO);
		}
		else
		{
			return false;
		}
	}
	return true;
}

//--------------------------------------------------------------------------------------------
// Name: OnnxSession::LoadModel
//
// Description:
//   Loads the ONNX model from the resolved model file path into the ONNX Runtime session.
//   On Linux, uses the UTF-8 path directly. Throws on error.
//
// Returns:
//   true if the model is loaded successfully, false otherwise.
//
bool OnnxSession::LoadModel()
{
	CheckStatus(g_apiFuncs.CreateSession(m_env, m_onnxModelFilePath.c_str(), m_session_options, &m_session));
	return true;
}

//--------------------------------------------------------------------------------------------
// Name: OnnxSession::LoadTokenizer
//
// Description:
//   Loads the tokenizer DLL/shared library from the resolved path and retrieves the function pointer
//   for tokenization (LoadBlobJsonAndEncode). On Windows, loads the DLL using LoadLibraryExW and gets
//   the function address with GetProcAddress. On POSIX, uses dlopen/dlsym. Logs and returns false if
//   loading fails or the function is not found.
//
// Returns:
//   true if the tokenizer library is loaded and the function pointer is set, false otherwise.
//
bool OnnxSession::LoadTokenizer()
{
	m_HMTokenizerDll = dlopen(m_onnxTokenizerDLLFilePath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
	if (!m_HMTokenizerDll)
	{
		LogMessage("Failed to load the shared library: " + std::string(dlerror()), LogLevel::CRITICAL_ERROR);
		return false;
	}

	m_pFnLoadBlobJsonAndEncode = (PFN_LoadBlobJsonAndEncode)dlsym(m_HMTokenizerDll, "LoadBlobJsonAndEncode");
	const char *error = dlerror();
	if (error != NULL)
	{
		LogMessage("Failed to get function address \"void LoadBlobJsonAndEncode(const std::string &, const std::string &, std::vector<int32_t> &)\": " + std::string(error), LogLevel::CRITICAL_ERROR);
		dlclose(m_HMTokenizerDll);
		m_HMTokenizerDll = nullptr;
		return false;
	}
	return true;
}
