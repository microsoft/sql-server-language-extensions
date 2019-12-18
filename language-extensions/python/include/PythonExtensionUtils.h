//*********************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: PythonExtensionUtils.h
//
// Purpose:
//  Platform specific utility functions for Python Extension
//
//*********************************************************************

#pragma once
#include "Common.h"

class PythonExtensionUtils
{
public:
	static std::string GetEnvVariable(const std::string & envVarName);
	
	static std::string ConvertGuidToString(const SQLGUID *guid);

	static void FreeDLL(void *pDll);

private:

};