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
// @File: Unicode.cpp
//
// Purpose:
// Functions providing lengths and conversion between utf16, utf8 encoded unicode character strings.
//
//**************************************************************************************************
#include <algorithm>
#include <cassert>
#include <string>
#include <stdexcept>

#ifdef _WIN64
#include <Windows.h>
#else
#include <iconv.h>
#endif // _WIN64

#include "Unicode.h"

namespace estd
{

// See CppCon2018: The C++ Conference 2018 https://www.youtube.com/watch?v=5FQ87-Ecb-A
//  Fast Conversion From UTF-8 with C++, DFAs, and SSE Intrinsics
//

constexpr uint16_t BadChar = 0xfffd;
constexpr size_t OneChar16Len = 1;
constexpr size_t TwoChar16Len = 2;

constexpr size_t OneChar8Len = 1;
constexpr size_t TwoChar8Len = 2;
constexpr size_t ThreeChar8Len = 3;
constexpr size_t FourChar8Len = 4;

//--------------------------------------------------------------------------------------------------
// Name: CharClass
//
// Description:
//  Enum listing different character classes.
//
enum CharClass : uint8_t
{
	ILL = 0,

	ASC = 1,    // 00..7f   ASCII leading byte range

	CR1 = 2,
	CR2 = 3,
	CR3 = 4,

	L2A = 5,
	L3A = 6,
	L3B = 7,
	L3C = 8,
	L4A = 9,
	L4B = 10,
	L4C = 11
};

//--------------------------------------------------------------------------------------------------
// Name: State
//
// Description:
//  DFA's states
//
enum State : uint8_t
{
	BGN = 0,
	ERR = 12,

	CS1 = 24,
	CS2 = 36,
	CS3 = 48,

	P3A = 60,
	P3B = 72,

	P4A = 84,
	P4B = 96,

