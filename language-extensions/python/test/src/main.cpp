//*************************************************************************************************
// Copyright (C) Microsoft Corporation.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)
//
// @File: main.cpp
//
// Purpose:
//  Initialize gtest and run all Python extension tests
//
//*************************************************************************************************
#include <gtest/gtest.h>

int main(int argc, const char **argv)
{
	// banner
	//
	std::cout << "Running PythonExtension C++ unit tests.\n";

	// First, initiate Google Test framework - this will remove
	// framework-specific parameters from argc and argv
	//
	::testing::InitGoogleTest(&argc, const_cast<char**>(argv));
	int rc = RUN_ALL_TESTS();

	return rc;
}
