//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: DotnetEnvironment.cpp
//
// Purpose:
//  Implement the .NET environment
//
//*********************************************************************
#include "DotnetEnvironment.h"
#include "Logger.h"

#ifdef _WIN32
#include "Windows.h"
#else
#include <dlfcn.h>
#include <unistd.h>
#include <nethost.h>
#endif

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <assert.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>

using namespace std;
using string_t = std::basic_string<char_t>;

// Named constant for hostfxr path buffer size on Linux
#ifndef _WIN32
constexpr size_t HOSTFXR_PATH_BUFFER_SIZE = 4096;
#endif

//--------------------------------------------------------------------------------------------------
// Name: DotnetEnvironment::DotnetEnvironment
//
// Description:
// Constructor to initialize the members. The language path is defined as the root path.
//
DotnetEnvironment::DotnetEnvironment(
    std::string language_params,
    std::string language_path,
    std::string public_library_path,
    std::string private_library_path) : m_root_path(convert_string(language_path)),
    m_init_fptr(nullptr),
    m_init_cmdline_fptr(nullptr),
    m_get_delegate_fptr(nullptr),
    m_close_fptr(nullptr),
    m_load_assembly_and_get_function_pointer(nullptr),
    m_get_function_pointer(nullptr),
    m_is_self_contained(false)
{
}

//--------------------------------------------------------------------------------------------------
// Name: DotnetEnvironment::init
//
// Description:
// Initialize the runtime by reading the runtimeconfig.json file.
//
short DotnetEnvironment::Init()
{
    LOG("DotnetEnvironment::Init");
    // STEP 1: Load HostFxr and get exported hosting functions
    //
    if (!load_hostfxr())
    {
        return E_FAIL;
    }

    // STEP 2: Initialize and start the .NET Core runtime
    //
    const string_t config_path = m_root_path + PATH_SEPARATOR + STR("Microsoft.SqlServer.CSharpExtension.runtimeconfig.json");
    hostfxr_handle cxt = get_dotnet(config_path.c_str());
    if (cxt == nullptr)
    {
        return E_FAIL;
    }

    // get_dotnet_load_assembly tries hdt_get_function_pointer first (Default ALC),
    // falls back to hdt_load_assembly_and_get_function_pointer (IsolatedComponentLoadContext).
    // On success with hdt_get_function_pointer, it returns the fn ptr and m_load_assembly_and_get_function_pointer stays null.
    // On fallback, it returns nullptr and sets m_load_assembly_and_get_function_pointer.
    m_get_function_pointer = get_dotnet_load_assembly(cxt);

    if (m_get_function_pointer == nullptr && m_load_assembly_and_get_function_pointer == nullptr)
    {
        return E_FAIL;
    }

    return S_OK;
}

//--------------------------------------------------------------------------------------------------
// Name: DotnetEnvironment::convert_string
//
// Description:
// Convert a std::string to the platform string_t type.
// On Windows this performs UTF-8 to UTF-16 conversion; on Linux it is a no-op.
//
string_t DotnetEnvironment::convert_string(const std::string& str)
{
#ifdef _WIN32
    return to_utf16_str(str);
#else
    return str;
#endif
}

#ifdef _WIN32
//--------------------------------------------------------------------------------------------------
// Name: DotnetEnvironment::to_utf16_str
//
// Description:
// Convert a utf8 string to utf16.
//
string_t DotnetEnvironment::to_utf16_str(const std::string& utf8str)
{
    LOG("DotnetEnvironment::to_utf16_str");
    int wchars_num = MultiByteToWideChar( CP_UTF8 , 0 , utf8str.c_str() , -1, nullptr, 0);
    unique_ptr<wchar_t> wstr (new wchar_t[wchars_num]);
    MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), -1, wstr.get(), wchars_num);
    return string_t(wstr.get());
}
#endif

//--------------------------------------------------------------------------------------------------
// Name: DotnetEnvironment::to_hex_string
//
// Description:
// Convert an int to string in hex.
//
string DotnetEnvironment::to_hex_string(int value)
{
    LOG("DotnetEnvironment::to_hex_string");
    std::stringstream s;
    s << "0x" << std::hex << std::showbase << value;
    return s.str();
}

