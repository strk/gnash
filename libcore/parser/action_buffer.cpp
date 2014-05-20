// action_buffer.cpp:  holds actions for later execution, for Gnash.
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
//

#include "action_buffer.h"

#include <string>
#include <cstring> // for memcpy

#include "log.h"
#include "SWFStream.h"
#include "SWF.h"
#include "ASHandlers.h"
#include "movie_definition.h"

namespace gnash {

// Forward declarations
namespace {
    float convert_float_little(const void *p);
    double convert_double_wacky(const void *p);
}

action_buffer::action_buffer(const movie_definition& md)
    :
    _pools(),
    _src(md)
{
}

void
action_buffer::read(SWFStream& in, unsigned long endPos)
{
    unsigned long startPos = in.tell();
    assert(endPos <= in.get_tag_end_position());
    unsigned size = endPos-startPos;

    if (!size) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("Empty action buffer starting at offset %lu"),
                startPos);
        );
        return;
    }

    // Allocate the buffer
    // 
    // NOTE: a .reserve would be fine here, except GLIBCPP_DEBUG will complain...
    //
    m_buffer.resize(size);
    unsigned char* buf = &m_buffer.front();

    // Read all the bytes in the buffer
    //
    // NOTE:
    // we might be reading more data then we'll actually
    // use here if the SWF contains Action blocks padded
    // with data after the terminating END.
    // This has a cost in memory use, but for the normal
    // case (non-malformed SWF) not looking for an END
    // tag should give significant speedup in parsing
    // large action-based movies.
    //
    in.read(reinterpret_cast<char*>(buf), size);

    // Consistency checks here
    //
    // NOTE: it is common to find such movies, swfmill is known to write
    //       DoAction w/out the terminating END tag
    //
    if (m_buffer.back() != SWF::ACTION_END) {
        // Add a null terminator so read_string won't read off
        // the end of the buffer.
        m_buffer.push_back(0x00);

        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("Action buffer starting at offset %lu doesn't "
                    "end with an END tag"), startPos);
        );
    }
    
}

const ConstantPool&
action_buffer::readConstantPool(size_t start_pc, size_t stop_pc) const
{
    assert(stop_pc <= m_buffer.size()); // TODO: drop, be safe instead

    // Return a previously parsed pool at the same position, if any
    PoolsMap::iterator pi = _pools.find(start_pc);
    if ( pi != _pools.end() ) return pi->second;

    // Actual processing.

    ConstantPool& pool = _pools[start_pc];

    size_t i = start_pc;
    const boost::uint16_t length = read_uint16(i + 1);
    const boost::uint16_t count = read_uint16(i + 3); 
    i += 2;
    
    assert(start_pc + 3 + length == stop_pc);
    
    pool.resize(count);
    
    // Index the strings.
    for (int ct = 0; ct < count; ct++) {
        // Point into the current action buffer.
        pool[ct] = reinterpret_cast<const char*>(&m_buffer[3 + i]);

        // TODO: rework this "safety" thing here (doesn't look all that safe)
        while (m_buffer[3 + i]) {
            // safety check.
            if (i >= stop_pc) {
                log_error(_("action buffer dict length exceeded"));
                // Jam something into the remaining (invalid) entries.
                while (ct < count) {
                    pool[ct] = "<invalid>";
                    ct++;
                }
                return pool;
            }
            i++;
        }
        i++;
    }

    return pool;
}


