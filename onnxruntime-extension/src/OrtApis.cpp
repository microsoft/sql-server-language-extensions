//**************************************************************************************************
// ONNXRuntime-extension : A language extension implementing the SQL Server AIRuntime extension
// for ONNX Runtime.
// Copyright (C) 2025 Microsoft Corporation.
//
// @File: OrtApis.cpp
//
// Purpose:
//  This file defines the exported methods from the onnxruntime library which are dynamically
//  loaded at runtime. It includes function pointers for the ONNX Runtime API functions that are
//  used in the OnnxSession class.
//
//**************************************************************************************************

#include <string>
#include <filesystem>
#include <assert.h>
#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include <onnxruntime_c_api.h>
#include "util.h"
#include "OrtApis.h"


// Global variable to hold the ONNX Runtime library handle
#if defined(_WIN32)
HMODULE g_ortLib = nullptr;
#else
void *g_ortLib = nullptr;
#endif

// Global variables to hold the ONNX Runtime API functions
PFN_OrtGetApiBase g_getApiBase = nullptr;
const OrtApi *g_ortApi = nullptr;
OrtApiFunctions g_apiFuncs = {};

// Define the ONNX Runtime DLL name
const char *ONNX_RUNTIME_DLL_NAME = "onnxruntime.dll";

// Define the ONNX Runtime so name in linux
const char *ONNX_RUNTIME_SO_NAME = "libonnxruntime.so.1";

// Flag to indicate if the ONNX Runtime library has been loaded
bool isOnnxRuntimeDllLoaded = false;

// Helper to check ONNX Runtime status
void CheckStatus(OrtStatus *status)
{
	if (status != nullptr)
	{
		if (!g_ortApi)
		{
			assert(false && "g_ortApi is not initialized");
			return;
		}
		std::string error = g_ortApi->GetErrorMessage(status);
		g_ortApi->ReleaseStatus(status);
		throw std::runtime_error("ONNX Runtime API call failed: " + error);
	}
}

// Macro to resolve and check ONNX Runtime API functions
#define RESOLVE_API_FUNC(apiFuncs, ortApi, func, member)                                   \
	apiFuncs.member = reinterpret_cast<decltype(apiFuncs.member)>(ortApi->func);           \
	if (!apiFuncs.member)                                                                  \
	{                                                                                      \
		LogMessage(std::string("Failed to resolve ") + #member, LogLevel::CRITICAL_ERROR); \
		return false;                                                                      \
	}

// Function to resolve ONNX Runtime API functions
// All the methods should be resolved (except AppendExecutionProviderCUDA which is optional).
// Returns true if all functions are resolved successfully, false otherwise.
bool ResolveApiFunctions(const OrtApi *ortApi, OrtApiFunctions &apiFuncs)
{
	RESOLVE_API_FUNC(apiFuncs, ortApi, CreateEnv, CreateEnv);
	RESOLVE_API_FUNC(apiFuncs, ortApi, CreateSession, CreateSession);
	RESOLVE_API_FUNC(apiFuncs, ortApi, CreateSessionOptions, CreateSessionOptions);
	RESOLVE_API_FUNC(apiFuncs, ortApi, Run, Run);
	RESOLVE_API_FUNC(apiFuncs, ortApi, CreateTensorWithDataAsOrtValue, CreateTensorWithDataAsOrtValue);
	RESOLVE_API_FUNC(apiFuncs, ortApi, ReleaseSession, ReleaseSession);
	RESOLVE_API_FUNC(apiFuncs, ortApi, ReleaseEnv, ReleaseEnv);
	RESOLVE_API_FUNC(apiFuncs, ortApi, ReleaseSessionOptions, ReleaseSessionOptions);
	RESOLVE_API_FUNC(apiFuncs, ortApi, ReleaseValue, ReleaseValue);
	RESOLVE_API_FUNC(apiFuncs, ortApi, SetSessionGraphOptimizationLevel, SetGraphOptimizationLevel);
	RESOLVE_API_FUNC(apiFuncs, ortApi, SetIntraOpNumThreads, SetIntraOpNumThreads);
	RESOLVE_API_FUNC(apiFuncs, ortApi, SetSessionExecutionMode, SetExecutionMode);
	RESOLVE_API_FUNC(apiFuncs, ortApi, SetSessionLogSeverityLevel, SetLogSeverityLevel);

	return true;
}

// Forward declarations of helper functions
// Platform-specific implementation in platform-specific files.

// Loads the ONNX Runtime DLL/shared library and sets the global handle.
// Returns true on success, false on failure.
bool LoadLibraryHandle(const std::string &runtimePath);

// Resolves the OrtGetApiBase symbol from the loaded DLL and sets the global pointer
bool ResolveApiBase();

// Resolves all ONNX Runtime API functions and sets them in g_apiFuncs.
// Returns true on success, false on failure.
bool ResolveApiFunctionsGlobal();

// Resolves and configures execution provider function pointers.
// Returns true on success, false on failure.
bool ConfigureExecutionProviders();

// Unloads the ONNX Runtime DLL and cleans up all related global resources.
void UnloadOnnxDll();

// Loads the ONNX Runtime DLL and resolves the API functions.
// Returns 0 on success, 1 on failure.
int LoadOnnxDll(const std::string runtimePath)
{
	// Step 1: Load the ONNX Runtime DLL/shared library and set the global handle.
	if (!LoadLibraryHandle(runtimePath))
		return 1;

	// Step 2: Resolve the OrtGetApiBase symbol and set the global pointer.
	if (!ResolveApiBase())
		return 1;

	// Step 3: Resolve all ONNX Runtime API functions and set them in g_apiFuncs.
	if (!ResolveApiFunctionsGlobal())
		return 1;

	// Step 4: Resolve and configure execution provider function pointers.
	if (!ConfigureExecutionProviders())
		return 1;

	// All steps succeeded.
	return 0;
}
