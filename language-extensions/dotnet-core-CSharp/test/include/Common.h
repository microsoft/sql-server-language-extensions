//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: Common.h
//
// Purpose:
//  Common headers for .NET Core CSharpExtenion-test.
//
//*********************************************************************
#pragma once

#ifdef _WIN64
#include <windows.h>
#else
#include <dlfcn.h>
#include <limits.h>
#include <unistd.h>
typedef void* HINSTANCE;
#endif

#include <iostream>
#include <cstring>
#ifdef _WIN32
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>
#else
#include <filesystem>
#endif
#include <math.h>
#include <memory>
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include "gtest/gtest.h"
#include "nativecsharpextension.h"
#include "CSharpExtensionApiTests.h"