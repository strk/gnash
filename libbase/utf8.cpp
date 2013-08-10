// utf8.cpp: utilities for converting to and from UTF-8
// 
//   Copyright (C) 2008, 2009, 2010, 2011, 2012 Free Software Foundation, Inc.
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

#include <limits>
#include <boost/cstdint.hpp>
#include <string>
#include <vector>
#include <cstdlib>

namespace gnash {
namespace utf8 {

namespace {
    const boost::uint32_t invalid = std::numeric_limits<boost::uint32_t>::max();
}

std::wstring
decodeCanonicalString(const std::string& str, int version)
{
    
    std::wstring wstr;
    
    std::string::const_iterator it = str.begin(), e = str.end();
    
    if (version > 5) {
        while (boost::uint32_t code = decodeNextUnicodeCharacter(it, e)) {
            if (code == invalid) {
                continue;        
            }
            wstr.push_back(static_cast<wchar_t>(code));
        }
    }
    else {
        while (it != str.end()) {
            // This mangles UTF-8 (UCS4) strings, but is what is
            // wanted for SWF5.
            wstr.push_back(static_cast<unsigned char>(*it++));
        }
    }
    
    return wstr;

}

std::string
encodeCanonicalString(const std::wstring& wstr, int version)
{

    std::string str;
    
    std::wstring::const_iterator it = wstr.begin();
    while ( it != wstr.end())
    {
        if (version > 5) str.append(encodeUnicodeCharacter(*it++));
        else str.append(encodeLatin1Character(*it++));
    }
    
    return str;

}

std::string
encodeLatin1Character(boost::uint32_t ucsCharacter)
{
    std::string text;
    text.push_back(static_cast<unsigned char>(ucsCharacter));
    return text;
}


boost::uint32_t
decodeNextUnicodeCharacter(std::string::const_iterator& it,
                             const std::string::const_iterator& e)
{
    boost::uint32_t uc;

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

#define FIRST_BYTE(mask, shift)        \
    /* Post-increment iterator */ \
    uc = (*it++ & (mask)) << (shift);

#define NEXT_BYTE(shift)                        \
                    \
    if (it == e || *it == 0) return 0; /* end of buffer, do not advance */    \
    if ((*it & 0xC0) != 0x80) return invalid; /* standard check */    \
    /* Post-increment iterator: */        \
    uc |= (*it++ & 0x3F) << shift;

    if (it == e || *it == 0) return 0;    // End of buffer.  Do not advance.

    // Conventional 7-bit ASCII; return and increment iterator:
    if ((*it & 0x80) == 0) return static_cast<boost::uint32_t>(*it++);

    // Multi-byte sequences
    if ((*it & 0xE0) == 0xC0) {
        // Two-byte sequence.
        FIRST_BYTE(0x1F, 6);
        NEXT_BYTE(0);
        if (uc < 0x80) return invalid;    // overlong
        return uc;
    }
    else if ((*it & 0xF0) == 0xE0) {
        // Three-byte sequence.
        FIRST_BYTE(0x0F, 12);
        NEXT_BYTE(6);
        NEXT_BYTE(0);
        if (uc < 0x800) {
            return invalid;
        }
        return uc;
    }
    else if ((*it & 0xF8) == 0xF0) {
        // Four-byte sequence.
        FIRST_BYTE(0x07, 18);
        NEXT_BYTE(12);
        NEXT_BYTE(6);
        NEXT_BYTE(0);
        if (uc < 0x010000) return invalid;    // overlong
        return uc;
    }
    else {
        // Invalid.
        it++;
        return invalid;
    }
}

// TODO: buffer as std::string; index (iterator); 

std::string
encodeUnicodeCharacter(boost::uint32_t ucs_character)
{

    std::string text;

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
    else if (ucs_character <= 0xFFFF) {
        // Three bytes.
        text.push_back(0xE0 | (ucs_character >> 12));
        text.push_back(0x80 | ((ucs_character >> 6) & 0x3F));
        text.push_back(0x80 | ((ucs_character >> 0) & 0x3F));
    }
    else if (ucs_character <= 0x1FFFFF) {
        // Four bytes.
        text.push_back(0xF0 | (ucs_character >> 18));
        text.push_back(0x80 | ((ucs_character >> 12) & 0x3F));
        text.push_back(0x80 | ((ucs_character >> 6) & 0x3F));
        text.push_back(0x80 | ((ucs_character >> 0) & 0x3F));
    }
    else {
        // Invalid char; don't encode anything.
    }
    
    return text;
}


#define ENC_DEFAULT 0
#define ENC_UTF8 1
#define ENC_UTF16BE 2
#define ENC_UTF16LE 3

char*
stripBOM(char* in, size_t& size, TextEncoding& encoding)
{
    encoding = encUNSPECIFIED;
    if ( size > 2 )
    {
        // need *ptr to be unsigned or cast all 0xNN
        unsigned char* ptr = reinterpret_cast<unsigned char*>(in);

        if (*ptr == 0xFF && *(ptr+1) == 0xFE) {
            // Text is UTF-16 LE
            encoding = encUTF16LE;
            in+=2;
            size-=2;
        }
        else if ( *ptr == 0xFE && *(ptr+1) == 0xFF )
        {
            // Text is UTF-16 BE
            encoding = encUTF16BE;
            in+=2;
            size-=2;
        }
        else if (size > 3 && *ptr == 0xEF && *(ptr+1) == 0xBB &&
                *(ptr+2) == 0xBF )
        {
            // Text is UTF-8
            encoding = encUTF8;
            in+=3;
            size-=3;
        }
        else if ( size > 4 && *ptr == 0x00 && *(ptr+1) == 0x00 &&
                *(ptr+2) == 0xFE && *(ptr+3) == 0xFF )
        {
            // Text is UTF-32 BE
            encoding = encUTF32BE;
            in+=4;
            size-=4;
        }
        else if ( size > 4 && *ptr == 0xFF && *(ptr+1) == 0xFE &&
                *(ptr+2) == 0x00 && *(ptr+3) == 0x00 )
        {
            // Text is UTF-32 LE
            encoding = encUTF32LE;
            in+=4;
            size-=4;
        }

        // TODO: check other kinds of boms !
        // See http://en.wikipedia.org/wiki/Byte-order_mark#Representations_of_byte_order_marks_by_encoding
    }

    return in;
}

const char*
textEncodingName(TextEncoding enc)
{
    switch (enc)
    {
        case encUNSPECIFIED: return "Unspecified";
        case encUTF8: return "UTF8";
        case encUTF16BE: return "UTF16BE";
        case encUTF16LE: return "UTF16LE";
        case encUTF32BE: return "UTF32BE";
        case encUTF32LE: return "UTF32LE";
        case encSCSU: return "SCSU";
        case encUTF7: return "UTF7";
        case encUTFEBCDIC: return "UTFEBCDIC";
        case encBOCU1: return "BOCU1";
        default: return "INVALID";
    }
}

EncodingGuess
guessEncoding(const std::string &str, int &length, std::vector<int>& offsets)
{
    int width = 0; // The remaining width, not the total.
    bool is_sought = true;

    std::string::const_iterator it = str.begin();
    const std::string::const_iterator e = str.end();

    length = 0;
    
    // First, assume it's UTF8 and try to be wrong.
    while (it != e && is_sought) {
        ++length;

        offsets.push_back(it - str.begin()); // current position

        // Advances the iterator to point to the next 
        boost::uint32_t c = utf8::decodeNextUnicodeCharacter(it, e);

        if (c == utf8::invalid) {
            is_sought = false;
            break;
        }
    }

    offsets.push_back(it - str.begin()); // current position

    if (it == e && is_sought) {
        // No characters left, so it's almost certainly UTF8.
        return ENCGUESS_UNICODE;
    }

    it = str.begin();
    int index = 0;
    is_sought = true;
    width = 0;
    length = 0;
    bool was_odd = true;
    bool was_even = true;
    // Now, assume it's SHIFT_JIS and try to be wrong.
    while (it != e && is_sought) {
        int c = static_cast<int> (*it);

        if (width) {
            --width;
            if ((c < 0x40) || ((c < 0x9F) && was_even) ||
                ((c > 0x9E) && was_odd) || (c == 0x7F)) {
                is_sought = false;
            }
            continue;
        }

        ++length;
        offsets.push_back(index); // [length - 1] = index;

        if ((c == 0x80) || (c == 0xA0) || (c >= 0xF0)) {
            is_sought = false;
            break;
        }

        if (((c >= 0x81) && (c <= 0x9F)) || ((c >= 0xE0) && (c <= 0xEF))) {
            width = 1;
            was_odd = c & 0x01;
            was_even = !was_odd;
        }
    
        ++it;
        ++index;    
    }
    offsets.push_back(index); // [length - 1] = index;
    
    if (!width && is_sought) {
        // No width left, so it's probably SHIFT_JIS.
        return ENCGUESS_JIS;
    }

    // It's something else.
#ifdef ANDROID
    length = str.size();
#else
    length = std::mbstowcs(NULL, str.c_str(), 0);
#endif
    if (length == -1)
    {
        length = str.length();
    }
    return ENCGUESS_OTHER;
}


} // namespace utf8
} // namespace gnash

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
