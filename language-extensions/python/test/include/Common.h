//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: Common.h
//
// Purpose:
//  Common headers for the python test project
//
//*************************************************************************************************
#pragma once

#ifdef _WIN64
#include <windows.h>
#endif

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include <iostream>
#include <string>
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

#include "gtest/gtest.h"
#include "sqlexternallibrary.h"
