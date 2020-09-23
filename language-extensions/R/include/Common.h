//**************************************************************************************************
// RExtension : A language extension implementing the SQL Server
// external language communication protocol for R.
// Copyright (C) 2020 Microsoft Corporation.
//
// This file is part of RExtension.
//
// RExtension is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// RExtension is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with RExtension.  If not, see <https://www.gnu.org/licenses/>.
//
// @File: Common.h
//
// Purpose:
//  Common headers for RExtension.
//
//**************************************************************************************************

#pragma once

#ifdef _WIN64
#include <windows.h>
#endif

#include <exception>
#include <memory>
#include <sqlext.h>
#include <sqltypes.h>
#include <stdlib.h>
#include <string>
#include <vector>

// Before including Rcpp headers, undefine ERROR if it is defined already since
// R_ext/RS.h defines ERROR and we want to avoid redefinition warnings.
//
#ifdef ERROR
	#undef ERROR
#endif
#include "Rcpp.h"
#include "RInside.h" // for the embedded R via RInside

#include "Logger.h"
#include "REnvironment.h"
#include "RPathSettings.h"
#include "Utilities.h"
