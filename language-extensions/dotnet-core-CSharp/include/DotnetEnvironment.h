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

#ifdef _WIN32
#include "Windows.h"
#else
#define E_FAIL -1
#define S_OK 0
#endif

#include <string>
#include <coreclr_delegates.h>
#include <hostfxr.h>

#ifdef _WIN32
#define STR(s) L ## s
#define CH(c) L ## c
#define PATH_SEPARATOR CH('\\')
#else
#define STR(s) s
#define CH(c) c
#define PATH_SEPARATOR CH('/')
#endif

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
        const string_t ManagedExtensionPath = m_root_path + PATH_SEPARATOR + ManagedExtensionName + STR(".dll");
        const string_t ManagedExtensionType = ManagedExtensionName + STR(".CSharpExtension, ") + ManagedExtensionName;
        const string_t ManagedExtensionMethod = convert_string(method_name);
        const string_t DelegateTypeName = ManagedExtensionName + STR(".CSharpExtension+") + ManagedExtensionMethod + STR("Delegate, ") + ManagedExtensionName;

        int rc = -1;
        if (m_get_function_pointer != nullptr)
        {
            // Preferred path: hdt_get_function_pointer loads into Default ALC.
            // This ensures the managed extension and user DLLs loaded via Assembly.LoadFrom
            // share the same assembly identity, allowing type casts to succeed.
            rc = m_get_function_pointer(
                ManagedExtensionType.c_str(),
                ManagedExtensionMethod.c_str(),
                DelegateTypeName.c_str(),
                nullptr, /* load_context: nullptr = Default ALC */
                nullptr, /* reserved */
                (void**)&managed_func);

            if (rc != 0 || managed_func == nullptr)
            {
                LOG_ERROR("get_function_pointer failed for " + method_name + ": " + to_hex_string(rc) + " (will try fallback)");
            }
        }

        if ((rc != 0 || managed_func == nullptr) && m_load_assembly_and_get_function_pointer != nullptr)
        {
            // Fallback: hdt_load_assembly_and_get_function_pointer (IsolatedComponentLoadContext).
            // Used on Windows and non-self-contained deployments, or when Default ALC
            // cannot resolve the assembly by name (e.g. component host scenario).
            managed_func = nullptr;
            rc = m_load_assembly_and_get_function_pointer(
                ManagedExtensionPath.c_str(),
                ManagedExtensionType.c_str(),
                ManagedExtensionMethod.c_str(),
                DelegateTypeName.c_str(),
                nullptr,
                (void**)&managed_func);

            if (rc != 0 || managed_func == nullptr)
            {
                LOG_ERROR("load_assembly_and_get_function_pointer also failed for " + method_name + ": " + to_hex_string(rc));
            }
        }

        if (rc != 0 || managed_func == nullptr)
        {
            LOG_ERROR("All managed method resolution failed for " + method_name + ", rc=" + to_hex_string(rc));
            return E_FAIL;
        }

        return managed_func(args...);
    }

private:
    hostfxr_initialize_for_runtime_config_fn m_init_fptr;
    hostfxr_get_runtime_delegate_fn m_get_delegate_fptr;
    hostfxr_close_fn m_close_fptr;
    load_assembly_and_get_function_pointer_fn m_load_assembly_and_get_function_pointer;
    get_function_pointer_fn m_get_function_pointer;
    string_t m_root_path;
    bool m_is_self_contained;

    // Convert a std::string to the platform string_t type.
    // On Windows this is UTF-8 -> UTF-16; on Linux it is a no-op.
    //
    static string_t convert_string(const std::string& str);

#ifdef _WIN32
    // Convert utf8_str to utf16_str (Windows only)
    //
    static string_t to_utf16_str(const std::string& utf8str);
#endif

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

    // Get the function pointer delegate for the loaded .NET Core.
    // Uses hdt_get_function_pointer to load into Default ALC.
    //
    get_function_pointer_fn get_dotnet_load_assembly(hostfxr_handle cxt);

    // Load and initialize .NET Core
    //
    hostfxr_handle get_dotnet(const char_t *config_path);
};
