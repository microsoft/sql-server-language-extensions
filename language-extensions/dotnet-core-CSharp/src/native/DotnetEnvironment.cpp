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

#ifndef _WIN32
#include <sys/stat.h>
#include <dirent.h>
#endif

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

    // get_dotnet_load_assembly always obtains both delegates when available.
    // m_get_function_pointer (Default ALC) is preferred for type identity,
    // m_load_assembly_and_get_function_pointer (explicit path) is the fallback
    // for when the Default ALC cannot resolve the assembly by name.
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
    s << "0x" << std::hex << value;
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

    return (m_init_fptr && m_get_delegate_fptr && m_close_fptr);
}

//--------------------------------------------------------------------------------------------------
// Name: DotnetEnvironment::get_dotnet_load_assembly
//
// Description:
// Get the function pointer delegates for the loaded .NET Core.
// Always obtains BOTH hdt_get_function_pointer (Default ALC) and
// hdt_load_assembly_and_get_function_pointer (explicit path) so call_managed_method
// can fall back to the explicit-path delegate when name-based resolution fails.
// This is critical for self-contained Linux deployments where the component host's
// Default ALC may not resolve application assemblies by name.
//
get_function_pointer_fn DotnetEnvironment::get_dotnet_load_assembly(hostfxr_handle cxt)
{
    LOG("DotnetEnvironment::get_dotnet_load_assembly");

    // Always get hdt_load_assembly_and_get_function_pointer as a fallback.
    // This loads assemblies by explicit file path into an IsolatedComponentLoadContext
    // and works reliably in all deployment scenarios.
    void *load_assembly_and_get_function_pointer = nullptr;
    int rc2 = m_get_delegate_fptr(
        cxt,
        hdt_load_assembly_and_get_function_pointer,
        &load_assembly_and_get_function_pointer);
    if (rc2 == 0 && load_assembly_and_get_function_pointer != nullptr)
    {
        m_load_assembly_and_get_function_pointer = (load_assembly_and_get_function_pointer_fn)load_assembly_and_get_function_pointer;
    }
    else
    {
        LOG_ERROR("Get load_assembly_and_get_function_pointer delegate failed: " + to_hex_string(rc2));
    }

    // Try hdt_get_function_pointer (loads into Default ALC).
    // This is preferred for self-contained Linux deployments to ensure
    // the managed extension and user DLLs share the same assembly context.
    void *get_fn_ptr = nullptr;
    int rc = m_get_delegate_fptr(
        cxt,
        hdt_get_function_pointer,
        &get_fn_ptr);

    m_close_fptr(cxt);

    if (rc == 0 && get_fn_ptr != nullptr)
    {
        return (get_function_pointer_fn)get_fn_ptr;
    }

    if (m_load_assembly_and_get_function_pointer == nullptr)
    {
        LOG_ERROR("Both get_function_pointer and load_assembly_and_get_function_pointer delegates failed");
    }

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
    // On Linux, for self-contained (bundled) deployments, set dotnet_root to
    // the extension directory so hostfxr finds the runtime via the shared/ directory.
    // The build script transforms the self-contained runtimeconfig.json to look
    // framework-dependent ("framework" instead of "includedFrameworks") and creates
    // a shared/Microsoft.NETCore.App/<version>/ directory with copies of the
    // root DLLs/SOs. File copies are used because SQL Server's tar extraction does
    // not preserve symbolic links or hard links when extracting CREATE EXTERNAL
    // LANGUAGE payloads.
    // For framework-dependent deployments, leave dotnet_root as nullptr
    // so hostfxr uses its default discovery mechanism.
    const char *dotnet_root_env = getenv("DOTNET_ROOT");
    if (dotnet_root_env != nullptr)
    {
        params.dotnet_root = dotnet_root_env;
    }
    else if (m_is_self_contained)
    {
        params.dotnet_root = m_root_path.c_str();
    }
#endif

#ifndef _WIN32
    // Concise diagnostic logging for runtime initialization.
    // Uses LOG_ERROR so it's visible in Release builds.
    if (params.dotnet_root != nullptr)
    {
        LOG_ERROR("CSharpExt: dotnet_root=" + std::string(params.dotnet_root) + " self_contained=" + std::to_string(m_is_self_contained));

        // Verify shared/ framework directory
        std::string shared_dir = std::string(params.dotnet_root) + "/shared";
        struct stat st;
        if (stat(shared_dir.c_str(), &st) != 0)
        {
            LOG_ERROR("CSharpExt: shared/ NOT found at " + shared_dir + " (errno=" + std::to_string(errno) + ")");
        }
        else
        {
            std::string app_dir = shared_dir + "/Microsoft.NETCore.App";
            if (stat(app_dir.c_str(), &st) != 0)
            {
                LOG_ERROR("CSharpExt: Microsoft.NETCore.App/ NOT found (errno=" + std::to_string(errno) + ")");
            }
            else
            {
                DIR *d = opendir(app_dir.c_str());
                if (d)
                {
                    struct dirent *de;
                    while ((de = readdir(d)) != nullptr)
                    {
                        if (de->d_name[0] != '.')
                        {
                            std::string ver_dir = app_dir + "/" + std::string(de->d_name);
                            DIR *vd = opendir(ver_dir.c_str());
                            int fcount = 0;
                            bool has_deps = false;
                            if (vd)
                            {
                                struct dirent *ve;
                                while ((ve = readdir(vd)) != nullptr)
                                {
                                    if (ve->d_name[0] != '.') fcount++;
                                    if (std::string(ve->d_name) == "Microsoft.NETCore.App.deps.json") has_deps = true;
                                }
                                closedir(vd);
                            }
                            LOG_ERROR("CSharpExt: framework " + std::string(de->d_name) + " files=" + std::to_string(fcount) + " deps.json=" + std::to_string(has_deps));
                        }
                    }
                    closedir(d);
                }
            }
        }

        // Verify runtimeconfig.json exists
        std::string cfg(config_path);
        struct stat cfgst;
        if (stat(cfg.c_str(), &cfgst) != 0)
            LOG_ERROR("CSharpExt: runtimeconfig NOT found: " + cfg);
        else
            LOG_ERROR("CSharpExt: runtimeconfig OK: " + cfg + " size=" + std::to_string(cfgst.st_size));

        // Verify managed assembly exists
        std::string managed_dll = std::string(params.dotnet_root) + "/Microsoft.SqlServer.CSharpExtension.dll";
        if (stat(managed_dll.c_str(), &cfgst) != 0)
            LOG_ERROR("CSharpExt: managed DLL NOT found: " + managed_dll);
        else
            LOG_ERROR("CSharpExt: managed DLL OK: " + managed_dll + " size=" + std::to_string(cfgst.st_size));
    }
    else
    {
        LOG_ERROR("CSharpExt: dotnet_root is nullptr");
    }
#endif

    int rc = m_init_fptr(config_path, &params, &cxt);

    if (rc != 0 || cxt == nullptr)
    {
        LOG_ERROR("CSharpExt: hostfxr_initialize_for_runtime_config failed: " + to_hex_string(rc));
        if (cxt) m_close_fptr(cxt);
        return nullptr;
    }

#ifndef _WIN32
    LOG_ERROR("CSharpExt: hostfxr init succeeded, rc=" + to_hex_string(rc));
#endif

    return cxt;
}

