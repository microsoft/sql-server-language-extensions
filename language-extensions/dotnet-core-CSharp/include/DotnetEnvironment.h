//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: DotnetEnvironment.h
//
// Purpose:
//  Implement the .NET environment
//
//*********************************************************************
#pragma once

#include "Windows.h"
#include <string>
#include <coreclr_delegates.h>
#include <hostfxr.h>

#define STR(s) L ## s
#define CH(c) L ## c

using namespace std;
using string_t = std::basic_string<char_t>;

class DotnetEnvironment
{
public:
    // Constructor to initialize the members
    //
    DotnetEnvironment(
        std::string language_params,
        std::string language_path,
        std::string public_library_path,
        std::string private_library_path);

    // Init the runtime
    //
    short Init();

    // Call managed method with method name and arguments, method needs to have delegate in format of MethodNameDelegate
    //
    template <typename T, typename... Args>
    short call_managed_method(const std::string& method_name, Args... args) noexcept
    {
        T managed_func = nullptr;

        // Load managed assembly and get function pointer to a managed method
        //
        const string_t ManagedExtensionName = STR("Microsoft.SqlServer.CSharpExtension");
        const string_t ManagedExtensionPath = m_root_path + STR("\\") + ManagedExtensionName + STR(".dll");
        const string_t ManagedExtensionType = ManagedExtensionName + STR(".CSharpExtension, ") + ManagedExtensionName;
        const string_t ManagedExtensionMethod = to_utf16_str(method_name);
        const string_t DelegateTypeName = ManagedExtensionName + STR(".CSharpExtension+") + to_utf16_str(method_name) + STR("Delegate, ") + ManagedExtensionName;
        int rc = m_load_assembly_and_get_function_pointer(
            ManagedExtensionPath.c_str(),
            ManagedExtensionType.c_str(),
            ManagedExtensionMethod.c_str(),
            DelegateTypeName.c_str(),
            nullptr,
            (void**)&managed_func);
        if (rc != 0 || managed_func == nullptr)
        {
            return E_FAIL;
        }

        return managed_func(args...);
    }

private:
    hostfxr_initialize_for_runtime_config_fn m_init_fptr;
    hostfxr_get_runtime_delegate_fn m_get_delegate_fptr;
    hostfxr_close_fn m_close_fptr;
    load_assembly_and_get_function_pointer_fn m_load_assembly_and_get_function_pointer;
    string_t m_root_path;

    // Convert utf8_str to utf16_str
    //
    static string_t to_utf16_str(const std::string& utf8str);

    // Convert an int to string in hex.
    //
    static std::string to_hex_string(int value);

    // Load the library from the path
    //
    void* load_library(const char_t *path);

    // Load hostfxr and get desired exports
    //
    void* get_export(void *h, const char *name);

    // Load hostfxr and get desired exports
    //
    bool load_hostfxr();

    // Get desired function pointer for scenario for the loaded .NET Core
    //
    load_assembly_and_get_function_pointer_fn get_dotnet_load_assembly(hostfxr_handle cxt);

    // Load and initialize .NET Core
    //
    hostfxr_handle get_dotnet(const char_t *config_path);
};
