//**************************************************************************************************
// ONNXRuntime-extension : A language extension implementing the SQL Server AIRuntime extension
// for ONNX Runtime.
// Copyright (C) 2025 Microsoft Corporation.
//
// @File: OrtApis.h
//
// Purpose:
//  This file defines the exported methods from the onnxruntime library which are dynamically
//  loaded at runtime. It includes function pointers for the ONNX Runtime API functions that are
//  used in the OnnxSession class.
//  This file must be updated for any changes in the onnxruntime library APIs.
//  Currently tested versions: 1.19.0 - 1.21.0
//
//**************************************************************************************************

#pragma once

#include <string>
#include <onnxruntime_c_api.h>

inline constexpr const char* MIN_SUPPORTED_VERSION = "1.19.0";

// Function pointers for ONNX Runtime API

// === API Base ===
using PFN_OrtGetApiBase = const OrtApiBase *(ORT_API_CALL *)();

// === Environment Management ===
using PFN_OrtCreateEnv = OrtStatus *(ORT_API_CALL *)(OrtLoggingLevel, const char *, OrtEnv **);
using PFN_OrtReleaseEnv = void(ORT_API_CALL *)(OrtEnv *);

// === Session Management ===
// The signature of OrtCreateSession is platform-specific: uses wchar_t* for Windows, char* for Linux/macOS.
#ifdef _WIN32
using PFN_OrtCreateSession = OrtStatus *(ORT_API_CALL *)(OrtEnv *, const wchar_t *, const OrtSessionOptions *, OrtSession **);
#else
using PFN_OrtCreateSession = OrtStatus *(ORT_API_CALL *)(OrtEnv *, const char *, const OrtSessionOptions *, OrtSession **);
#endif
using PFN_OrtCreateSessionOptions = OrtStatus *(ORT_API_CALL *)(OrtSessionOptions **);
using PFN_OrtReleaseSession = void(ORT_API_CALL *)(OrtSession *);
using PFN_OrtReleaseSessionOptions = void(ORT_API_CALL *)(OrtSessionOptions *);

// === Session Options Configuration ===
using PFN_OrtSessionOptionsSetGraphOptimizationLevel = OrtStatus *(ORT_API_CALL *)(OrtSessionOptions *, GraphOptimizationLevel);
using PFN_OrtSessionOptionsSetIntraOpNumThreads = OrtStatus *(ORT_API_CALL *)(OrtSessionOptions *, int);
using PFN_OrtSessionOptionsSetExecutionMode = OrtStatus *(ORT_API_CALL *)(OrtSessionOptions *, ExecutionMode);
using PFN_OrtSetSessionLogSeverityLevel = OrtStatus *(ORT_API_CALL *)(OrtSessionOptions *, int);

// === Tensor and Value Management ===
using PFN_OrtCreateTensorWithDataAsOrtValue = OrtStatus *(ORT_API_CALL *)(const OrtMemoryInfo *, void *, size_t, const int64_t *, size_t, ONNXTensorElementDataType, OrtValue **);
using PFN_OrtReleaseValue = void(ORT_API_CALL *)(OrtValue *);

// === Model Execution ===
using PFN_OrtRun = OrtStatus *(ORT_API_CALL *)(OrtSession *, const OrtRunOptions *, const char *const *, const OrtValue *const *, size_t, const char *const *, size_t, OrtValue **);

// === Execution Provider ===
using PFN_OrtSessionOptionsAppendExecutionProviderCPU = OrtStatus *(ORT_API_CALL *)(OrtSessionOptions *, int);
using PFN_OrtSessionOptionsAppendExecutionProviderCUDA = OrtStatus *(ORT_API_CALL *)(OrtSessionOptions *, const OrtCUDAProviderOptions *);


// Structure to hold resolved API functions
struct OrtApiFunctions
{
	PFN_OrtCreateEnv CreateEnv;
	PFN_OrtCreateSession CreateSession;
	PFN_OrtCreateSessionOptions CreateSessionOptions;
	PFN_OrtRun Run;
	PFN_OrtCreateTensorWithDataAsOrtValue CreateTensorWithDataAsOrtValue;
	PFN_OrtReleaseSession ReleaseSession;
	PFN_OrtReleaseEnv ReleaseEnv;
	PFN_OrtReleaseSessionOptions ReleaseSessionOptions;
	PFN_OrtReleaseValue ReleaseValue;
	PFN_OrtSessionOptionsSetGraphOptimizationLevel SetGraphOptimizationLevel;
	PFN_OrtSessionOptionsSetIntraOpNumThreads SetIntraOpNumThreads;
	PFN_OrtSessionOptionsSetExecutionMode SetExecutionMode;
	PFN_OrtSetSessionLogSeverityLevel SetLogSeverityLevel;
	PFN_OrtSessionOptionsAppendExecutionProviderCUDA AppendExecutionProviderCUDA;
	PFN_OrtSessionOptionsAppendExecutionProviderCPU AppendExecutionProviderCPU;
};

// Helper to check ONNX Runtime status
void CheckStatus(OrtStatus *status);

// Function to resolve ONNX Runtime API functions in struct OrtApi.
bool ResolveApiFunctions(const OrtApi *ortApi, OrtApiFunctions &apiFuncs);

// Loads the ONNX Runtime DLL/shared library and sets the global handle.
// Returns true on success, false on failure.
bool LoadLibraryHandle(const std::string& runtimePath);

// Resolves the OrtGetApiBase symbol and sets the global pointer.
// Returns true on success, false on failure.
bool ResolveApiBase();

// Resolves all ONNX Runtime API functions and sets them in g_apiFuncs.
// Returns true on success, false on failure.
bool ResolveApiFunctionsGlobal();

// Resolves and configures execution provider function pointers.
// Returns true on success, false on failure.
bool ConfigureExecutionProviders();

// Loads the ONNX Runtime DLL and resolves the API functions.
int LoadOnnxDll(const std::string runtimePath);

// Unloads the ONNX Runtime DLL and cleans up resources.
void UnloadOnnxDll();

// Global variables to hold the ONNX Runtime API functions and API interface
#if defined(_WIN32)
#include <windows.h>
extern HMODULE g_ortLib;
#else
#include <dlfcn.h>
extern void *g_ortLib;
#endif

extern bool isOnnxRuntimeDllLoaded;
extern PFN_OrtGetApiBase g_getApiBase;
extern const OrtApi *g_ortApi;
extern OrtApiFunctions g_apiFuncs;
extern const char *ONNX_RUNTIME_DLL_NAME;
extern const char *ONNX_RUNTIME_SO_NAME;