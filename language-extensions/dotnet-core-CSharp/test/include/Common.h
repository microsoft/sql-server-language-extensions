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
#endif

#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <iostream>
#include <experimental/filesystem>
#include <math.h>
#include <memory>
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include "gtest/gtest.h"
#include "nativecsharpextension.h"
#include "CSharpExtensionApiTests.h"