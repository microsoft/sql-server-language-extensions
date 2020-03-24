//*************************************************************************************************
// RExtension : A language extension implementing the SQL Server
// external language communication protocol for R.
// Copyright (C) 2019 Microsoft Corporation.
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
// @File: Utilities.cpp
//
// Purpose:
//  Utility functions implemented for both the platforms.
//
//*************************************************************************************************

#include <memory>
#include <string.h>
#include <vector>
#include "Common.h"
#include "Utilities.h"
#include "Logger.h"

using namespace std;

// Given a constant string input, generate a unique pointer
// pointing to a char array containing the same contents as that of the input
//
unique_ptr<char> Utilities::GenerateUniquePtr(const string &inputStr)
{
	int inputStrLen = inputStr.length();

	// Reserve additional space for null terminating character.
	//
	unique_ptr<char> outPtr(new char[inputStrLen + 1]);
	memcpy(outPtr.get(), inputStr.c_str(), inputStrLen);
	*(outPtr.get() + inputStrLen) = '\0';
	return outPtr;
}
