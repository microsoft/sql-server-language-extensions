//**************************************************************************************************
// RExtension : A language extension implementing the SQL Server
// external language communication protocol for R.
// Copyright (C) 2020 Microsoft Corporation.
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
// @File: Unicode.h
//
// Purpose:
//  Functions providing lengths and conversion between utf16, utf8 encoded unicode characters.
//
//**************************************************************************************************
#pragma once

#include <string>

namespace estd
{
	std::u16string ToUtf16(const std::string &str);
	std::string ToUtf8(const std::u16string &str);

	std::u16string ToUtf16(const char *s, size_t len);
	std::string ToUtf8(const char16_t *s, size_t len);

	// These versions avoid memory allocation if the output string is big enough.
	// Useful for loops when multiple strings are converted.
	//
	void ToUtf16(const std::string &u8, std::u16string &u16);
	void ToUtf8(const std::u16string &str, std::string &u8);
	void ToUtf16(const char *s, size_t len, std::u16string &u16);
	void ToUtf8(const char16_t *s, size_t len, std::string &u8, bool throwOnError = false);

	// Returns the number of chars (bytes) in the string
	//
	size_t Utf8Size(const std::string &str);

	// Returns the number of chars (bytes) in the UTF8 representation of the string
	//
	size_t Utf8Size(const std::u16string &str);
	size_t Utf8Size(const char16_t *s, size_t l);

	// Returns the number of char16s in the string
	//
	size_t Utf16Size(const std::u16string &str);

	// Returns the number of char16s in the UTF16 representation of the string
	//
	size_t Utf16Size(const std::string &str);
	size_t Utf16Size(const char *s, size_t l);

	// Returns True if the string is a valid UTF8, false otherwise.
	//
	bool IsValidUTF8(const char* str, int strlen);
}