	END = BGN,
	err = ERR
};

// -------------------------------------------------------------------------------------------------
// Name: FirstUnitInfo
//
// Description:
//  Info for the first character in a multi-byte UTF-8 sequence
//
struct FirstUnitInfo
{
	unsigned char value;
	State nextState;
};

// -------------------------------------------------------------------------------------------------
// Name: LookupTables
//
// Description:
//  The structure and definition for lookupTables
//
struct alignas(2048) LookupTables
{
	CharClass charToCharClass[256];
	FirstUnitInfo charToUnitInfo[256];
	State transition[108];
};

LookupTables lookupTables =
{
	{
	//   0    1    2    3    4    5    6    7    8    9    a    b    c    d    e    f
	//===========================================================================================
		ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, // 00..0f
		ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, // 10..1f
		ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, // 20..2f
		ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, // 30..3f

		ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, // 40..4f
		ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, // 50..5f
		ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, // 60..6f
		ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, ASC, // 70..7f

		CR1, CR1, CR1, CR1, CR1, CR1, CR1, CR1, CR1, CR1, CR1, CR1, CR1, CR1, CR1, CR1, // 80..8f
		CR2, CR2, CR2, CR2, CR2, CR2, CR2, CR2, CR2, CR2, CR2, CR2, CR2, CR2, CR2, CR2, // 90..9f
		CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, // a0..af
		CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, CR3, // b0..bf

		ILL, ILL, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, // c0..cf
		L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, L2A, // d0..df
		L3A, L3B, L3B, L3B, L3B, L3B, L3B, L3B, L3B, L3B, L3B, L3B, L3B, L3B, L3B, L3B, // e0..ef
		L4A, L4B, L4B, L4B, L4C, ILL, ILL, ILL, ILL, ILL, ILL, ILL, ILL, ILL, ILL, ILL  // f0..ff
	},
	{
		// 00..7f
		{0x00, BGN}, {0x01, BGN}, {0x02, BGN}, {0x03, BGN},
		{0x04, BGN}, {0x05, BGN}, {0x06, BGN}, {0x07, BGN},
		{0x08, BGN}, {0x09, BGN}, {0x0a, BGN}, {0x0b, BGN},
		{0x0c, BGN}, {0x0d, BGN}, {0x0e, BGN}, {0x0f, BGN},
		{0x10, BGN}, {0x11, BGN}, {0x12, BGN}, {0x13, BGN},
		{0x14, BGN}, {0x15, BGN}, {0x16, BGN}, {0x17, BGN},
		{0x18, BGN}, {0x19, BGN}, {0x1a, BGN}, {0x1b, BGN},
		{0x1c, BGN}, {0x1d, BGN}, {0x1e, BGN}, {0x1f, BGN},
		{0x20, BGN}, {0x21, BGN}, {0x22, BGN}, {0x23, BGN},
		{0x24, BGN}, {0x25, BGN}, {0x26, BGN}, {0x27, BGN},
		{0x28, BGN}, {0x29, BGN}, {0x2a, BGN}, {0x2b, BGN},
		{0x2c, BGN}, {0x2d, BGN}, {0x2e, BGN}, {0x2f, BGN},
		{0x30, BGN}, {0x31, BGN}, {0x32, BGN}, {0x33, BGN},
		{0x34, BGN}, {0x35, BGN}, {0x36, BGN}, {0x37, BGN},
		{0x38, BGN}, {0x39, BGN}, {0x3a, BGN}, {0x3b, BGN},
		{0x3c, BGN}, {0x3d, BGN}, {0x3e, BGN}, {0x3f, BGN},
		{0x40, BGN}, {0x41, BGN}, {0x42, BGN}, {0x43, BGN},
		{0x44, BGN}, {0x45, BGN}, {0x46, BGN}, {0x47, BGN},
		{0x48, BGN}, {0x49, BGN}, {0x4a, BGN}, {0x4b, BGN},
		{0x4c, BGN}, {0x4d, BGN}, {0x4e, BGN}, {0x4f, BGN},
		{0x50, BGN}, {0x51, BGN}, {0x52, BGN}, {0x53, BGN},
		{0x54, BGN}, {0x55, BGN}, {0x56, BGN}, {0x57, BGN},
		{0x58, BGN}, {0x59, BGN}, {0x5a, BGN}, {0x5b, BGN},
		{0x5c, BGN}, {0x5d, BGN}, {0x5e, BGN}, {0x5f, BGN},
		{0x60, BGN}, {0x61, BGN}, {0x62, BGN}, {0x63, BGN},
		{0x64, BGN}, {0x65, BGN}, {0x66, BGN}, {0x67, BGN},
		{0x68, BGN}, {0x69, BGN}, {0x6a, BGN}, {0x6b, BGN},
		{0x6c, BGN}, {0x6d, BGN}, {0x6e, BGN}, {0x6f, BGN},
		{0x70, BGN}, {0x71, BGN}, {0x72, BGN}, {0x73, BGN},
		{0x74, BGN}, {0x75, BGN}, {0x76, BGN}, {0x77, BGN},
		{0x78, BGN}, {0x79, BGN}, {0x7a, BGN}, {0x7b, BGN},
		{0x7c, BGN}, {0x7d, BGN}, {0x7e, BGN}, {0x7f, BGN},

		// 80..8f
		{0x80, ERR}, {0x81, ERR}, {0x82, ERR}, {0x83, ERR},
		{0x84, ERR}, {0x85, ERR}, {0x86, ERR}, {0x87, ERR},
		{0x88, ERR}, {0x89, ERR}, {0x8a, ERR}, {0x8b, ERR},
		{0x8c, ERR}, {0x8d, ERR}, {0x8e, ERR}, {0x8f, ERR},

		// 90..9f
		{0x90, ERR}, {0x91, ERR}, {0x92, ERR}, {0x93, ERR},
		{0x94, ERR}, {0x95, ERR}, {0x96, ERR}, {0x97, ERR},
		{0x98, ERR}, {0x99, ERR}, {0x9a, ERR}, {0x9b, ERR},
		{0x9c, ERR}, {0x9d, ERR}, {0x9e, ERR}, {0x9f, ERR},

		// a0..af
		{0xa0, ERR}, {0xa1, ERR}, {0xa2, ERR}, {0xa3, ERR},
		{0xa4, ERR}, {0xa5, ERR}, {0xa6, ERR}, {0xa7, ERR},
		{0xa8, ERR}, {0xa9, ERR}, {0xaa, ERR}, {0xab, ERR},
		{0xac, ERR}, {0xad, ERR}, {0xae, ERR}, {0xaf, ERR},

		// b0..bf
		{0xb0, ERR}, {0xb1, ERR}, {0xb2, ERR}, {0xb3, ERR},
		{0xb4, ERR}, {0xb5, ERR}, {0xb6, ERR}, {0xb7, ERR},
		{0xb8, ERR}, {0xb9, ERR}, {0xba, ERR}, {0xbb, ERR},
		{0xbc, ERR}, {0xbd, ERR}, {0xbe, ERR}, {0xbf, ERR},

		// c0..cf
		{0xc0, ERR}, {0xc1, ERR}, {0x02, CS1}, {0x03, CS1},
		{0x04, CS1}, {0x05, CS1}, {0x06, CS1}, {0x07, CS1},
		{0x08, CS1}, {0x09, CS1}, {0x0a, CS1}, {0x0b, CS1},
		{0x0c, CS1}, {0x0d, CS1}, {0x0e, CS1}, {0x0f, CS1},

		// d0..df
		{0x10, CS1}, {0x11, CS1}, {0x12, CS1}, {0x13, CS1},
		{0x14, CS1}, {0x15, CS1}, {0x16, CS1}, {0x17, CS1},
		{0x18, CS1}, {0x19, CS1}, {0x1a, CS1}, {0x1b, CS1},
		{0x1c, CS1}, {0x1d, CS1}, {0x1e, CS1}, {0x1f, CS1},

		// e0..ef
		{0x00, P3A}, {0x01, CS2}, {0x02, CS2}, {0x03, CS2},
		{0x04, CS2}, {0x05, CS2}, {0x06, CS2}, {0x07, CS2},
		{0x08, CS2}, {0x09, CS2}, {0x0a, CS2}, {0x0b, CS2},
		{0x0c, CS2}, {0x0d, P3B}, {0x0e, CS2}, {0x0f, CS2},

		// f0..ff
		{0x00, P4A}, {0x01, CS3}, {0x02, CS3}, {0x03, CS3},
		{0x04, P4B}, {0x05, ERR}, {0x06, ERR}, {0x07, ERR},
		{0x08, ERR}, {0x09, ERR}, {0x0a, ERR}, {0x0b, ERR},
		{0x0c, ERR}, {0x0d, ERR}, {0x0e, ERR}, {0x0f, ERR}
	},
	{
	//  ILL  ASC  CR1  CR2  CR3  L2A  L3A  L3B  L3C  L4A  L4B  L4C
	//==============================================================
		err, END, err, err, err, CS1, P3A, CS2, P3B, P4A, CS3, P4B, // BGN|END (0)
		err, err, err, err, err, err, err, err, err, err, err, err, // ERR (12)

		err, err, END, END, END, err, err, err, err, err, err, err, // CS1 (24)
		err, err, CS1, CS1, CS1, err, err, err, err, err, err, err, // CS2 (36)
		err, err, CS2, CS2, CS2, err, err, err, err, err, err, err, // CS3 (48)

		err, err, err, err, CS1, err, err, err, err, err, err, err, // P3A (60)
		err, err, CS1, CS1, err, err, err, err, err, err, err, err, // P3B (72)

		err, err, err, CS2, CS2, err, err, err, err, err, err, err, // P4A (84)
		err, err, CS2, err, err, err, err, err, err, err, err, err  // P4B (96)
	}
};

// -------------------------------------------------------------------------------------------------
// Name: GetCharClass
//
// Description:
//  Gets the character class
//
static inline CharClass
GetCharClass(uint8_t u)
{
	return lookupTables.charToCharClass[u];
}

// -------------------------------------------------------------------------------------------------
// Name: GetFirstUnitInfo
//
// Description:
//  Gets the first unit info
//
static inline const FirstUnitInfo &
GetFirstUnitInfo(uint8_t u)
{
	return lookupTables.charToUnitInfo[u];
}

// -------------------------------------------------------------------------------------------------
// Name: GetFirstUnitInfo
//
// Description:
//  Gets the next state
//
static inline State
GetNextState(uint8_t u)
{
	return lookupTables.transition[u];
}

// -------------------------------------------------------------------------------------------------
// Name: GetChar16
//
// Description:
//  Converts a UTF32 character to UTF16 and return the number of char16_t units in len.
//
static inline void
GetChar16(char32_t char32, char16_t *char16, size_t *len)
{
	if (char32 <= 0xdf77 || (char32 >= 0xe000 && char32 <= 0xffff))
	{
		*char16 = static_cast<char16_t>(char32);
		*len = OneChar16Len;
	}
	else if (char32 >= 0x10000 && char32 <= 0x10ffff)
	{
		char32 -= 0x10000;
		*char16 = static_cast<char16_t>(0xd800 + (char32 >> 10));
		*(char16 + 1) = static_cast<char16_t>(0xdc00 + (char32 & 0x0000003ff));
		*len = TwoChar16Len;
	}
	else
	{
		*char16 = BadChar;
		*len = OneChar16Len;
	}
}

// -------------------------------------------------------------------------------------------------
// Name: GetChar16Len
//
// Description:
//  Gets the length of the UTF16 representation of a UTF32 character
//
static inline size_t
GetChar16Len(char32_t char32)
{
	size_t ans;
	if (char32 <= 0xdf77 || (char32 >= 0xe000 && char32 <= 0xffff))
	{
		ans = OneChar16Len;
	}
	else if (char32 >= 0x10000 && char32 <= 0x10ffff)
	{
		ans = TwoChar16Len;
	}
	else
	{
		// Bad character
		//
		ans = OneChar16Len;
	}
	return ans;
}

// -------------------------------------------------------------------------------------------------
// Name: GetChar32
//
// Description:
//  Converts a UTF16 character to UTF32 and return the number of char16s in len.
//
static inline void
GetChar32(
	const char16_t   *char16,
	size_t           char16Len,
	char32_t         *char32,
	size_t           *len,
	bool             throwOnError = false)
{
	if (*char16 <= 0xd7ff || *char16 >= 0xe000)
	{
		*char32 = *char16;
		*len = OneChar16Len;
	}
	else if (*char16 <= 0xdbff && char16Len > 1 && char16[1] >= 0xdc00 && char16[1] <= 0xdfff)
	{
		*char32 =
			0x10000 +
			(((static_cast<char32_t>(char16[0]) - 0xd800) << 10) |
			(static_cast<char32_t>(char16[1]) - 0xdc00));
		*len = TwoChar16Len;
	}
	else if (throwOnError)
	{
		throw std::invalid_argument("There is a bad UTF-16 character");
	}
	else
	{
		// Not a valid UTF16 sequence. WideCharToMultiByte replaces invalid characters
		// with 0xfffd, unless the WC_ERR_INVALID_CHARS flag is used. In the name to
		// avoid errors, our implementation follows the same approach.
		//
		*char32 = BadChar;
		*len = OneChar16Len;
	}
}

// -------------------------------------------------------------------------------------------------
// Name: Handle2ByteUtf8
//
// Description:
//  Handles utf8 characters that are 2 bytes long.
//
static inline void
Handle2ByteUtf8(const uint8_t* p, size_t char8Len, char32_t* char32, size_t* len, bool* ill)
{
	const FirstUnitInfo& fui = GetFirstUnitInfo(*p);
	State state = fui.nextState;
	if (char8Len > 1)
	{
		++p;
		*char32 = (fui.value << 6) | (*p & 0x3f);
		state = GetNextState(state + GetCharClass(*p));
	}

	if (state == State::END)
	{
		*len = TwoChar8Len;
		*ill = false;
	}
	else
	{
		*len = OneChar8Len;
		*char32 = BadChar;
		*ill = true;
	}
}

// -------------------------------------------------------------------------------------------------
// Name: HandleUtf8
//
// Description:
//  Templatized function that handles utf8 characters that are 3 or 4 bytes long.
//
template<size_t Bytes>
static inline void
HandleUtf8(const uint8_t* char8, size_t char8Len, char32_t* char32, size_t* len, bool* ill)
{
	const uint8_t* p = char8;
	if (char8Len >= Bytes)
	{
		const FirstUnitInfo& fui = GetFirstUnitInfo(*p);
		State state = fui.nextState;
		*char32 = fui.value;

		// Fill up char32 with UTF-32 encoding using the bytes from UTF-8
		// encoded character ensuring the state doesn't become ERR.
		//
		++p;
		for (size_t i = 1; i < Bytes && state != ERR; ++i, ++p)
		{
			*char32 = (*char32 << 6) | (*p & 0x3f);
			state = GetNextState(state + GetCharClass(*p));
		}

		if (state == State::END)
		{
			*len = Bytes;
			*ill = false;
		}
		else
		{
			if (state == State::ERR)
			{
				*len = p - char8 - 1;
				if (*len == 1 && ((*(char8 + 1) & 0xc0) == 0x80))
				{
					*len = TwoChar8Len;
				}
			}
			else
			{
				*len = TwoChar8Len;
			}

			*char32 = BadChar;
			*ill = true;
		}
	}
	else
	{
		const FirstUnitInfo& fui = GetFirstUnitInfo(*p);
		State state = fui.nextState;

		// When the char8Len is less than the expected Bytes, its a bad utf-32
		// character; nevertheless process the utf-8 characters to find the
		// length of the bad character.
		//
		size_t processed = 1;
		while (processed < char8Len)
		{
			++p;
			state = GetNextState(state + GetCharClass(*p));
			if (state == State::ERR)
			{
				break;
			}
			++processed;
		}

		if (processed == 1 && char8Len > 1 && ((*(char8 + 1) & 0xc0) == 0x80))
		{
			*len = TwoChar8Len;
		}
		else
		{
			*len = processed;
		}

		*char32 = BadChar;
		*ill = true;
	}
}

// -------------------------------------------------------------------------------------------------
// Name: GetChar32
//
// Description:
//  Converts a UTF8 character to UTF32
//
static inline void
GetChar32(const char *char8, size_t char8Len, char32_t *char32, size_t *len, bool *ill)
{
	*ill = false;

	const uint8_t* p = reinterpret_cast<const uint8_t*>(char8);

	if (*p <= 0x7f)
	{
		// Handle ASCII separately
		//
		*char32 = static_cast<unsigned char>(*p);
		*len = OneChar8Len;
	}
	else
	{
		CharClass charClass = GetCharClass(*p);
		switch (charClass)
		{
		case CharClass::L2A:
			Handle2ByteUtf8(p, char8Len, char32, len, ill);
			break;

		case CharClass::L3A:
		case CharClass::L3B:
		case CharClass::L3C:
			HandleUtf8<3>(p, char8Len, char32, len, ill);
			break;

		case CharClass::L4A:
		case CharClass::L4B:
		case CharClass::L4C:
			HandleUtf8<4>(p, char8Len, char32, len, ill);
			break;

		default:
			// Invalid character
			*ill = true;
			*char32 = BadChar;
			*len = OneChar8Len;
		}
	}
}

// -------------------------------------------------------------------------------------------------
// Name: GetChar8Len
//
// Description:
//  Gets the length of the UTF8 representation of a UTF32 character
//
static inline size_t
GetChar8Len(char32_t char32) noexcept
{
	size_t ans;
	if (char32 < 0x80)
	{
		ans = OneChar8Len;
	}
	else if (char32 < 0x800)
	{
		ans = TwoChar8Len;
	}
	else if (char32 < 0x10000)
	{
		ans = ThreeChar8Len;
	}
	else
	{
		ans = FourChar8Len;
	}
	return ans;
}

// -------------------------------------------------------------------------------------------------
// Name: GetChar8
//
// Description:
//  Converts UTF32 to UTF8 and return the number of char8s in charLen.
//
static inline void
GetChar8(char32_t char32, char *char8, size_t *charLen) noexcept
{
	if (char32 < 0x80)
	{
		*char8 = static_cast<char>(char32);
		*charLen = OneChar8Len;
	}
	else if (char32 < 0x800)
	{
		char8[1] = 0x80 | (char32 & 0x3f);
		char32 >>= 6;

		char8[0] = static_cast<char>(0xc0 | char32);
		*charLen = TwoChar8Len;
	}
	else if (char32 < 0x10000)
	{
		char8[2] = 0x80 | (char32 & 0x3f);
		char32 >>= 6;

		char8[1] = 0x80 | (char32 & 0x3f);
		char32 >>= 6;

		char8[0] = static_cast<char>(0xe0 | char32);
		*charLen = ThreeChar8Len;
	}
	else
	{
		char8[3] = 0x80 | (char32 & 0x3f);
		char32 >>= 6;

		char8[2] = 0x80 | (char32 & 0x3f);
		char32 >>= 6;

		char8[1] = 0x80 | (char32 & 0x3f);
		char32 >>= 6;

		char8[0] = static_cast<char>(0xf0 | char32);
		*charLen = FourChar8Len;
	}
}

// -------------------------------------------------------------------------------------------------
// Name: ToUtf8
//
// Description:
//  Converts the given char16_t to an utf-8 encoded string
//
void ToUtf8(const char16_t *s, size_t len, std::string &ans, bool throwOnError)
{
	ans.resize(Utf8Size(s, len));
	size_t i = 0;
	size_t j = 0;
	while (i < len)
	{
		char32_t char32;
		size_t char16Len;

		GetChar32(s+i, len-i, &char32, &char16Len, throwOnError);
		i += char16Len;

		size_t char8Len;
		GetChar8(char32, &ans[j], &char8Len);
		j += char8Len;
	}
}

// -------------------------------------------------------------------------------------------------
// Name: ToUtf8
//
// Description:
//  Converts the given u16string to an utf8 encoded string
//
void ToUtf8(const std::u16string &str, std::string &ans)
{
	ToUtf8(str.data(), str.size(), ans);
}

// -------------------------------------------------------------------------------------------------
// Name: ToUtf8
//
// Description:
//  Converts the given char16_t and size l to an utf-8 encoded string and return it back
//
std::string ToUtf8(const char16_t *s, size_t l)
{
	std::string ans;
	ToUtf8(s, l, ans);
	return ans;
}

// -------------------------------------------------------------------------------------------------
// Name: ToUtf8
//
// Description:
//  Converts the given u16string to an utf-8 encoded string
//
std::string ToUtf8(const std::u16string &str)
{
	return ToUtf8(str.data(), str.size());
}

// -------------------------------------------------------------------------------------------------
// Name: ToUtf16
//
// Description:
//  Converts the given char string with the given length to an utf-16 encoded u16string
//
void ToUtf16(const char *s, size_t l, std::u16string &ans)
{
	ans.resize(0);

	size_t i = 0;
	size_t j = 0;

	char32_t char32 = 0;
	size_t char8Len;
	char16_t char16s[2];
	size_t char16Len = 0;

	bool isIll;

	while (i < l)
	{
		GetChar32(s+i, l-i, &char32, &char8Len, &isIll);
		if (!isIll)
		{
			GetChar16(char32, char16s, &char16Len);
			j += char16Len;
		}
		else
		{
			size_t char16Len;
			GetChar16(char32, char16s, &char16Len);
			j += char16Len;
		}

		switch(char16Len)
		{
		case 1:
			ans.push_back(char16s[0]);
			break;
		case 2:
			ans.push_back(char16s[0]);
			ans.push_back(char16s[1]);
			break;
		}

		i += char8Len;
	}
}

// -------------------------------------------------------------------------------------------------
// Name: ToUtf16
//
// Description:
//  Converts the given char string with the given length to an utf-16 encoded u16string
//
void ToUtf16(const std::string &str, std::u16string &ans)
{
	ToUtf16(str.data(), str.size(), ans);
}

// -------------------------------------------------------------------------------------------------
// Name: ToUtf16
//
// Description:
//  Converts the given char string with the given length to an utf-16 encoded u16string
//
std::u16string ToUtf16(const char *s, size_t l)
{
	std::u16string ans;
	ToUtf16(s, l, ans);
	return ans;
}

// -------------------------------------------------------------------------------------------------
// Name: ToUtf16
//
// Description:
//  Converts the given char string to an utf-16 encoded u16string
//
std::u16string ToUtf16(const std::string &str)
{
	return ToUtf16(str.data(), str.size());
}

// -------------------------------------------------------------------------------------------------
// Name: Utf8Size
//
// Description:
//  Returns the number of chars (bytes) in the string. Same as str.size().
//
size_t Utf8Size(const std::string &str)
{
	return str.size();
}

// -------------------------------------------------------------------------------------------------
// Name: Utf8Size
//
// Description:
//  Returns the number of chars (bytes) in the UTF8 representation of the string
//
size_t Utf8Size(const char16_t *s, size_t l)
{
	size_t ans = 0;
	size_t i = 0;
	while (i < l)
	{
		char32_t char32;
		size_t char16Len;

		GetChar32(s + i, l - i, &char32, &char16Len);
		ans += GetChar8Len(char32);
		i += char16Len;
	}
	return ans;
}

// -------------------------------------------------------------------------------------------------
// Name: Utf8Size
//
// Description:
//  Returns the number of chars (bytes) in the UTF8 representation of the string
//
size_t Utf8Size(const std::u16string &str)
{
	return Utf8Size(str.data(), str.size());
}

// -------------------------------------------------------------------------------------------------
// Name: Utf16Size
//
// Description:
//  Returns the number of char16s in the string. Same as str.size().
//
size_t Utf16Size(const std::u16string &str)
{
	return str.size();
}

// -------------------------------------------------------------------------------------------------
// Name: Utf16Size
//
// Description:
//  Returns the number of char16s in the UTF16 representation of the string
//
size_t Utf16Size(const char *s, size_t l)
{
	size_t ans = 0;
	size_t i = 0;
	while(i < l)
	{
		char32_t char32;
		size_t char8Len;
		bool isIll;

		GetChar32(s + i, l - i, &char32, &char8Len, &isIll);
		if (!isIll)
		{
			ans += GetChar16Len(char32);
		}
		else
		{
			ans += OneChar16Len;
		}
		i += char8Len;
	}
	return ans;
}

// -------------------------------------------------------------------------------------------------
// Name: Utf16Size
//
// Description:
//  Returns the number of char16s in the UTF16 representation of the string
//
size_t Utf16Size(const std::string &str)
{
	return Utf16Size(str.data(), str.size());
}

// -------------------------------------------------------------------------------------------------
// Name: IsValidUTF8
//
// Description:
// Returns True if the string is a valid UTF8, false otherwise.
//
bool IsValidUTF8(const char* str, int numberOfBytesInStr)
{
	int trailingBytesToValidate = 0;
	
	for (int i = 0; i < numberOfBytesInStr; ++i)
	{
		int c = str[i];
		if (0x00 <= c && c <= 0x7f) 
		{
			// 0xxxxxxx -> One byte
			//
			trailingBytesToValidate = OneChar8Len - 1;
		}
		else if ((c & 0xE0) == 0xC0)
		{
			// 110xxxxx -> Two bytes
			//
			trailingBytesToValidate = TwoChar8Len - 1;
		}
		else if ((c & 0xF0) == 0xE0)
		{
			// 1110xxxx -> Three bytes
			//
			trailingBytesToValidate = ThreeChar8Len - 1;
		}
		else if ((c & 0xF8) == 0xF0)
		{
			// 11110xxx -> Four bytes
			//
			trailingBytesToValidate = FourChar8Len - 1;
		}
		else 
		{
			return false;
		}
		
		// trailingBytesToValidate bytes should match 10xxxxxx
		//
		for (int j = 0; j < trailingBytesToValidate && i < numberOfBytesInStr; ++j)
		{
			++i;

			// Expect to have trailingBytesToValidate bytes, but ended soon.
			//
			if (i == numberOfBytesInStr)
			{
				return false;
			}

			// If following byte does not match 10xxxxxx
			//
			if (((unsigned char)str[i] & 0xC0) != 0x80)
			{
				return false;
			}
		}
	}

	return true;
}
} // namespace estd
