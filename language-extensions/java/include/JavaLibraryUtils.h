//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: JavaLibraryUtils.h
//
// Purpose:
//  Library utility classes used by the Java extension.
//
//*********************************************************************
#pragma once

#include "Common.h"

//---------------------------------------------------------------------
// Description:
//	Library utility functions for the Java extension.
//
class JavaLibraryUtils
{
public:
	static std::string GetLibrariesClassPath();

private:
	static void FindAppendFileNames(const std::string &basePath, std::string &output);
};
