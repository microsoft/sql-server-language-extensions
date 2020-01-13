//******************************************************************************************************
// RExtension : A language extension implementing the SQL Server external language communication protocol.
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
// @File: Utilities_linux.cpp
//
// Purpose:
//  Utility functions implemented for the linux based OSes.
//
//******************************************************************************************************

#include <memory>
#include <string.h>
#include <vector>
#include "Common.h"
#include "Utilities.h"
#include "Logger.h"

using namespace std;

const char* GuidFormat="%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X";

//--------------------------------------------------------------------------------------------------
// Name: Utilities::ConvertGuidToString
//
// Description:
//  Converts a SQLGUID to a string
//
// Returns:
//	string of the guid
//
std::string Utilities::ConvertGuidToString(const SQLGUID *guid)
{
	// 32 hex chars + 4 hyphens + null terminator, so 37 characters.
	//
	char guidString[37];
	snprintf(guidString, sizeof(guidString) / sizeof(guidString[0]),
		GuidFormat,
		static_cast<unsigned long>(guid->Data1), guid->Data2, guid->Data3,
		guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
		guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
	std::string s(guidString);
	return s;
}

//--------------------------------------------------------------------------------------------------
// Name: Utilities::Tokenize
//
// Description:
//  Splits the given character string using the delimiter and
//  adds the tokens to the input vector.
// Remarks:
//  If the input is invalid and unable to be parsed, we throw an exception
//  token will be nullptr and we add nothing to the vector.
//
// Returns:
//	nothing.
//
void Utilities::Tokenize(
	char *input,
	const char *delimiter,
	vector<char *> *tokens)
{
	char *internalState = nullptr;
	char *token = strtok_r(input, delimiter, &internalState);

	// Use strtok_r iteratively to do the tokenization.
	//
	while (token != nullptr)
	{
		(*tokens).push_back(token);
		token = strtok_r(nullptr, delimiter, &internalState);
	}
}
