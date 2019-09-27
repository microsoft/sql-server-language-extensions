
#ifdef _WIN64
#include <windows.h>
#endif
#include <iostream>
#include <string>
#include <sqltypes.h>
#include <ctime>
#include "Utilities.h"
#include "Logger.h"

void* Utilities::GetFunctionFromModule(const void * pHDLL, const std::string fnName)
{

#if defined ( _MSC_VER )
    // this will be the pointer to the call back function
    void *pExportedCallBack = NULL;
    if (pHDLL != nullptr)
    {
        HMODULE hDLL = *(HMODULE *)pHDLL;
        if (hDLL != NULL) {

            pExportedCallBack = GetProcAddress(hDLL, fnName.c_str());

            if (!pExportedCallBack)
            {
                LOG_ERROR("GetProcAddress for '" + fnName + "' in dll failed.");
                FreeLibrary(hDLL);
            }
        }
        else {
            LOG_ERROR("Received null module in GetFunctionFromModule.");
        }
    }
    else
    {
        LOG_ERROR("Received null dll in GetFunctionFromModule.");
    }

    return pExportedCallBack;
#else
    void *lib_handle = pHDLL;
    void *fn;

    lib_handle = dlopen(dllPath, RTLD_LAZY);

    if (!lib_handle)
    {
        std::string errString = "Unable to load library ";
        errString += dllPath;
        LOG_ERROR(errString.c_str());
        return (void*)NULL;
    }

    fn = (void*)dlsym(lib_handle, fnName);

    char* error = NULL;
    if ((error = dlerror()) != NULL)
    {
        std::string errString = "Error loading library: ";
        errString += dllPath;
        errString += "   function: ";
        errString += fnName;
        LOG_ERROR(errString.c_str());
        return (void*)NULL;
    }

    return fn;
#endif
}


void* Utilities::GetFunctionFromDLL(const std::string dllPath, const std::string fnName)
{

#if defined ( _MSC_VER )
    // this will be the pointer to the call back function
    void *pExportedCallBack = NULL;

    HMODULE hDLL = LoadLibraryExA(dllPath.c_str(), nullptr, 0);
    if (hDLL != NULL)
    {
        pExportedCallBack = GetProcAddress(hDLL, fnName.c_str());

        if (!pExportedCallBack)
        {
            LOG_ERROR("GetProcAddress for '" + fnName + "' in '" + dllPath + "' failed.");
            FreeLibrary(hDLL);
        }
    }
    else
    {
        LOG_ERROR("Failed to load '" + dllPath + "'.");
    }

    return pExportedCallBack;
#else
    void *lib_handle;
    void *fn;

    lib_handle = dlopen(dllPath, RTLD_LAZY);

    if (!lib_handle)
    {
        std::string errString = "Unable to load library ";
        errString += dllPath;
        LOG_ERROR(errString.c_str());
        return (void*)NULL;
    }

    fn = (void*)dlsym(lib_handle, fnName);

    char* error = NULL;
    if ((error = dlerror()) != NULL)
    {
        std::string errString = "Error loading library: ";
        errString += dllPath;
        errString += "   function: ";
        errString += fnName;
        LOG_ERROR(errString.c_str());
        return (void*)NULL;
    }

    return fn;
#endif
}


void* Utilities::GetDLL(const std::string dllPath) {

#if defined ( _MSC_VER )
    // this will be the pointer to the call back function
    void *pExportedCallBack = NULL;

    HMODULE hDLL = LoadLibraryExA(dllPath.c_str(), nullptr, 0);

    if (hDLL == NULL)
    {
        LOG_ERROR("Failed to load '" + dllPath + "'.");
    }
    return hDLL;
#else
    void *lib_handle;
    void *fn;

    lib_handle = dlopen(dllPath, RTLD_LAZY);

    return lib_handle;
#endif
}


//--------------------------------------------------------------------------------------------------
// Name: Utilities::ConvertGuidToString
//
// Description:
//  Converts a SQLGUID to a string
//
// Returns:
//	string of the guid
//
std::string Utilities::ConvertGuidToString(_In_ const SQLGUID *guid)
{
    // 32 hex chars + 4 hyphens + null terminator, so 37 characters.
    //
    char guidString[37];
#ifdef _WIN64
    sprintf_s(guidString, sizeof(guidString) / sizeof(guidString[0]),
        GuidFormat,
        guid->Data1, guid->Data2, guid->Data3,
        guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
        guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
#elif __linux__
    snprintf(guidString, sizeof(guidString) / sizeof(guidString[0]),
        GuidFormat,
        static_cast<unsigned long>(guid->Data1), guid->Data2, guid->Data3,
        guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
        guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
#else
    throw "The current platform is not supported.";
#endif

    std::string s(guidString);
    return s;
}