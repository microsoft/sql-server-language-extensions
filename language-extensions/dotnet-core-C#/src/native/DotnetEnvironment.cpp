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
#include "Windows.h"
#include <iostream>
#include <string>
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
//	Constructor to initialize the members. The language path is defined as the root path.
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
//	Initialize the runtime by reading the runtimeconfig.json file. 
//
short DotnetEnvironment::Init()
{
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
//	Convert a utf8 string to utf16.
//
string_t DotnetEnvironment::to_utf16_str(const std::string& utf8str)
{
    int wchars_num = MultiByteToWideChar( CP_UTF8 , 0 , utf8str.c_str() , -1, nullptr, 0);
    unique_ptr<wchar_t> wstr (new wchar_t[wchars_num]);
    MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), -1, wstr.get(), wchars_num);
    return string_t(wstr.get());
}

//--------------------------------------------------------------------------------------------------
// Name: DotnetEnvironment::load_library
//
// Description:
//	Load the library from the path 
//
void* DotnetEnvironment::load_library(const char_t *path)
{
    HMODULE h = ::LoadLibraryW(path);
    assert(h != nullptr);
    return (void*)h;
}

//--------------------------------------------------------------------------------------------------
// Name: DotnetEnvironment::get_export
//
// Description:
//	Export the method from the library
//
void* DotnetEnvironment::get_export(void *h, const char *name)
{
    void *f = ::GetProcAddress((HMODULE)h, name);
    assert(f != nullptr);
    return f;
}

//--------------------------------------------------------------------------------------------------
// Name: DotnetEnvironment::get_export
//
// Description:
//	Load hostfxr and get desired exports
//
bool DotnetEnvironment::load_hostfxr()
{
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
//	load assembly function pointer from the path.
//
load_assembly_and_get_function_pointer_fn DotnetEnvironment::get_dotnet_load_assembly(hostfxr_handle cxt)
{
    // Load .NET Core
    void *load_assembly_and_get_function_pointer = nullptr;

    // Get the load assembly function pointer
    int rc = m_get_delegate_fptr(
        cxt,
        hdt_load_assembly_and_get_function_pointer,
        &load_assembly_and_get_function_pointer);
    if (rc != 0 || load_assembly_and_get_function_pointer == nullptr)
        std::cerr << "Get delegate failed: " << std::hex << std::showbase << rc << std::endl;

    m_close_fptr(cxt);
    return (load_assembly_and_get_function_pointer_fn)load_assembly_and_get_function_pointer;
}

//--------------------------------------------------------------------------------------------------
// Name: DotnetEnvironment::get_dotnet
//
// Description:
//	load the .NET from the path and get the hostfxr handle
//
hostfxr_handle DotnetEnvironment::get_dotnet(const char_t *config_path){
    hostfxr_handle cxt = nullptr;
    int rc = m_init_fptr(config_path, nullptr, &cxt);
    if (rc != 0 || cxt == nullptr)
    {
        std::cerr << "Init failed: " << std::hex << std::showbase << rc << std::endl;
        m_close_fptr(cxt);
        return nullptr;
    }
    return cxt;
}