// Disassemble one instruction to the log. The maxBufferLength
// argument is the number of bytes remaining in the action_buffer
// and prevents malformed instructions causing a read past the
// end of the buffer.
static std::string
disasm_instruction(const unsigned char* instruction_data,
        size_t maxBufferLength)
{

    using namespace SWF;

    const SWF::SWFHandlers& ash = SWF::SWFHandlers::instance();

    assert (maxBufferLength > 0);

    ArgumentType fmt = ARG_HEX;
    ActionType action_id = static_cast<ActionType>(instruction_data[0]);

    std::stringstream ss;

    // Show instruction.
    if (action_id > ash.lastType()) {
        ss << "<unknown>[0x]" <<  action_id << "\n";
    }
    else {
        ss << ash[action_id].getType();
    }
    
    // Show instruction argument(s).
    if (action_id & 0x80) {

        assert(maxBufferLength >= 3);
        ss << " (";
        fmt = ash[action_id].getArgFormat();
        
        size_t length = (instruction_data[1] | (instruction_data[2] << 8));
        
        // Assert that length without the three initial bytes
        // is always within the buffer.
        assert(length <= maxBufferLength - 3);

        switch (fmt) {

            case ARG_NONE:
                break;

            case ARG_HEX:
                ss << hexify(&instruction_data[3], length, false) << " ";
                break;

            case ARG_STR:
            {
                const std::string str =
                    hexify(&instruction_data[3], length, true);
                ss << "\"" << str.c_str() << "\"";
                break;
            }

            case ARG_U8:
            {
                const int val = instruction_data[3];
                ss << " " << val;
                break;
            }

            case ARG_U16:
            {
                const int val =
                    instruction_data[3] | (instruction_data[4] << 8);
                ss << " " << val;
                break;
            }

            case ARG_S16:
            {
                int val = instruction_data[3] | (instruction_data[4] << 8);
                if (val & 0x8000) val |= ~0x7FFF;    // sign-extend
                ss << " " << val;
                break;
            }

            case ARG_PUSH_DATA:
            {
                size_t i = 0;
                while (i < length) {
                    int type = instruction_data[3 + i];
                    
                    // This should be safe, as the buffer is always
                    // 0-terminated.
                    if (i++) ss << ", ";

                    switch (type)
                    {
                        case 0:
                        {
                            // string
                            std::string str;
                            while (instruction_data[3 + i] && i < length)
                            {
                                str += hexify(&instruction_data[3 + i], 1, true);
                                i++;
                            }
                            i++;
                            ss << "\"" << str.c_str() << "\"";
                            break;
                        }

                        case 1:
                        {
                            // float (little-endian)
                            if (i + 4 > length) break;
                            float f = convert_float_little(instruction_data + 3 + i);
                            i += 4;
                            ss << "(float) " << f;
                            break;
                        }

                        case 2:
                            ss << "NULL";
                            break;

                        case 3:
                            ss << "undef";
                            break;

                        case 4:
                        {
                            // contents of register
                            int reg = instruction_data[3 + i];
                            i++;
                            ss << "reg[" << reg << "]";
                            break;
                        }

                        case 5:
                        {
                            
                            int bool_val = instruction_data[3 + i];
                            i++;
                            ss << "bool(" << bool_val << ")";
                            break;
                        }

                        case 6:
                        {
                            // double in wacky format: 45670123
                            if (i + 8 > length) break;
                            double d = convert_double_wacky(instruction_data + 3 + i);
                            i += 8;
                            ss << "(double) " << d;
                            break;
                        }

                        case 7:
                        {
                            // boost::int32_t
                            boost::int32_t val = instruction_data[3 + i]
                            | (instruction_data[3 + i + 1] << 8)
                            | (instruction_data[3 + i + 2] << 16)
                            | (instruction_data[3 + i + 3] << 24);
                            i += 4;
                            ss << "(int) " << val;
                            break;
                        }

                        case 8:
                        {
                            int id = instruction_data[3 + i];
                            i++;
                            ss << "dict_lookup[" << id << "]";
                            break;
                        }

                        case 9:
                        {
                            int id = instruction_data[3 + i] | (instruction_data[3 + i + 1] << 8);
                            i += 2;
                            ss << "dict_lookup_lg[" << id << "]";
                            break;
                        }
                    }
                }
                break;
            }

            case ARG_DECL_DICT:
            {
                size_t i = 0;
                size_t count = instruction_data[3 + i] | (instruction_data[3 + i + 1] << 8);
                i += 2;
                
                ss << " [" << count << "] ";
                
                // Print strings.
                for (size_t ct = 0; ct < count; ct++)
                {
                    if ( ct ) ss << ", ";

                    ss << ct << ":"; 
                    
                    std::string str;
                    while (instruction_data[3 + i] && i < length)
                    {
                        str += instruction_data[3 + i];
                        i++;
                    }
                    ss << "\"" << str.c_str() << "\"";
                    i++;
                }
                break;
            }

            case ARG_FUNCTION2:
            {
                size_t i = 0;
                std::string functionName;
                // Signature info for a function2 opcode.
                while (instruction_data[3 + i] && i <= length)
                {
                    functionName.push_back(instruction_data[3 + i]);
                    ++i;
                }
 
                // Don't read outside the instruction.
                if (i + 6 > length) break;
                ++i;
                       
                boost::uint16_t argCount = instruction_data[3 + i] | (instruction_data[3 + i + 1] << 8);
                i += 2;
                
                boost::uint8_t registerCount = instruction_data[3 + i];
                i++;

                ss << "\tname = '" << functionName << "'"
                       << " arg count = " << argCount
                       << " register count = " << static_cast<int>(registerCount);
                
                const boost::uint16_t flags =
                    (instruction_data[3 + i]) |
                    (instruction_data[3 + i + 1] << 8);

                i += 2;
                
                const bool preload_global = (flags & 0x100);
                const bool preload_parent = (flags & 0x80);
                const bool preload_root   = (flags & 0x40);
                const bool suppress_super = (flags & 0x20);
                const bool preload_super  = (flags & 0x10);
                const bool suppress_args  = (flags & 0x08);
                const bool preload_args   = (flags & 0x04);
                const bool suppress_this  = (flags & 0x02);
                const bool preload_this   = (flags & 0x01);
                
                ss << " pg=" << preload_global
                << " pp=" << preload_parent
                << " pr=" << preload_root
                << " ss=" << suppress_super
                << " ps=" << preload_super
                << " sa=" << suppress_args
                << " pa=" << preload_args
                << " st=" << suppress_this
                << " pt=" << preload_this;

                for (size_t argi = 0; argi < argCount; ++argi) {

                    // Make sure not to read past the end of the
                    // instruction.
                    if (i >= length) break;
                    
                    int arg_register = instruction_data[3 + i];
                    i++;

                    std::string argName;
                    // Signature info for a function2 opcode.
                    while (instruction_data[3 + i] && i <= length) {
                        argName.push_back(instruction_data[3 + i]);
                        i++;
                    }
                    
                    ss << "\targ[" << argi << "]"
                       << " - reg[" << arg_register << "]"
                       << " - '" << argName << "'";

                    if (i == length) break;
                    
                    // Advance past the terminating 0
                    i++;

                }
                
                if (i + 2 > length) break;
                int function_length = instruction_data[3 + i] | (instruction_data[3 + i + 1] << 8);
                i += 2;
                
                ss << "\t\tfunction length = " << function_length;
                break;
            }            
        } // Switch
        
        ss << ")";    
    } // If action & 0x80
    
    return ss.str();
}

