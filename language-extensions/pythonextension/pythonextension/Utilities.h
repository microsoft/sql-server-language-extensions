#pragma once

#define GuidFormat "%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X"

class Utilities
{
public:
    static void* GetFunctionFromModule(const void * pHDLL, const std::string fnName);

    static void* GetFunctionFromDLL(const std::string dllPath, const std::string fnName);
    static void* GetDLL(const std::string dllPath);

    static std::string ConvertGuidToString(const SQLGUID * guid);

private:

};

