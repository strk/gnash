// AMF.h    Low level functions for manipulating and reading AMF buffers.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

// This file provides low-level manipulators for AMF buffers. It can be used
// without reliance on libcore.

#ifndef GNASH_AMF_H
#define GNASH_AMF_H

#include <string>
#include <cstdint>

#include "dsodefs.h"
#include "GnashException.h"

namespace gnash {
    class SimpleBuffer;
}

namespace gnash {

/// Functions and classes for handling AMF.
//
/// AMF is a simple serialization format for ActionScript objects and values,
/// allowing them to be stored and transmitted. The AMF namespace provides
/// both low-level and high-level conversion to and from AMF buffers.
namespace amf {

enum Type {
    NOTYPE            = -1,
    NUMBER_AMF0       = 0x00,
    BOOLEAN_AMF0      = 0x01,
    STRING_AMF0       = 0x02,
    OBJECT_AMF0       = 0x03,
    MOVIECLIP_AMF0    = 0x04,
    NULL_AMF0         = 0x05,
    UNDEFINED_AMF0    = 0x06,
    REFERENCE_AMF0    = 0x07,
    ECMA_ARRAY_AMF0   = 0x08,
    OBJECT_END_AMF0   = 0x09,
    STRICT_ARRAY_AMF0 = 0x0a,
    DATE_AMF0         = 0x0b,
    LONG_STRING_AMF0  = 0x0c,
    UNSUPPORTED_AMF0  = 0x0d,
    RECORD_SET_AMF0   = 0x0e,
    XML_OBJECT_AMF0   = 0x0f,
    TYPED_OBJECT_AMF0 = 0x10
};

/// Exception for handling malformed buffers.
//
/// All low-level reading operations can throw this error.
class DSOEXPORT
AMFException : public GnashException
{
public:
    AMFException(const std::string& msg)
        :
        GnashException(msg)
    {}
};

/// Read a number from an AMF buffer
//
/// This does not read a type byte; use AMF::Reader when the type should
/// be determined from the buffer.
//
/// This function will throw an AMFException if it encounters ill-formed AMF.
DSOEXPORT double readNumber(const std::uint8_t*& pos,
        const std::uint8_t* end);

/// Read a boolean value from the buffer.
//
/// This does not read a type byte; use AMF::Reader when the type should
/// be determined from the buffer.
//
/// This function will throw an AMFException if it encounters ill-formed AMF.
DSOEXPORT bool readBoolean(const std::uint8_t*& pos,
        const std::uint8_t* end);

/// Read a string value from the buffer.
//
/// This does not read a type byte; use AMF::Reader when the type should
/// be determined from the buffer.
//
/// This function will throw an AMFException if it encounters ill-formed AMF.
DSOEXPORT std::string readString(const std::uint8_t*& pos,
        const std::uint8_t* end);

/// Read a long string value from the buffer.
//
/// This does not read a type byte; use AMF::Reader when the type should
/// be determined from the buffer.
//
/// This function will throw an AMFException if it encounters ill-formed AMF.
DSOEXPORT std::string readLongString(const std::uint8_t*& pos,
        const std::uint8_t* end);

/// Read an unsigned 16-bit value in network byte order.
//
/// You must ensure that the buffer contains at least 2 bytes!
inline std::uint16_t
readNetworkShort(const std::uint8_t* buf)
{
    const std::uint16_t s = buf[0] << 8 | buf[1];
    return s;
}

/// Read an unsigned 32-bit value in network byte order.
//
/// You must ensure that the buffer contains at least 4 bytes!
inline std::uint32_t
readNetworkLong(const std::uint8_t* buf)
{
    const std::uint32_t s = buf[0] << 24 | buf[1] << 16 |
                              buf[2] << 8 | buf[3];
    return s;
}

/// Write a string to an AMF buffer.
//
/// This function writes the type byte and the string value. It also handles
/// both long and short strings automatically.
//
/// This is overloaded for automatic type deduction to allow the use of
/// a template for more complex operations. You must be careful when using
/// it!
DSOEXPORT void write(SimpleBuffer& buf, const std::string& str);

/// Write a C string to an AMF buffer.
//
/// The overload is necessary to prevent const char* being resolved to the
/// boolean overload.
inline void write(SimpleBuffer& buf, const char* str) {
    return write(buf, std::string(str));
}

/// Write a number to an AMF buffer.
//
/// This function writes the type byte and the double value.
//
/// This is overloaded for automatic type deduction to allow the use of
/// a template for more complex operations. You must be careful when using
/// it!
DSOEXPORT void write(SimpleBuffer& buf, double d);

/// Write a boolean value to an AMF buffer.
//
/// This function writes the type byte and the boolean value.
//
/// This is overloaded for automatic type deduction to allow the use of
/// a template for more complex operations. You must be careful when using
/// it!
DSOEXPORT void write(SimpleBuffer& buf, bool b);

/// Encode a plain short string to an AMF buffer.
//
/// This does not encode a type byte; it is used for cases where a string is
/// required, such as for the name of an object property, and therefore does
/// not use a type byte.
DSOEXPORT void writePlainString(SimpleBuffer& buf, const std::string& str,
        Type t);

/// Write a number to an AMF buffer.
//
/// This function writes the double value without a type byte.
DSOEXPORT void writePlainNumber(SimpleBuffer& buf, double d);

/// Encode a string-value pair.
//
/// This is used for object properties; the string is always encoded with
/// a 2-byte length.
template<typename T>
void
writeProperty(SimpleBuffer& buf, const std::string& name, const T& t)
{
    writePlainString(buf, name, STRING_AMF0);
    write(buf, t);
}

} // namespace amf
} // namespace gnash

#endif
