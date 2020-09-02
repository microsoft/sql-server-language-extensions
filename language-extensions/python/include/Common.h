//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: Common.h
//
// Purpose:
//  Common headers for extension
//
//*************************************************************************************************

#pragma once

#ifdef _WIN64
#include <windows.h>
#endif

#include <string>
#include <iostream>
#include <exception>
#include <stdio.h>
#include <vector>
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
