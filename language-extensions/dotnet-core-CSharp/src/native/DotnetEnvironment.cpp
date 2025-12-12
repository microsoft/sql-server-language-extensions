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
#include "Windows.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <assert.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>

#define STR(s) L ## s
#define CH(c) L ## c

using namespace std;
using string_t = std::basic_string<char_t>;

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
    std::string private_library_path) : m_root_path(to_utf16_str(language_path))
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
    const string_t config_path = m_root_path + STR("\\Microsoft.SqlServer.CSharpExtension.runtimeconfig.json");
    hostfxr_handle cxt = get_dotnet(config_path.c_str());
    if (cxt == nullptr)
    {
        return E_FAIL;
    }

    m_load_assembly_and_get_function_pointer = get_dotnet_load_assembly(cxt);

    if (m_load_assembly_and_get_function_pointer == nullptr)
    {
        return E_FAIL;
    }

    return S_OK;
}

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
    HMODULE h = ::LoadLibraryW(path);
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
    void *f = ::GetProcAddress((HMODULE)h, name);
    assert(f != nullptr);
    return f;
}

//--------------------------------------------------------------------------------------------------
// Name: DotnetEnvironment::get_export
//
// Description:
// Load hostfxr and get desired exports
//
bool DotnetEnvironment::load_hostfxr()
{
    LOG("DotnetEnvironment::load_hostfxr");
    string_t hostfxr_location = m_root_path + STR("\\hostfxr.dll");
    void *lib = load_library(hostfxr_location.c_str());
    m_init_fptr = (hostfxr_initialize_for_runtime_config_fn)get_export(lib, "hostfxr_initialize_for_runtime_config");
    m_get_delegate_fptr = (hostfxr_get_runtime_delegate_fn)get_export(lib, "hostfxr_get_runtime_delegate");
    m_close_fptr = (hostfxr_close_fn)get_export(lib, "hostfxr_close");

    return (m_init_fptr && m_get_delegate_fptr && m_close_fptr);
}

//--------------------------------------------------------------------------------------------------
// Name: DotnetEnvironment::get_dotnet_load_assembly
//
// Description:
// load assembly function pointer from the path.
//
load_assembly_and_get_function_pointer_fn DotnetEnvironment::get_dotnet_load_assembly(hostfxr_handle cxt)
{
    LOG("DotnetEnvironment::get_dotnet_load_assembly");
    // Load .NET Core
    void *load_assembly_and_get_function_pointer = nullptr;

    // Get the load assembly function pointer
    int rc = m_get_delegate_fptr(
        cxt,
        hdt_load_assembly_and_get_function_pointer,
        &load_assembly_and_get_function_pointer);
    if (rc != 0 || load_assembly_and_get_function_pointer == nullptr)
    {
        LOG_ERROR("Get delegate failed: " + to_hex_string(rc));
    }

    m_close_fptr(cxt);
    return (load_assembly_and_get_function_pointer_fn)load_assembly_and_get_function_pointer;
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

    int rc = m_init_fptr(config_path, &params, &cxt);
    if (rc != 0 || cxt == nullptr)
    {
        LOG_ERROR("Init failed: " + to_hex_string(rc));
        if (cxt) m_close_fptr(cxt);
        return nullptr;
    }
    return cxt;
}

