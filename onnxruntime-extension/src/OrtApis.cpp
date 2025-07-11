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
#include <windows.h>
#include <onnxruntime_c_api.h>
#include "util.h"
#include "OrtApis.h"

// Global variable to hold the ONNX Runtime library handle
static HMODULE g_ortLib = nullptr;

// Flag to indicate if the ONNX Runtime library has been loaded
static BOOL fOnnxRuntimeDllLoaded = false;

// Global variables to hold the ONNX Runtime API functions
PFN_OrtGetApiBase g_getApiBase = nullptr;
const OrtApi *g_ortApi = nullptr;
OrtApiFunctions g_apiFuncs = {};

// Define the ONNX Runtime DLL name
static const WCHAR *ONNX_RUNTIME_DLL_NAME = L"onnxruntime.dll";

// Helper to check ONNX Runtime status
void CheckStatus(OrtStatus *status)
{
	if (status != nullptr)
	{
		if (!g_ortApi) {
			assert(false && "g_ortApi is not initialized");
			return;
		}
		std::string error = g_ortApi->GetErrorMessage(status);
		g_ortApi->ReleaseStatus(status);
		throw std::runtime_error("ONNX Runtime API call failed: " + error);
	}
}


// Macro to resolve and check ONNX Runtime API functions
#define RESOLVE_API_FUNC(apiFuncs, ortApi, func, member) \
	apiFuncs.member = reinterpret_cast<decltype(apiFuncs.member)>(ortApi->func); \
	if (!apiFuncs.member) { \
		LogMessage(L"Failed to resolve " L#member, LogLevel::CRITICAL_ERROR); \
		return false; \
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

// Loads the ONNX Runtime DLL and resolves the API functions.
// Returns 0 on success, 1 on failure.
int LoadOnnxDll(const std::wstring modelPath)
{
	// Use std::filesystem to construct the DLL path properly
	std::filesystem::path onnxDllPath = std::filesystem::path(modelPath) / ONNX_RUNTIME_DLL_NAME;

	// Load onnxruntime.dll
	g_ortLib = LoadLibraryW(onnxDllPath.c_str());
	if (!g_ortLib)
	{
		LogMessage(L"Failed to load onnxruntime.dll: " + std::to_wstring(GetLastError()), LogLevel::CRITICAL_ERROR);

		// Print the error message for common error codes
		switch (GetLastError())
		{
		case ERROR_MOD_NOT_FOUND:
			LogMessage(L"onnxruntime.dll not found. Please ensure it is in the correct directory.", LogLevel::CRITICAL_ERROR);
			break;
		case ERROR_INVALID_PARAMETER:
			LogMessage(L"Invalid parameter passed to LoadLibraryW. Please check the path.", LogLevel::CRITICAL_ERROR);
			break;
		case ERROR_ACCESS_DENIED:
			LogMessage(L"Access denied when trying to load onnxruntime.dll. Please check file permissions.", LogLevel::CRITICAL_ERROR);
			break;
		default:
			LogMessage(L"An unknown error occurred while loading onnxruntime.dll.", LogLevel::CRITICAL_ERROR);
			break;
		}

		return 1;
	}

	// Set the flag to indicate the DLL is loaded
	fOnnxRuntimeDllLoaded = true;

	// Get the ONNX Runtime API
	g_getApiBase = reinterpret_cast<PFN_OrtGetApiBase>(GetProcAddress(g_ortLib, "OrtGetApiBase"));
	if (!g_getApiBase)
	{
		LogMessage(L"Failed to get OrtGetApiBase.", LogLevel::CRITICAL_ERROR);
		FreeLibrary(g_ortLib);
		return 1;
	}

	g_ortApi = g_getApiBase()->GetApi(ORT_API_VERSION);
	LogMessage(L"ONNX Runtime API version: " + std::to_wstring(ORT_API_VERSION), LogLevel::INFO);

	if (!g_ortApi)
	{
		LogMessage(L"Failed to get ONNX Runtime API interface.", LogLevel::CRITICAL_ERROR);
		FreeLibrary(g_ortLib);
		return 1;
	}

	// Resolve all API functions of struct OrtApi
	if (!ResolveApiFunctions(g_ortApi, g_apiFuncs))
	{
		LogMessage(L"Failed to resolve ONNX Runtime API functions", LogLevel::CRITICAL_ERROR);
		assert(g_ortLib != nullptr);
		FreeLibrary(g_ortLib);
		return 1;
	}

	// Resolve OrtSessionOptionsAppendExecutionProviderCPU method which is not part of OrtApi
	g_apiFuncs.AppendExecutionProviderCPU = reinterpret_cast<PFN_OrtSessionOptionsAppendExecutionProviderCPU>(GetProcAddress(g_ortLib, "OrtSessionOptionsAppendExecutionProvider_CPU"));
	if (!g_apiFuncs.AppendExecutionProviderCPU)
	{
		LogMessage(L"Failed to resolve OrtSessionOptionsAppendExecutionProvider_CPU method", LogLevel::CRITICAL_ERROR);
		FreeLibrary(g_ortLib);
		return 1;
	}

	g_apiFuncs.AppendExecutionProviderCUDA = reinterpret_cast<PFN_OrtSessionOptionsAppendExecutionProviderCUDA>(GetProcAddress(g_ortLib, "OrtSessionOptionsAppendExecutionProvider_CUDA"));
	// If AppendExecutionProviderCUDA is not available then fallback to CPU provider
	if (!g_apiFuncs.AppendExecutionProviderCUDA)
	{
		LogMessage(L"OrtSessionOptionsAppendExecutionProvider_CUDA not available, falling back to CPU provider", LogLevel::INFO);
	}
	else
	{
		LogMessage(L"OrtSessionOptionsAppendExecutionProvider_CUDA resolved successfully", LogLevel::INFO);
	}

	return 0;
}

// Unloads the ONNX Runtime DLL and cleans up resources.
// This function is idempotent and safe to call multiple times.
void UnloadOnnxDll()
{
	if (!fOnnxRuntimeDllLoaded)
		return;

	// Set API pointers to nullptr
	g_ortApi = nullptr;
	g_getApiBase = nullptr;
	memset(&g_apiFuncs, 0, sizeof(g_apiFuncs));

	// Free the library if it was loaded
	if (g_ortLib)
	{
		FreeLibrary(g_ortLib);
		g_ortLib = nullptr;
	}

	fOnnxRuntimeDllLoaded = false;
}