std::string
action_buffer::disasm(size_t pc) const
{
    const size_t maxBufferLength = m_buffer.size() - pc;
    return disasm_instruction(&m_buffer[pc], maxBufferLength);
}

float
action_buffer::read_float_little(size_t pc) const
{
    return convert_float_little(&m_buffer[pc]);
}

double
action_buffer::read_double_wacky(size_t pc) const
{
    return convert_double_wacky(&m_buffer[pc]);
}

const std::string&
action_buffer::getDefinitionURL() const
{
    return _src.get_url();
}

int
action_buffer::getDefinitionVersion() const
{
    return _src.get_version();
}

namespace {

// Endian conversion routines.
//
// Flash format stores integers as little-endian,
// floats as little-endian IEEE754,
// and doubles as little-endian IEEE754 with the two 32-bit words swapped over.
//
// We detect endianness at runtime.
// It looks hairy but the cost is small (one assignment, one switch),
// and it is less of a maintenance/portability nightmare.
// It also allows us to detect three existing variants instead of two and
// to reject incompatible (non-IEEE754) floating point formats (VAX etc).
// For these we would need to interpret the IEEE bitvalues explicitly.

// Read a little-endian 32-bit float from m_buffer[pc]
// and return it as a host-endian float.
float
convert_float_little(const void *p)
{
    // Hairy union for endian detection and munging
    union {
        float f;
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
            std::memcpy(&u.i, p, 4); // TODO: use std::copy instead ..
            break;
        case 0x3f80:    // big-endian host
        {
            const boost::uint8_t *cp = static_cast<const boost::uint8_t*>(p);
            u.c.c0 = cp[3];
            u.c.c1 = cp[2];
            u.c.c2 = cp[1];
            u.c.c3 = cp[0];
            break;
        }
        default:
            log_error(_("Native floating point format not recognised"));
            std::abort();
    }
    
    return u.f;
}


// Read a 64-bit double from memory, stored in word-swapped little-endian
// format and return it as a host-endian double.
// "Wacky format" is 45670123.
double
convert_double_wacky(const void *p)
{
    const boost::uint8_t *cp = static_cast<const boost::uint8_t*>(p);
    union {
        double d;
        boost::uint64_t i;
        struct {
            boost::uint32_t l0;
            boost::uint32_t l1;
        } l;
        struct {
            boost::uint16_t s0;
            boost::uint16_t s1;
            boost::uint16_t s2;
            boost::uint16_t s3;
        } s;
        struct {
            boost::uint8_t c0;
            boost::uint8_t c1;
            boost::uint8_t c2;
            boost::uint8_t c3;
            boost::uint8_t c4;
            boost::uint8_t c5;
            boost::uint8_t c6;
            boost::uint8_t c7;
        } c;
    } u;

    static_assert(sizeof(u) == sizeof(u.i), "u must be 8 bytes");

    // Detect endianness of doubles by storing a value that is
    // exactly representable and that has different values in the
    // four 16-bit words.
    // 0x11223344 is represented as 0x41b1 2233 4400 0000 (bigendian)
    u.d = static_cast<double>(0x11223344);
    switch (u.s.s0) {
    case 0x0000:    // pure little-endian host: swap words only.
        std::memcpy(&u.l.l1, cp, 4);
        std::memcpy(&u.l.l0, cp + 4, 4);
        break;
    case 0x41b1:    // pure big-endian host: swap contents of 32-bit words
        u.c.c0 = cp[3];
        u.c.c1 = cp[2];
        u.c.c2 = cp[1];
        u.c.c3 = cp[0];
        u.c.c4 = cp[7];
        u.c.c5 = cp[6];
        u.c.c6 = cp[5];
        u.c.c7 = cp[4];
        break;
    case 0x2233:    // word-swapped little-endian host (PDP / ARM FPA)
            // is the same as wacky format.
        std::memcpy(&u.i, cp, 8);
        break;
    case 0x4400:    // word-swapped big-endian host: does this exist?
        u.c.c0 = cp[7];
        u.c.c1 = cp[6];
        u.c.c2 = cp[5];
        u.c.c3 = cp[4];
        u.c.c4 = cp[3];
        u.c.c5 = cp[2];
        u.c.c6 = cp[1];
        u.c.c7 = cp[0];
        break;
    default:
        log_error(_("Native double floating point format not recognised"));
        abort();
    }

    return u.d;
}

} // unnamed namespace
} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
