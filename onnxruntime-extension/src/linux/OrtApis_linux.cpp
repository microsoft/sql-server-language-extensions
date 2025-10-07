//**************************************************************************************************
// ONNXRuntime-extension : Linux-specific OrtApis implementation
//**************************************************************************************************
#include <dlfcn.h>
#include <string>
#include <filesystem>
#include <assert.h>
#include "OrtApis.h"
#include "util.h"

// Loads the ONNX Runtime shared library from the specified path and sets the global handle.
bool LoadLibraryHandle(const std::string &runtimePath)
{
	std::filesystem::path onnxSoPath = std::filesystem::path(runtimePath) / ONNX_RUNTIME_SO_NAME;
	g_ortLib = dlopen(onnxSoPath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
	if (!g_ortLib)
	{
		LogMessage(std::string("Failed to load libonnxruntime.so.1: ") + dlerror(), LogLevel::CRITICAL_ERROR);
		return false;
	}
	return true;
}

// Resolves the OrtGetApiBase symbol from the loaded shared library and sets the global pointer.
bool ResolveApiBase()
{
	bool success = true;
	g_getApiBase = reinterpret_cast<PFN_OrtGetApiBase>(dlsym(g_ortLib, "OrtGetApiBase"));

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
		dlclose(g_ortLib);
		g_ortLib = nullptr;
		return false;
	}
	return true;
}

// Configures available execution providers (e.g., CUDA, CPU) for ONNX Runtime sessions.
bool ConfigureExecutionProviders()
{
	g_apiFuncs.AppendExecutionProviderCPU = reinterpret_cast<PFN_OrtSessionOptionsAppendExecutionProviderCPU>(dlsym(g_ortLib, "OrtSessionOptionsAppendExecutionProvider_CPU"));
	if (!g_apiFuncs.AppendExecutionProviderCPU)
	{
		LogMessage("Failed to resolve OrtSessionOptionsAppendExecutionProvider_CPU method", LogLevel::CRITICAL_ERROR);
		dlclose(g_ortLib);
		g_ortLib = nullptr;
		return false;
	}
	g_apiFuncs.AppendExecutionProviderCUDA = reinterpret_cast<PFN_OrtSessionOptionsAppendExecutionProviderCUDA>(dlsym(g_ortLib, "OrtSessionOptionsAppendExecutionProvider_CUDA"));
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
		dlclose(g_ortLib);
		g_ortLib = nullptr;
		return false;
	}
	return true;
}

// Unloads the ONNX Runtime shared library and cleans up all related global resources.
void UnloadOnnxDll()
{
	g_ortApi = nullptr;
	g_getApiBase = nullptr;
	memset(&g_apiFuncs, 0, sizeof(g_apiFuncs));
	if (g_ortLib)
	{
		dlclose(g_ortLib);
		g_ortLib = nullptr;
	}
}