//--------------------------------------------------------------------------------------------------
// Name: DotnetEnvironment::load_library
//
// Description:
// Load the library from the path
//
void* DotnetEnvironment::load_library(const char_t *path)
{
    LOG("DotnetEnvironment::load_library");
#ifdef _WIN32
    HMODULE h = ::LoadLibraryW(path);
#else
    void *h = dlopen(path, RTLD_LAZY);
#endif
    assert(h != nullptr);
    return (void*)h;
}

//--------------------------------------------------------------------------------------------------
// Name: DotnetEnvironment::get_export
//
// Description:
// Export the method from the library
//
void* DotnetEnvironment::get_export(void *h, const char *name)
{
    LOG("DotnetEnvironment::get_export");
#ifdef _WIN32
    void *f = ::GetProcAddress((HMODULE)h, name);
#else
    void *f = dlsym(h, name);
#endif
    assert(f != nullptr);
    return f;
}

//--------------------------------------------------------------------------------------------------
// Name: DotnetEnvironment::load_hostfxr
//
// Description:
// Load hostfxr and get desired exports
//
bool DotnetEnvironment::load_hostfxr()
{
    LOG("DotnetEnvironment::load_hostfxr");
#ifdef _WIN32
    string_t hostfxr_location = m_root_path + STR("\\hostfxr.dll");
#else
    // For self-contained deployment, load the bundled libhostfxr.so directly
    // from the extension directory. This is analogous to what Windows does
    // with hostfxr.dll and avoids picking up an incompatible older hostfxr
    // (e.g. .NET Core 3.x shipped with SQL Server).
    string_t hostfxr_location = m_root_path + STR("/libhostfxr.so");

    // If the bundled hostfxr doesn't exist (framework-dependent deployment),
    // fall back to nethost discovery.
    if (access(hostfxr_location.c_str(), F_OK) != 0)
    {
        m_is_self_contained = false;
        char buffer[HOSTFXR_PATH_BUFFER_SIZE];
        size_t buffer_size = sizeof(buffer);
        if (get_hostfxr_path(buffer, &buffer_size, nullptr) != 0)
        {
            LOG_ERROR("Failed to locate hostfxr via nethost");
            return false;
        }
        hostfxr_location = string_t(buffer);
    }
    else
    {
        m_is_self_contained = true;
    }
#endif
    void *lib = load_library(hostfxr_location.c_str());
    m_init_fptr = (hostfxr_initialize_for_runtime_config_fn)get_export(lib, "hostfxr_initialize_for_runtime_config");
    m_get_delegate_fptr = (hostfxr_get_runtime_delegate_fn)get_export(lib, "hostfxr_get_runtime_delegate");
    m_close_fptr = (hostfxr_close_fn)get_export(lib, "hostfxr_close");

#ifndef _WIN32
    // For self-contained deployments, hostfxr_initialize_for_runtime_config
    // rejects runtimeconfig.json files with "includedFrameworks" (error 0x80008093).
    // Instead, use hostfxr_initialize_for_dotnet_command_line which supports
    // both framework-dependent and self-contained applications (.NET 5+).
    if (m_is_self_contained)
    {
        m_init_cmdline_fptr = (hostfxr_initialize_for_dotnet_command_line_fn)get_export(lib, "hostfxr_initialize_for_dotnet_command_line");
        return (m_init_cmdline_fptr && m_get_delegate_fptr && m_close_fptr);
    }
#endif

    return (m_init_fptr && m_get_delegate_fptr && m_close_fptr);
}

