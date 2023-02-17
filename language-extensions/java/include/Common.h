//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: Common.h
//
// Purpose:
//  Common header containing only the most common includes shared
//  across majority of the files
//
//*********************************************************************
#pragma once

#ifdef _WIN64
#include <windows.h>
#endif
#include <exception>
#include <jni.h>
#include <memory>
#include <new>
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
