// utf8.cpp: utilities for converting to and from UTF-8
// 
//   Copyright (C) 2008, 2009, 2010 Free Software Foundation, Inc.
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
    
    std::wstring wstr;
    
    std::string::const_iterator it = str.begin(), e = str.end();
    
    if (version > 5)
    {
        while (boost::uint32_t code = decodeNextUnicodeCharacter(it, e))
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
utf8::encodeLatin1Character(boost::uint32_t ucsCharacter)
{
    std::string text;
    text.push_back(static_cast<unsigned char>(ucsCharacter));
    return text;
}


boost::uint32_t
utf8::decodeNextUnicodeCharacter(std::string::const_iterator& it,
                                 const std::string::const_iterator& e)
{
    boost::uint32_t    uc;

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
    if ((*it & 0xC0) != 0x80) return utf8::invalid; /* standard check */    \
    /* Post-increment iterator: */        \
    uc |= (*it++ & 0x3F) << shift;

    if (it == e || *it == 0) return 0;    // End of buffer.  Do not advance.

    // Conventional 7-bit ASCII; return and increment iterator:
    if ((*it & 0x80) == 0) return static_cast<boost::uint32_t>(*it++);

    // Multi-byte sequences
    if ((*it & 0xE0) == 0xC0)
    {
        // Two-byte sequence.
        FIRST_BYTE(0x1F, 6);
        NEXT_BYTE(0);
        if (uc < 0x80) return utf8::invalid;    // overlong
        return uc;
    }
    else if ((*it & 0xF0) == 0xE0)
    {
        // Three-byte sequence.
        FIRST_BYTE(0x0F, 12);
        NEXT_BYTE(6);
        NEXT_BYTE(0);
        if (uc < 0x800) return utf8::invalid;    // overlong
        if (uc >= 0x0D800 && uc <= 0x0DFFF) return utf8::invalid;    // not valid ISO 10646
        if (uc == 0x0FFFE || uc == 0x0FFFF) return utf8::invalid;    // not valid ISO 10646
        return uc;
    }
    else if ((*it & 0xF8) == 0xF0)
    {
        // Four-byte sequence.
        FIRST_BYTE(0x07, 18);
        NEXT_BYTE(12);
        NEXT_BYTE(6);
        NEXT_BYTE(0);
        if (uc < 0x010000) return utf8::invalid;    // overlong
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
        if (uc < 0x0200000) return utf8::invalid;    // overlong
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
        if (uc < 0x04000000) return utf8::invalid;    // overlong
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


#define ENC_DEFAULT 0
#define ENC_UTF8 1
#define ENC_UTF16BE 2
#define ENC_UTF16LE 3

char*
utf8::stripBOM(char* in, size_t& size, TextEncoding& encoding)
{
    encoding = encUNSPECIFIED;
    if ( size > 2 )
    {
        // need *ptr to be unsigned or cast all 0xNN
        unsigned char* ptr = reinterpret_cast<unsigned char*>(in);

        if ( *ptr == 0xFF && *(ptr+1) == 0xFE )
        {
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
utf8::textEncodingName(TextEncoding enc)
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


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
