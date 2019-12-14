//******************************************************************************************************
// RExtension-test : Executable testing language extension that implements the SQL Server external language communication protocol.
// Copyright (C) 2019 Microsoft Corporation.

// This file is part of RExtension.

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

// @File: Utilities.cpp
//
// Purpose:
//  Utility functions
//
//******************************************************************************************************
#pragma once

class Utilities
{
public:
	static void* CrossPlatGetFunctionFromLibPath(const std::string &libPath, const std::string &fnName);

private:
	static void* CrossPlatGetFunctionFromLibHandle(void *libHandle, const std::string &fnName);
	static void* CrossPlatLoadLibrary(const char *libPath);
};