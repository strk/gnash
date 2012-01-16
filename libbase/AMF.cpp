// AMFConverter.h   High-level functions for converting as_values to AMF.
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

// This file provides high-level function objects for converting ActionScript
// values to AMF buffers and vice-versa.

#include <map>
#include <algorithm>

#include "log.h"
#include "SimpleBuffer.h"
#include "AMF.h"

// Define this macro to make AMF parsing verbose
//#define GNASH_DEBUG_AMF_DESERIALIZE 1

// Define this macto to make AMF writing verbose
// #define GNASH_DEBUG_AMF_SERIALIZE 1

namespace gnash {
namespace amf {

namespace {
    /// Swap bytes in raw data.
    //
    ///	This only swaps bytes if the host byte order is little endian.
    ///
    /// @param word The address of the data to byte swap.
    /// @param size The number of bytes in the data.
    void swapBytes(void* word, size_t size);
}

bool
readBoolean(const boost::uint8_t*& pos, const boost::uint8_t* _end)
{
    if (pos == _end) {
        throw AMFException("Read past _end of buffer for boolean type");
    }

    const bool val = *pos;
    ++pos;
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
    log_debug("amf0 read bool: %d", val);
#endif
    return val;
}

double
readNumber(const boost::uint8_t*& pos, const boost::uint8_t* end)
{

    if (end - pos < 8) {
        throw AMFException("Read past _end of buffer for number type");
    }

    double d;
    // TODO: may we avoid a copy and swapBytes call
    //       by bitshifting b[0] trough b[7] ?
    std::copy(pos, pos + 8, reinterpret_cast<char*>(&d));
    pos += 8; 
    swapBytes(&d, 8);

#ifdef GNASH_DEBUG_AMF_DESERIALIZE
    log_debug("amf0 read double: %e", dub);
#endif

    return d;
}

std::string
readString(const boost::uint8_t*& pos, const boost::uint8_t* end)
{
    if (end - pos < 2) {
        throw AMFException(_("Read past _end of buffer for string length"));
    }

    const boost::uint16_t si = readNetworkShort(pos);
    pos += 2;

    if (end - pos < si) {
        throw AMFException(_("Read past _end of buffer for string type"));
    }

    const std::string str(reinterpret_cast<const char*>(pos), si);
    pos += si;
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
    log_debug("amf0 read string: %s", str);
#endif
    return str;
}

std::string
readLongString(const boost::uint8_t*& pos, const boost::uint8_t* end)
{
    if (end - pos < 4) {
        throw AMFException("Read past _end of buffer for long string length");
    }

    const boost::uint32_t si = readNetworkLong(pos);
    pos += 4;
    if (static_cast<boost::uint32_t>(end - pos) < si) {
        throw AMFException("Read past _end of buffer for long string type");
    }

    const std::string str(reinterpret_cast<const char*>(pos), si);
    pos += si;

#ifdef GNASH_DEBUG_AMF_DESERIALIZE
    log_debug("amf0 read long string: %s", str);
#endif

    return str;

}

void
writePlainString(SimpleBuffer& buf, const std::string& str, Type t)
{
    const size_t len = str.size();
    switch (t) {
        default:
            log_error(_("writePlainString called with invalid type!"));
            return;
       
        case LONG_STRING_AMF0:
            buf.appendNetworkLong(len);
            break;
           
        case STRING_AMF0:
            buf.appendNetworkShort(len);
            break;

    }
    buf.append(str.c_str(), len);
}

void
writePlainNumber(SimpleBuffer& buf, double d)
{
    swapBytes(&d, 8);
    buf.append(&d, 8);
}

void
write(SimpleBuffer& buf, const std::string& str)
{
    Type t = str.size() < 65536 ? STRING_AMF0 : LONG_STRING_AMF0;
    buf.appendByte(t);
    writePlainString(buf, str, t);
}

void
write(SimpleBuffer& buf, double d)
{
    buf.appendByte(NUMBER_AMF0);
    writePlainNumber(buf, d);
}

void
write(SimpleBuffer& buf, bool b)
{
    buf.appendByte(BOOLEAN_AMF0);
    buf.appendByte(b ? 1 : 0);
}

namespace {

void
swapBytes(void* word, size_t size)
{
    union {
        boost::uint16_t s;
        struct {
             boost::uint8_t c0;
             boost::uint8_t c1;
        } c;
    } u;
       
    u.s = 1;
    if (u.c.c0 == 0) {
        // Big-endian machine: do nothing
        return;
    }

    // Little-endian machine: byte-swap the word
    // A conveniently-typed pointer to the source data
    boost::uint8_t *x = static_cast<boost::uint8_t *>(word);

    // Handles odd as well as even counts of bytes
    std::reverse(x, x + size);
}

} // unnamed namespace

} // namespace amf
} // namespace gnash
