// utf8.h: utilities for converting to and from UTF-8
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

#ifndef UTF8_H
#define UTF8_H

#include "tu_config.h" // For DSOEXPORT
#include <string>
#include <boost/cstdint.hpp> // for boost::?int??_t


namespace utf8
{
	// Converts a UTF-8 encoded std::string with multibyte characters into
	// a std::wstring.
	DSOEXPORT std::wstring decodeCanonicalString(const std::string& str, int version);

	// Converts a std::wstring into a UTF-8 encoded std::string.
	DSOEXPORT std::string encodeCanonicalString(const std::wstring& wstr, int version);

	// Return the next Unicode character in the UTF-8 encoded
	// string.  Invalid UTF-8 sequences produce a U+FFFD character
	// as output.  Advances string iterator past the character
	// returned, unless the returned character is '\0', in which
	// case the iterator does not advance.
	boost::uint32_t decodeNextUnicodeCharacter(std::string::const_iterator& it);

	// Encodes the given UCS character into the given UTF-8
	// buffer.  Writes the data starting at buffer[offset], and
	// increments offset by the number of bytes written.
	//
	// May write up to 6 bytes, so make sure there's room in the
	// buffer!
	std::string encodeUnicodeCharacter(boost::uint32_t ucs_character);
	
	std::string encodeLatin1Character(boost::uint32_t ucsCharacter);
}


#endif // UTF8_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
