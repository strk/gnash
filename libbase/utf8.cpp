// utf8.cpp: utilities for converting to and from UTF-8
// 
//   Copyright (C) 2008 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
// Based on the public domain work of Thatcher Ulrich <tu@tulrich.com> 2004
//
// Much useful info at "UTF-8 and Unicode FAQ" http://www.cl.cam.ac.uk/~mgk25/unicode.html


#include "utf8.h"

// This isn't actually an invalid character; it's a valid char that
// looks like an inverted question mark.
#define INVALID_CHAR 0x0FFFD

std::wstring
utf8::decodeCanonicalString(const std::string& str, int version)
{
	
	std::wstring wstr = L"";
	
	std::string::const_iterator it = str.begin();
	
	if (version > 5)
	{
		while (boost::uint32_t code = decodeNextUnicodeCharacter(it))
		{
		    if (code == utf8::invalid)
		    {
			    wstr.push_back(static_cast<wchar_t>(INVALID_CHAR));
			    continue;	    
		    }
		    wstr.push_back(static_cast<wchar_t>(code));
		}
	}
	else
	{
		while (it != str.end())
		{
			// This mangles UTF-8 (UCS4) strings, but is what is
			// wanted for SWF5.
			wstr.push_back(static_cast<unsigned char>(*it++));
		}
	}
	
	return wstr;

}

std::string
utf8::encodeCanonicalString(const std::wstring& wstr, int version)
{

	std::string str = "";
	
	std::wstring::const_iterator it = wstr.begin();
	while ( it != wstr.end())
	{
		if (version > 5) str.append(encodeUnicodeCharacter(*it++));
		else str.append(encodeLatin1Character(*it++));
	}
	
	return str;

}

std::string
utf8::encodeLatin1Character(boost::uint32_t ucsCharacter)
{
	std::string text = "";
	text.push_back(static_cast<unsigned char>(ucsCharacter));
	return text;
}


boost::uint32_t
utf8::decodeNextUnicodeCharacter(std::string::const_iterator& it)
{
	boost::uint32_t	uc;

	// Security considerations:
	//
	// If we hit a zero byte, we want to return 0 without stepping
	// the buffer pointer past the 0.
	//
	// If we hit an "overlong sequence"; i.e. a character encoded
	// in a longer multibyte string than is necessary, then we
	// need to discard the character.  This is so attackers can't
	// disguise dangerous characters or character sequences --
	// there is only one valid encoding for each character.
	//
	// If we decode characters { 0xD800 .. 0xDFFF } or { 0xFFFE,
	// 0xFFFF } then we ignore them; they are not valid in UTF-8.

#define FIRST_BYTE(mask, shift)		\
	/* Post-increment iterator */ \
	uc = (*it++ & (mask)) << (shift);

#define NEXT_BYTE(shift)						\
					\
	if (*it == 0) return 0; /* end of buffer, do not advance */	\
	if ((*it & 0xC0) != 0x80) return utf8::invalid; /* standard check */	\
	/* Post-increment iterator: */		\
	uc |= (*it++ & 0x3F) << shift;

	if (*it == 0) return 0;	// End of buffer.  Do not advance.

	// Conventional 7-bit ASCII; return and increment iterator:
	if ((*it & 0x80) == 0) return (boost::uint32_t) *it++;

	// Multi-byte sequences
	if ((*it & 0xE0) == 0xC0)
	{
		// Two-byte sequence.
		FIRST_BYTE(0x1F, 6);
		NEXT_BYTE(0);
		if (uc < 0x80) return utf8::invalid;	// overlong
		return uc;
	}
	else if ((*it & 0xF0) == 0xE0)
	{
		// Three-byte sequence.
		FIRST_BYTE(0x0F, 12);
		NEXT_BYTE(6);
		NEXT_BYTE(0);
		if (uc < 0x800) return utf8::invalid;	// overlong
		if (uc >= 0x0D800 && uc <= 0x0DFFF) return utf8::invalid;	// not valid ISO 10646
		if (uc == 0x0FFFE || uc == 0x0FFFF) return utf8::invalid;	// not valid ISO 10646
		return uc;
	}
	else if ((*it & 0xF8) == 0xF0)
	{
		// Four-byte sequence.
		FIRST_BYTE(0x07, 18);
		NEXT_BYTE(12);
		NEXT_BYTE(6);
		NEXT_BYTE(0);
		if (uc < 0x010000) return utf8::invalid;	// overlong
		return uc;
	}
	else if ((*it & 0xFC) == 0xF8)
	{
		// Five-byte sequence.
		FIRST_BYTE(0x03, 24);
		NEXT_BYTE(18);
		NEXT_BYTE(12);
		NEXT_BYTE(6);
		NEXT_BYTE(0);
		if (uc < 0x0200000) return utf8::invalid;	// overlong
		return uc;
	}
	else if ((*it & 0xFE) == 0xFC)
	{
		// Six-byte sequence.
		FIRST_BYTE(0x01, 30);
		NEXT_BYTE(24);
		NEXT_BYTE(18);
		NEXT_BYTE(12);
		NEXT_BYTE(6);
		NEXT_BYTE(0);
		if (uc < 0x04000000) return utf8::invalid;	// overlong
		return uc;
	}
	else
	{
		// Invalid.
		it++;
		return utf8::invalid;
	}
}

// TODO: buffer as std::string; index (iterator); 

std::string
utf8::encodeUnicodeCharacter(boost::uint32_t ucs_character)
{

	std::string text = "";

	if (ucs_character <= 0x7F)
	{
		// Plain single-byte ASCII.
		text.push_back(ucs_character);
	}
	else if (ucs_character <= 0x7FF)
	{
		// Two bytes.
		text.push_back(0xC0 | (ucs_character >> 6));
		text.push_back(0x80 | ((ucs_character >> 0) & 0x3F));
	}
	else if (ucs_character <= 0xFFFF)
	{
		// Three bytes.
		text.push_back(0xE0 | (ucs_character >> 12));
		text.push_back(0x80 | ((ucs_character >> 6) & 0x3F));
		text.push_back(0x80 | ((ucs_character >> 0) & 0x3F));
	}
	else if (ucs_character <= 0x1FFFFF)
	{
		// Four bytes.
		text.push_back(0xF0 | (ucs_character >> 18));
		text.push_back(0x80 | ((ucs_character >> 12) & 0x3F));
		text.push_back(0x80 | ((ucs_character >> 6) & 0x3F));
		text.push_back(0x80 | ((ucs_character >> 0) & 0x3F));
	}
	else if (ucs_character <= 0x3FFFFFF)
	{
		// Five bytes.
		text.push_back(0xF8 | (ucs_character >> 24));
		text.push_back(0x80 | ((ucs_character >> 18) & 0x3F));
		text.push_back(0x80 | ((ucs_character >> 12) & 0x3F));
		text.push_back(0x80 | ((ucs_character >> 6) & 0x3F));
		text.push_back(0x80 | ((ucs_character >> 0) & 0x3F));
	}
	else if (ucs_character <= 0x7FFFFFFF)
	{
		// Six bytes.
		text.push_back(0xFC | (ucs_character >> 30));
		text.push_back(0x80 | ((ucs_character >> 24) & 0x3F));
		text.push_back(0x80 | ((ucs_character >> 18) & 0x3F));
		text.push_back(0x80 | ((ucs_character >> 12) & 0x3F));
		text.push_back(0x80 | ((ucs_character >> 6) & 0x3F));
		text.push_back(0x80 | ((ucs_character >> 0) & 0x3F));
	}
	else
	{
		// Invalid char; don't encode anything.
	}
	
	return text;
}

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
