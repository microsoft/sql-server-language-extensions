//*************************************************************************************************
// RExtension-test : Executable testing language extension that implements the SQL Server
// external language communication protocol.
// Copyright (C) 2020 Microsoft Corporation.
//
// This file is part of RExtension-test.
//
// RExtension-test is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// RExtension-test is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with RExtension-test.  If not, see <https://www.gnu.org/licenses/>.
//
// @File: main.cpp
//
// Purpose:
//  Tests the RExtension's implementation of the external language APIs.
//
//*************************************************************************************************

#include "Common.h"

int g_argc = 0;
const char **g_argv = nullptr;

int main(int argc, const char **argv)
{
	// Banner
	//
	std::cout << "Running RExtension C++ unit tests.\n";

	// First, initiate Google Test framework - this will remove
	// framework-specific parameters from argc and argv
	//
	::testing::InitGoogleTest(&argc, const_cast<char**>(argv));
	g_argc = argc;
	g_argv = argv;

	int rc = RUN_ALL_TESTS();

	return rc;
}
