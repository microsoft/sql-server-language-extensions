//**************************************************************************************************
// ONNXRuntime-extension : Windows-specific OrtApis implementation
//**************************************************************************************************
#include "OrtApis.h"
#include <windows.h>
#include <string>
#include <filesystem>
#include <assert.h>
#include <onnxruntime_c_api.h>
#include "util.h"
// Windows headers
#include <windows.h>
#include <strsafe.h>

// SQL headers (must come after strsafe.h)
#include <sql.h>
#include <sqlext.h>

// Loads the ONNX Runtime DLL from the specified path and sets the global handle.
bool LoadLibraryHandle(const std::string &runtimePath)
{
	std::filesystem::path onnxDllPath = std::filesystem::path(runtimePath) / ONNX_RUNTIME_DLL_NAME;
	std::wstring wDllPath = onnxDllPath.wstring();
	g_ortLib = LoadLibraryW(wDllPath.c_str());
	if (!g_ortLib)
	{
		LogMessage("Failed to load onnxruntime.dll: " + std::to_string(GetLastError()), LogLevel::CRITICAL_ERROR);
		switch (GetLastError())
		{
		case ERROR_MOD_NOT_FOUND:
			LogMessage("onnxruntime.dll not found. Please ensure it is in the correct directory.", LogLevel::CRITICAL_ERROR);
			break;
		case ERROR_INVALID_PARAMETER:
			LogMessage("Invalid parameter passed to LoadLibraryW. Please check the path.", LogLevel::CRITICAL_ERROR);
			break;
		case ERROR_ACCESS_DENIED:
			LogMessage("Access denied when trying to load onnxruntime.dll. Please check file permissions.", LogLevel::CRITICAL_ERROR);
			break;
		default:
			LogMessage("An unknown error occurred while loading onnxruntime.dll.", LogLevel::CRITICAL_ERROR);
			break;
		}
		return false;
	}
	return true;
}

// Resolves the OrtGetApiBase symbol from the loaded DLL and sets the global pointer.
bool ResolveApiBase()
{
	bool success = true;

	g_getApiBase = reinterpret_cast<PFN_OrtGetApiBase>(GetProcAddress(g_ortLib, "OrtGetApiBase"));

	if (!g_getApiBase)
	{
		LogMessage("Failed to get OrtGetApiBase.", LogLevel::CRITICAL_ERROR);
		success = false;
	}

	// Version compatibility check
	if (success)
	{
		const char *ortVersion = g_getApiBase()->GetVersionString();
		if (std::string(ortVersion) < MIN_SUPPORTED_VERSION)
		{
			LogMessage(std::string("ONNX Runtime version not supported: ") + ortVersion + ". Minimum required: " + MIN_SUPPORTED_VERSION, LogLevel::CRITICAL_ERROR);
			success = false;
		}
	}

	if (success)
	{
		g_ortApi = g_getApiBase()->GetApi(ORT_API_VERSION);
		LogMessage("ONNX Runtime API version: " + std::to_string(ORT_API_VERSION), LogLevel::INFO);
		if (!g_ortApi)
		{
			LogMessage("Failed to get ONNX Runtime API interface.", LogLevel::CRITICAL_ERROR);
			success = false;
		}
	}

	if (!success)
	{
		FreeLibrary(g_ortLib);
		g_ortLib = nullptr;
		return false;
	}
	return true;
}

// Configures available execution providers (e.g., CUDA, CPU) for ONNX Runtime sessions.
bool ConfigureExecutionProviders()
{
	g_apiFuncs.AppendExecutionProviderCPU = reinterpret_cast<PFN_OrtSessionOptionsAppendExecutionProviderCPU>(GetProcAddress(g_ortLib, "OrtSessionOptionsAppendExecutionProvider_CPU"));
	if (!g_apiFuncs.AppendExecutionProviderCPU)
	{
		LogMessage("Failed to resolve OrtSessionOptionsAppendExecutionProvider_CPU method", LogLevel::CRITICAL_ERROR);
		FreeLibrary(g_ortLib);
		g_ortLib = nullptr;
		return false;
	}
	g_apiFuncs.AppendExecutionProviderCUDA = reinterpret_cast<PFN_OrtSessionOptionsAppendExecutionProviderCUDA>(GetProcAddress(g_ortLib, "OrtSessionOptionsAppendExecutionProvider_CUDA"));
	if (!g_apiFuncs.AppendExecutionProviderCUDA)
	{
		LogMessage("OrtSessionOptionsAppendExecutionProvider_CUDA not available, falling back to CPU provider", LogLevel::INFO);
	}
	else
	{
		LogMessage("OrtSessionOptionsAppendExecutionProvider_CUDA resolved successfully", LogLevel::INFO);
	}
	return true;
}

// Resolves all required ONNX Runtime API function pointers and stores them in a global struct.
bool ResolveApiFunctionsGlobal()
{
	if (!ResolveApiFunctions(g_ortApi, g_apiFuncs))
	{
		LogMessage("Failed to resolve ONNX Runtime API functions", LogLevel::CRITICAL_ERROR);
		assert(g_ortLib != nullptr);
		FreeLibrary(g_ortLib);
		g_ortLib = nullptr;
		return false;
	}
	return true;
}

// Unloads the ONNX Runtime DLL and cleans up all related global resources.
void UnloadOnnxDll()
{
	g_ortApi = nullptr;
	g_getApiBase = nullptr;
	memset(&g_apiFuncs, 0, sizeof(g_apiFuncs));
	if (g_ortLib)
	{
		FreeLibrary(g_ortLib);
		g_ortLib = nullptr;
	}
}
