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

/// Utilities to convert between std::string and std::wstring.
/// Strings in Gnash are generally stored as std::strings.
/// We have to deal, however, with characters larger than standard
/// ASCII (128), which can be encoded in two different ways.
///
/// SWF 6 and later use UTF-8, encoded as multibyte characters and
/// allowing many thousands of unique codes. Multibyte characters are 
/// difficult to handle, as their length - used for many string
/// operations - is not certain without parsing the string.
/// Converting the string to a wstring (generally a uint32_t - how
/// many codes the reference player can deal with is unknown)
/// facilitates string operations, as the length of the string
/// is equal to the number of valid characters. 
/// 
/// SWF5 and earlier, however, used the ISO-8859 specification,
/// allowing the standard 128 ASCII characters plus 128 extra
/// characters that depend on the particular subset of ISO-8859.
/// Characters are 8 bits, not the ASCII standard 7. SWF5 cannot
/// handle multi-byte characters without special functions.
///
/// It is important that SWF5 can distinguish between the two encodings,
/// so we cannot convert all strings to UTF-8.
///
/// Presently, this code is used for the AS String object,
/// edit_text_character, ord() and chr().

namespace utf8
{
	/// Converts a canonical std::string with multibyte characters into
	/// a std::wstring.
	/// @ param str the canonical string to convert
	/// @ param version the SWF version, used to decide how to decode the string.
	//
	/// For SWF5, UTF-8 (or any other) multibyte encoded characters are
	/// converted char by char, mangling the string. 
	DSOEXPORT std::wstring decodeCanonicalString(const std::string& str, int version);

	/// Converts a std::wstring into canonical std::string, depending on
	/// version.
	/// @ param wstr the wide string to convert
	/// @ param version the SWF version, used to decide how to encode the string.
	///
	/// For SWF 5, each character is stored as an 8-bit (at least) char, rather
	/// than converting it to a canonical UTF-8 byte sequence. Gnash can then
	/// distinguish between 8-bit characters, which it handles correctly, and 
	/// multi-byte characters, which are regarded as multiple characters for
	/// string methods. 
	DSOEXPORT std::string encodeCanonicalString(const std::wstring& wstr, int version);

	/// Return the next Unicode character in the UTF-8 encoded
	/// string.  Invalid UTF-8 sequences produce a U+FFFD character
	/// as output.  Advances string iterator past the character
	/// returned, unless the returned character is '\0', in which
	/// case the iterator does not advance.
	boost::uint32_t decodeNextUnicodeCharacter(std::string::const_iterator& it);

	/// Encodes the given wide character into a canonical
	/// string, theoretically up to 6 chars in length.
	std::string encodeUnicodeCharacter(boost::uint32_t ucs_character);
	
	/// Encodes the given wide character into an at least 8-bit character,
	/// allowing storage of Latin1 (ISO-8859-1) characters. This
	/// is the format of SWF5 and below.
	std::string encodeLatin1Character(boost::uint32_t ucsCharacter);
}


#endif // UTF8_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
