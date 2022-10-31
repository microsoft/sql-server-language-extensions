//**************************************************************************************************
// RExtension-test : Executable testing language extension that implements the SQL Server
// external language communication protocol.
// Copyright (C) 2020 Microsoft Corporation.

// This file is part of RExtension-test.

// RExtension-test is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// RExtension-test is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with RExtension-test.  If not, see <https://www.gnu.org/licenses/>.

// @File: Common.h
//
// Purpose:
//  Common headers for RExtenion-test.
//
//**************************************************************************************************

#pragma once

#ifdef _WIN64
	#include <windows.h>
	#include <filesystem>
#else
	#include <dlfcn.h>
	#include <experimental/filesystem>
#endif

#include <iostream>
#include <math.h>
#include <memory>
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

#include "gtest/gtest.h"

// Before including Rcpp headers, undefine ERROR if it is defined already since
// R_ext/RS.h defines ERROR and we want to avoid redefinition warnings.
//
#ifdef ERROR
	#undef ERROR
#endif
#include "Rcpp.h"
#include "RInside.h"

#define TOTAL_NUMBER_OF_TEST_SUITES 2

#include "RExtensionApiTests.h"
#include "Unicode.h"
#include "Utilities.h"
