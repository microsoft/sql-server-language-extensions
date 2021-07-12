//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: main.cpp
//
// Purpose:
//  Main driver to invoke tests testing the dotnet-core-CSharp-extension's
//  implementation of the external language APIs.
//
//*********************************************************************
#include "gtest/gtest.h"
#include <iostream>

int g_argc = 0;
const char **g_argv = nullptr;

int main(int argc, const char **argv)
{
	std::cout << "Running dotnetcore-CSharp-extension C++ unit tests.\n";

	// First, initiate Google Test framework - this will remove
	// framework-specific parameters from argc and argv
	//
	::testing::InitGoogleTest(&argc, const_cast<char**>(argv));
	g_argc = argc;
	g_argv = argv;

	int rc = RUN_ALL_TESTS();

	return rc;
}