//--------------------------------------------------------------------------------------------------
// Name: DotnetEnvironment::get_dotnet_load_assembly
//
// Description:
// Get the function pointer delegate for the loaded .NET Core.
// Uses hdt_get_function_pointer to load into Default ALC, ensuring type identity
// compatibility with user DLLs loaded via Assembly.LoadFrom.
// Falls back to hdt_load_assembly_and_get_function_pointer for Windows/non-self-contained.
//
get_function_pointer_fn DotnetEnvironment::get_dotnet_load_assembly(hostfxr_handle cxt)
{
    LOG("DotnetEnvironment::get_dotnet_load_assembly");

    // Try hdt_get_function_pointer first (loads into Default ALC).
    // This is required for self-contained Linux deployments to ensure
    // the managed extension and user DLLs share the same assembly context.
    void *get_fn_ptr = nullptr;
    int rc = m_get_delegate_fptr(
        cxt,
        hdt_get_function_pointer,
        &get_fn_ptr);
    if (rc == 0 && get_fn_ptr != nullptr)
    {
        m_close_fptr(cxt);
        return (get_function_pointer_fn)get_fn_ptr;
    }

    // Fallback to hdt_load_assembly_and_get_function_pointer (IsolatedComponentLoadContext).
    // This path is used on Windows and non-self-contained deployments.
    void *load_assembly_and_get_function_pointer = nullptr;
    rc = m_get_delegate_fptr(
        cxt,
        hdt_load_assembly_and_get_function_pointer,
        &load_assembly_and_get_function_pointer);
    if (rc != 0 || load_assembly_and_get_function_pointer == nullptr)
    {
        LOG_ERROR("Get delegate failed: " + to_hex_string(rc));
    }

    // Store the load_assembly_and_get_function_pointer for the fallback path
    m_load_assembly_and_get_function_pointer = (load_assembly_and_get_function_pointer_fn)load_assembly_and_get_function_pointer;

    m_close_fptr(cxt);
    // Return nullptr to signal that the caller should use m_load_assembly_and_get_function_pointer
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
// Name: DotnetEnvironment::get_dotnet
//
// Description:
// Load the .NET runtime from the configuration path and DOTNET_ROOT environment variable, and get the hostfxr handle.
//
hostfxr_handle DotnetEnvironment::get_dotnet(const char_t *config_path){
    LOG("DotnetEnvironment::get_dotnet");
    hostfxr_handle cxt = nullptr;

    hostfxr_initialize_parameters params = { 0 };
    params.size = sizeof(hostfxr_initialize_parameters);
    params.host_path = nullptr;
    params.dotnet_root = nullptr;

#ifdef _WIN32
    // Get the required size for the environment variable
    DWORD requiredSize = GetEnvironmentVariableW(L"DOTNET_ROOT", nullptr, 0);
    std::vector<wchar_t> dotnet_root_buffer;
    if (requiredSize > 0)
    {
        dotnet_root_buffer.resize(requiredSize);
        if (GetEnvironmentVariableW(L"DOTNET_ROOT", dotnet_root_buffer.data(), requiredSize) > 0)
        {
            params.dotnet_root = dotnet_root_buffer.data();
        }
    }
#else
    // On Linux, use DOTNET_ROOT if set; otherwise default to the extension
    // root path so hostfxr can find the self-contained shared framework.
    const char *dotnet_root_env = getenv("DOTNET_ROOT");
    if (dotnet_root_env != nullptr)
    {
        params.dotnet_root = dotnet_root_env;
    }
    else
    {
        params.dotnet_root = m_root_path.c_str();
    }
#endif

    int rc = 0;

#ifndef _WIN32
    if (m_is_self_contained && m_init_cmdline_fptr)
    {
        // For self-contained deployment, use hostfxr_initialize_for_dotnet_command_line.
        // hostfxr_initialize_for_runtime_config rejects self-contained runtimeconfig.json
        // files that contain "includedFrameworks" with error 0x80008093.
        // hostfxr_initialize_for_dotnet_command_line supports both framework-dependent
        // and self-contained applications (.NET 5+).
        const string_t app_path = m_root_path + STR("/Microsoft.SqlServer.CSharpExtension.dll");
        const char_t *argv[] = { app_path.c_str() };
        rc = m_init_cmdline_fptr(1, argv, &params, &cxt);
    }
    else
#endif
    {
        rc = m_init_fptr(config_path, &params, &cxt);
    }

    if (rc != 0 || cxt == nullptr)
    {
        LOG_ERROR("Init failed: " + to_hex_string(rc));
        if (cxt) m_close_fptr(cxt);
        return nullptr;
    }
    return cxt;
}

