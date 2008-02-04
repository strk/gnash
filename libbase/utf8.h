// utf8.h	-- Thatcher Ulrich <tu@tulrich.com> 2004

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.  THE AUTHOR DOES NOT WARRANT THIS CODE.

// Utility code for dealing with UTF-8 encoded text.


#ifndef UTF8_H
#define UTF8_H

#include "tu_config.h" // needed ?
#include <string>

#include <boost/cstdint.hpp> // for boost::?int??_t


namespace utf8
{
	// Return the next Unicode character in the UTF-8 encoded
	// string.  Invalid UTF-8 sequences produce a U+FFFD character
	// as output.  Advances string iterator past the character
	// returned, unless the returned character is '\0', in which
	// case the iterator does not advance.
	DSOEXPORT boost::uint32_t decodeNextUnicodeCharacter(std::string::const_iterator& it);

	// Encodes the given UCS character into the given UTF-8
	// buffer.  Writes the data starting at buffer[offset], and
	// increments offset by the number of bytes written.
	//
	// May write up to 6 bytes, so make sure there's room in the
	// buffer!
	DSOEXPORT std::string encodeUnicodeCharacter(boost::uint32_t ucs_character);
}


#endif // UTF8_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
