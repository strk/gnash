// stream.cpp - SWF stream reading class, for Gnash
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


#include "SWFStream.h"

#include "log.h"
#include "IOChannel.h"
#include "SWF.h"
#include "Property.h"
#include "action_buffer.h"

#include <cstring>
#include <climits>
#include <boost/static_assert.hpp>

//#define USE_TU_FILE_BYTESWAPPING 1

namespace gnash {
    
SWFStream::SWFStream(IOChannel* input)
    :
    m_input(input),
    m_current_byte(0),
    m_unused_bits(0)
{
}


SWFStream::~SWFStream()
{
}

void
SWFStream::ensureBytes(unsigned long needed)
{
#ifndef GNASH_TRUST_SWF_INPUT

    // Not in a tag (should we check file length?)
    if ( _tagBoundsStack.empty() ) return; 

    unsigned long int left = get_tag_end_position() - tell();
    if ( left < needed )
    {
        std::stringstream ss;
        ss << "premature end of tag: need to read " << needed << 
            " bytes, but only " << left << " left in this tag";
        throw ParserException(ss.str());
    }
#endif
}

unsigned SWFStream::read(char *buf, unsigned count)
{
    align();

    // If we're in a tag, make sure we're not seeking outside the tag.
    if ( ! _tagBoundsStack.empty() )
    {
        TagBoundaries& tb = _tagBoundsStack.back();
        unsigned long endPos = tb.second;
        unsigned long cur_pos = tell();
        assert(endPos >= cur_pos);
        unsigned long left = endPos - cur_pos;
        if ( left < count ) count = left;
    }

    if ( ! count ) return 0;

    return m_input->read(buf, count);
}

bool SWFStream::read_bit()
{
    if (!m_unused_bits)
    {
        m_current_byte = m_input->read_byte(); // don't want to align here
        m_unused_bits = 7;
        return (m_current_byte&0x80);
    }
    else
    {
        return ( m_current_byte & (1<<(--m_unused_bits)) );
    }
}

unsigned SWFStream::read_uint(unsigned short bitcount)
{
    // htf_sweet.swf fails when this is set to 24. There seems to
    // be no reason why this should be limited to 32 other than
    // that it is higher than a movie is likely to need.
    if (bitcount > 32)
    {
        // This might overflow a uint32_t or attempt to read outside
        // the byte cache (relies on there being only 4 bytes after
        // possible unused bits.)
        throw ParserException("Unexpectedly long value advertised.");
    }

    // Optimization for multibyte read
    if ( bitcount > m_unused_bits )
    {
        typedef unsigned char byte;

        boost::uint32_t value = 0;

        if (m_unused_bits) // Consume all the unused bits.
        {
            int unusedMask = (1 << m_unused_bits)-1;
            bitcount -= m_unused_bits; 
            value |= ((m_current_byte&unusedMask) << bitcount);
        }

        int bytesToRead = bitcount/8;
        int spareBits = bitcount%8; // additional bits to read

        //std::cerr << "BytesToRead: " << bytesToRead << " spareBits: " << spareBits << " unusedBits: " << (int)m_unused_bits << std::endl;

        assert (bytesToRead <= 4);
        byte cache[4]; // at most 4 bytes in the cache

        if ( spareBits ) m_input->read(&cache, bytesToRead+1);
        else m_input->read(&cache, bytesToRead);

        for (int i=0; i<bytesToRead; ++i)
        {
            bitcount -= 8;
            value |= cache[i] << bitcount; 
        }

        //assert(bitcount == spareBits);
        if ( bitcount )
        {
            m_current_byte = cache[bytesToRead];
            m_unused_bits = 8-bitcount;
            value |= m_current_byte >> m_unused_bits;
        }
        else
        {
            m_unused_bits = 0;
        }

        return value;
        
    }

    if (!m_unused_bits)
    {
        m_current_byte = m_input->read_byte();
        m_unused_bits = 8;
    }

    // TODO: optimize unusedMask creation ?
    //       (it's 0xFF if ! m_unused_bits above)
    int unusedMask = (1 << m_unused_bits)-1;

    if (bitcount == m_unused_bits)
    {
        // Consume all the unused bits.
        m_unused_bits = 0;
        return (m_current_byte&unusedMask);
    }
    else
    {
        assert(bitcount < m_unused_bits);
        // Consume some of the unused bits.

        m_unused_bits -= bitcount;
        return ((m_current_byte&unusedMask) >> m_unused_bits);
    }

}


int
SWFStream::read_sint(unsigned short bitcount)
{
    //assert(bitcount <= 32); // already asserted in read_uint

    boost::int32_t value = boost::int32_t(read_uint(bitcount));

    // Sign extend...
    if (value & (1 << (bitcount - 1))) {
        value |= -1 << bitcount;
    }

//        IF_DEBUG(log_debug("SWFStream::read_sint(%d) == %d\n", bitcount, value)));

    return value;
}


float    SWFStream::read_fixed()
{
    // align(); // read_u32 will align 
    return static_cast<float> (
        static_cast<double>(read_s32()) / 65536.0f
    );

}

// float is not large enough to hold a 32 bit value without doing the wrong thing with the sign.
// So we upgrade to double for the calculation and then resize when we know it's small enough.
float    SWFStream::read_ufixed()
{
    // align(); // read_u32 will align 
    return static_cast<float> (
        static_cast<double>(read_u32()) / 65536.0f
    );
}

// Read a short fixed value, unsigned.
float   SWFStream::read_short_ufixed()
{
    // align(); // read_u16 will align 
    return read_u16() / 256.0f;
}

// Read a short fixed value, signed.
float    SWFStream::read_short_sfixed()
{
    // align(); // read_s16 will align 
    return read_s16() / 256.0f;
}

/// Read a 16bit (1:sign 5:exp 10:mantissa) floating point value
float    SWFStream::read_short_float()
{
    // read_s16 will align
    return static_cast<float> ( read_s16() );
}

// Read a little-endian 32-bit float from p
// and return it as a host-endian float.
static float
convert_float_little(const void *p)
{
    // Hairy union for endian detection and munging
    union {
        float    f;
        boost::uint32_t i;
        struct {    // for endian detection
            boost::uint16_t s0;
            boost::uint16_t s1;
        } s;
        struct {    // for byte-swapping
            boost::uint8_t c0;
            boost::uint8_t c1;
            boost::uint8_t c2;
            boost::uint8_t c3;
        } c;
    } u;

    u.f = 1.0;
    switch (u.s.s0) {
    case 0x0000:    // little-endian host
        memcpy(&u.i, p, 4);
        break;
    case 0x3f80:    // big-endian host
        {
        const boost::uint8_t *cp = (const boost::uint8_t *) p;
        u.c.c0 = cp[3];
        u.c.c1 = cp[2];
        u.c.c2 = cp[1];
        u.c.c3 = cp[0];
        }
        break;
    default:
        log_error(_("Native floating point format not recognised"));
        abort();
    }
    
    return u.f;
}

/// Read a 32bit (1:sign 8:exp 23:mantissa) floating point value
float    SWFStream::read_long_float()
{
    const unsigned short dataLength = 4;

    char data[dataLength];
    
    // Should align
    if (read(data, dataLength) < dataLength)
    {
        throw ParserException(_("Unexpected end of stream while reading"));
    }
    return convert_float_little(data); 
}

// Read a 64-bit double value
double SWFStream::read_d64()
{

    // TODO: This is very dodgy and doesn't account for endianness!
    const unsigned short dataLength = 8;
    double d = 0;

    BOOST_STATIC_ASSERT(sizeof(double) == dataLength);

    // Should align:
    if (read(reinterpret_cast<char*>(&d), dataLength) < dataLength)
    {
        throw ParserException(_("Unexpected end of stream while reading"));
    }
 
    return d;

}

boost::uint8_t    SWFStream::read_u8()
{
    align();
    return m_input->read_byte();
}

boost::int8_t
SWFStream::read_s8()
{
    // read_u8 will align
    return read_u8();
}

boost::uint16_t SWFStream::read_u16()
{
#ifdef USE_TU_FILE_BYTESWAPPING 
    align();
    return m_input->read_le16();
#else
    const unsigned short dataLength = 2;

    unsigned char buf[dataLength];

    // Should align:
    if (read(reinterpret_cast<char*>(buf), dataLength) < dataLength)
    {
        throw ParserException(_("Unexpected end of stream while reading"));
    }
    
    boost::uint32_t result = buf[0];
    result |= (buf[1] << 8);

    return result;
#endif
}

boost::int16_t SWFStream::read_s16()
{
    // read_u16 will align
    return read_u16();
}

boost::uint32_t    SWFStream::read_u32()
{
#ifdef USE_TU_FILE_BYTESWAPPING 
    align();
    return m_input->read_le32();
#else
    using boost::uint32_t;

    const unsigned short dataLength = 4;

    unsigned char buf[dataLength];
    
    // Should align
    if (read(reinterpret_cast<char*>(buf), dataLength) < dataLength)
    {
        throw ParserException(_("Unexpected end of stream while reading"));
    }
    
    uint32_t result = buf[0];
         result |= buf[1] << 8;
         result |= buf[2] << 16;
         result |= buf[3] << 24;

    return result;
#endif
}

boost::int32_t    SWFStream::read_s32()
{
    // read_u32 will align
    return read_u32();
}

void
SWFStream::read_string(std::string& to)
{
    align();

    to.clear();

    do
    {
        ensureBytes(1);
        const char& c = read_u8();
        if ( c == 0 ) break; // don't store a NULL in the string.
        to.push_back(c);
    } while(1);

}

void SWFStream::read_string_with_length(std::string& to)
{
    align();

    ensureBytes(1);
    const unsigned int len = read_u8();
    read_string_with_length(len, to); // will check 'len'
}

void SWFStream::read_string_with_length(unsigned len, std::string& to)
{
    align();

    to.resize(len);

    ensureBytes(len);
    for (unsigned int i = 0; i < len; ++i)
    {
        to[i] = read_u8();
    }

    // drop trailing nulls (see swf6/Bejeweled.swf)
    std::string::size_type last = to.find_last_not_of('\0');
    if ( last == std::string::npos ) to.clear();
    else {
        ++last;
        if (last < len) {
            // seems common to find null-terminated lenght-equipped strings...
            to.erase(last);
        }
    }

}


unsigned long
SWFStream::tell()
{
    int pos = m_input->tell();
    // TODO: check return value? Could be negative.
    return static_cast<unsigned long>(pos);
}


bool
SWFStream::seek(unsigned long pos)
{
    align();

    // If we're in a tag, make sure we're not seeking outside the tag.
    if ( ! _tagBoundsStack.empty() )
    {
        TagBoundaries& tb = _tagBoundsStack.back();
        unsigned long endPos = tb.second;
        if ( pos > endPos )
        {
		log_error(_("Attempt to seek past the end of an opened tag"));
            // abort(); // ?
            // throw ParserException ?
            return false;
        }
        unsigned long startPos = tb.first;
        if ( pos < startPos )
        {
		log_error(_("Attempt to seek before start of an opened tag"));
            // abort(); // ?
            // throw ParserException ?
            return false;
        }
    }

    // Do the seek.
    if (!m_input->seek(pos))
    {
        // TODO: should we throw an exception ?
        //       we might be called from an exception handler
        //       so throwing here might be a double throw...
        log_swferror(_("Unexpected end of stream"));
        return false;
    }

    return true;
}


unsigned long
SWFStream::get_tag_end_position()
{
    assert(!_tagBoundsStack.empty());
    return _tagBoundsStack.back().second;
}


SWF::TagType
SWFStream::open_tag()
{
    align();

    unsigned long tagStart = tell();

    ensureBytes(2);

    int tagHeader = read_u16();
    int tagType = tagHeader >> 6;
    int tagLength = tagHeader & 0x3F;
    assert(m_unused_bits == 0);
        
    if (tagLength == 0x3F)
    {
        ensureBytes(4);
        tagLength = read_u32();
    }

    if (tagLength < 0)
    {
        throw ParserException("Negative tag length advertised.");
    }

    if ( tagLength > 1024*64 )
    {
        //log_debug("Tag %d has a size of %d bytes !!", tagType, tagLength);
    }

    unsigned long tagEnd = tell() + tagLength;
    
    // Check end position doesn't overflow a signed int - that makes
    // zlib adapter's inflate_seek(int pos, void* appdata) unhappy.
    // The cast stops compiler warnings. We know it's a positive number.
    // TODO: make IOChannel take a long instead of an int.
    // TODO: check against stream length.
    if (tagEnd > static_cast<unsigned int>(std::numeric_limits<signed int>::max()))
    {
        std::stringstream ss;
        ss << "Invalid tag end position " << tagEnd << " advertised (tag length "
            << tagLength << ").";
        throw ParserException(ss.str());
    }    

    if ( ! _tagBoundsStack.empty() )
    {
        // check that this tag doesn't cross containing tag bounds
        unsigned long containerTagEnd = _tagBoundsStack.back().second;
        if ( tagEnd > containerTagEnd )
        {
            unsigned long containerTagStart = _tagBoundsStack.back().first;
            log_swferror(_("Tag %d starting at offset %d is advertised to end "
                    "at offset %d, which is after end of previously opened "
                    "tag starting at offset %d and ending at offset %d. "
                    "Making it end where container tag ends."),
                    tagType, tagStart, tagEnd, containerTagStart, containerTagEnd);

            // what to do now ?
            tagEnd = containerTagEnd;
            //throw ParserException(ss.str());
        }
    }
        
    // Remember where the end of the tag is, so we can
    // fast-forward past it when we're done reading it.
    _tagBoundsStack.push_back(std::make_pair(tagStart, tagEnd));

    IF_VERBOSE_PARSE (
	    log_parse(_("SWF[%lu]: tag type = %d, tag length = %d, end tag = %lu"),
        tagStart, tagType, tagLength, tagEnd);
    );

    return static_cast<SWF::TagType>(tagType);
}


void
SWFStream::close_tag()
{
    assert(!_tagBoundsStack.empty());
    std::streampos endPos = _tagBoundsStack.back().second;
    _tagBoundsStack.pop_back();

    //log_debug("Close tag called at %d, stream size: %d", endPos);

    if (!m_input->seek(endPos))
    {
        // We'll go on reading right past the end of the stream
        // if we don't throw an exception.
        throw ParserException(_("Could not seek to reported end of tag"));
    }

    m_unused_bits = 0;
}

void
SWFStream::consumeInput()
{
	// IOChannel::go_to_end is documented
	// to possibly throw an exception (!)
	try {
		m_input->go_to_end();
	}
    catch (IOException& ex) {
	    log_error(_("SWFStream::consumeInput: underlying stream couldn't "
			"go_to_end: %s"), ex.what());
		// eh.. and now ?!
	}
}

} // end namespace gnash

    
// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
