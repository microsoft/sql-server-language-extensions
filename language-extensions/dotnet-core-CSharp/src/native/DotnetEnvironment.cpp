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
#include <fstream>
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
    // Diagnostic logging: verify dotnet_root and shared/ directory presence
    // before calling hostfxr_initialize_for_runtime_config.
    if (params.dotnet_root != nullptr)
    {
        LOG("dotnet_root: " + std::string(params.dotnet_root));

        // Check shared/ directory
        std::string shared_dir = std::string(params.dotnet_root) + "/shared";
        struct stat st;
        if (stat(shared_dir.c_str(), &st) == 0)
        {
            LOG("shared/ exists (mode=" + std::to_string(st.st_mode) + " uid=" + std::to_string(st.st_uid) + " gid=" + std::to_string(st.st_gid) + ")");

            std::string app_dir = shared_dir + "/Microsoft.NETCore.App";
            if (stat(app_dir.c_str(), &st) == 0)
            {
                LOG("Microsoft.NETCore.App/ exists (mode=" + std::to_string(st.st_mode) + ")");

                // List version subdirectories and their contents
                DIR *d = opendir(app_dir.c_str());
                if (d)
                {
                    struct dirent *de;
                    while ((de = readdir(d)) != nullptr)
                    {
                        if (de->d_name[0] != '.')
                        {
                            LOG("  framework version: " + std::string(de->d_name));

                            // List files inside this version directory
                            std::string ver_dir = app_dir + "/" + std::string(de->d_name);
                            DIR *vd = opendir(ver_dir.c_str());
                            if (vd)
                            {
                                struct dirent *ve;
                                int fcount = 0;
                                bool has_deps = false, has_coreclr = false, has_dotversion = false;
                                while ((ve = readdir(vd)) != nullptr)
                                {
                                    if (ve->d_name[0] != '.')
                                        fcount++;
                                    if (std::string(ve->d_name) == "Microsoft.NETCore.App.deps.json")
                                        has_deps = true;
                                    if (std::string(ve->d_name) == "libcoreclr.so")
                                        has_coreclr = true;
                                    if (std::string(ve->d_name) == ".version")
                                        has_dotversion = true;
                                }
                                closedir(vd);
                                LOG("    file count: " + std::to_string(fcount) +
                                    " deps.json=" + std::to_string(has_deps) +
                                    " libcoreclr.so=" + std::to_string(has_coreclr) +
                                    " .version=" + std::to_string(has_dotversion));
                            }
                        }
                    }
                    closedir(d);
                }
            }
            else
            {
                LOG_ERROR("Microsoft.NETCore.App/ NOT found (errno=" + std::to_string(errno) + ")");
            }
        }
        else
        {
            LOG_ERROR("shared/ NOT found at " + shared_dir + " (errno=" + std::to_string(errno) + ")");

            // List root directory contents for debugging
            DIR *d = opendir(params.dotnet_root);
            if (d)
            {
                struct dirent *de;
                int count = 0;
                while ((de = readdir(d)) != nullptr)
                {
                    if (de->d_name[0] != '.')
                    {
                        if (count < 30)
                            LOG("  root entry: " + std::string(de->d_name) + " (type=" + std::to_string(de->d_type) + ")");
                        count++;
                    }
                }
                LOG("  total entries: " + std::to_string(count));
                closedir(d);
            }
            else
            {
                LOG_ERROR("Cannot opendir dotnet_root (errno=" + std::to_string(errno) + ")");
            }
        }
    }
    else
    {
        LOG("dotnet_root is nullptr");
    }

    // Log runtimeconfig path and check it exists
    {
        std::string cfg(config_path);
        LOG("config_path: " + cfg);
        struct stat cfgst;
        if (stat(cfg.c_str(), &cfgst) != 0)
            LOG_ERROR("config_path NOT found (errno=" + std::to_string(errno) + ")");

        // Read and log first 500 chars of runtimeconfig for verification
        std::ifstream rcf(cfg);
        if (rcf.is_open())
        {
            std::string content((std::istreambuf_iterator<char>(rcf)),
                                 std::istreambuf_iterator<char>());
            if (content.size() > 500) content.resize(500);
            LOG("runtimeconfig content: " + content);
        }
    }

    // Log DOTNET_ROOT and DOTNET_MULTILEVEL_LOOKUP env vars
    {
        const char *dr = getenv("DOTNET_ROOT");
        LOG("env DOTNET_ROOT=" + std::string(dr ? dr : "(null)"));
        const char *ml = getenv("DOTNET_MULTILEVEL_LOOKUP");
        LOG("env DOTNET_MULTILEVEL_LOOKUP=" + std::string(ml ? ml : "(null)"));
    }

    // Enable COREHOST_TRACE for detailed hostfxr diagnostics
    setenv("COREHOST_TRACE", "1", 1);
    setenv("COREHOST_TRACEFILE", "/tmp/hostfxr_trace.log", 1);
    setenv("COREHOST_TRACE_VERBOSITY", "4", 1);
#endif

    int rc = m_init_fptr(config_path, &params, &cxt);

#ifndef _WIN32
    // Read and log the COREHOST_TRACE output
    {
        std::ifstream trace("/tmp/hostfxr_trace.log");
        if (trace.is_open())
        {
            std::string line;
            int linecount = 0;
            while (std::getline(trace, line))
            {
                if (linecount < 200)
                    LOG("TRACE: " + line);
                linecount++;
            }
            if (linecount >= 200)
                LOG("TRACE: ... (truncated, total " + std::to_string(linecount) + " lines)");
            trace.close();
        }
        else
        {
            LOG_ERROR("Could not read /tmp/hostfxr_trace.log");
        }
        // Clean up trace env vars
        unsetenv("COREHOST_TRACE");
        unsetenv("COREHOST_TRACEFILE");
        unsetenv("COREHOST_TRACE_VERBOSITY");
    }
#endif

    if (rc != 0 || cxt == nullptr)
    {
        LOG_ERROR("Init failed: " + to_hex_string(rc));
        if (cxt) m_close_fptr(cxt);
        return nullptr;
    }
    return cxt;
}

