// action_buffer.cpp:  holds actions for later execution, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "action_buffer.h"
#include "log.h"
#include "stream.h"
#include "swf.h"
#include "ASHandlers.h"
#include "as_environment.h"
#include "movie_definition.h"

#include <typeinfo> 

#include <string>

using std::string;
using std::endl;

namespace {
//gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
}

namespace gnash {

// Forward declarations
static float convert_float_little(const void *p);
static double convert_double_wacky(const void *p);

action_buffer::action_buffer(const movie_definition& md)
    :
    m_decl_dict_processed_at(-1),
    _src(md)
{
//	static int count=0;
//	printf("Action buffer %d created\n", ++count);
}

void
action_buffer::read(SWFStream& in, unsigned long endPos)
{
	unsigned long startPos = in.get_position();
	assert(endPos <= in.get_tag_end_position());
	unsigned size = endPos-startPos;

	if ( ! size )
	{
		IF_VERBOSE_MALFORMED_SWF(
		log_swferror(_("Empty action buffer starting at offset %lu"), startPos);
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
    if ( m_buffer.back() != SWF::ACTION_END )
    {
        // Add a null terminator so read_string won't read off
        // the end of the buffer.
        m_buffer.push_back(0x00);

	    IF_VERBOSE_MALFORMED_SWF(
		    log_swferror(_("Action buffer starting at offset %lu doesn't end with an END tag"),
			    startPos);
        );
    }
    
}

/*public*/
void
action_buffer::process_decl_dict(size_t start_pc, size_t stop_pc) const
{
    assert(stop_pc <= m_buffer.size());
    
    if (static_cast<size_t>(m_decl_dict_processed_at) == start_pc) {
	// We've already processed this decl_dict. 
#ifndef NDEBUG
	int count = read_int16(start_pc+3);
	assert((int) m_dictionary.size() == count);
#endif
	return;
    }
    
#if 0 // debugging
    if (m_decl_dict_processed_at != -1)	{
	log_debug(_("process_decl_dict(" SIZET_FMT ", " SIZET_FMT "): decl_dict was already processed at %d. "
		"Overriding."),
		  start_pc, stop_pc, m_decl_dict_processed_at);
	//return;
    }
#endif
    
    m_decl_dict_processed_at = start_pc;
    
    // Actual processing.
    size_t i = start_pc;
    boost::uint16_t length = boost::uint16_t(read_int16(i+1));
    boost::uint16_t count = boost::uint16_t(read_int16(i+3)); 
    i += 2;
    
//log_debug(_("Start at %d, stop at %d, length read was %d, count read was %d"), start_pc, stop_pc, length, count);

    assert(start_pc + 3 + length == stop_pc);
    
    m_dictionary.resize(count);
    
    // Index the strings.
    for (int ct = 0; ct < count; ct++) {
	// Point into the current action buffer.
	m_dictionary[ct] = (const char*) &m_buffer[3 + i];
	
	while (m_buffer[3 + i]) {
	    // safety check.
	    if (i >= stop_pc) {
		log_error(_("action buffer dict length exceeded"));
		
		// Jam something into the remaining (invalid) entries.
		while (ct < count) {
		    m_dictionary[ct] = "<invalid>";
		    ct++;
		}
		return;
	    }
	    i++;
	}
	i++;
    }
}

#if 0
// Interpret the actions in this action buffer, and evaluate
// them in the given environment.  Execute our whole buffer,
// without any arguments passed in.
void
action_buffer::execute(as_environment* env) const
{
	assert(env);

	int local_stack_top = env->get_local_frame_top();
	env->add_frame_barrier();

	ActionExec exec(*this, *env);
	exec();
    
	env->set_local_frame_top(local_stack_top);
}

// Interpret the specified subset of the actions in our
// buffer.  Caller is responsible for cleaning up our local
// stack frame (it may have passed its arguments in via the
// local stack frame).
// 
// The is_function2 flag determines whether to use global or local registers.
void
action_buffer::execute(
    as_environment* env,
    size_t start_pc,
    size_t exec_bytes, // used when invoked as a function call
    as_value* retval, // used when invoked as a function call
    const std::vector<with_stack_entry>& initial_with_stack,
    bool is_function2) const
{
	assert(env);
	ActionExec exec(*this, *env, start_pc, exec_bytes, retval,
		initial_with_stack, is_function2);
	exec();
}
#endif

// Disassemble one instruction to the log.
static std::string
disasm_instruction(const unsigned char* instruction_data)
{

    using namespace gnash::SWF;

    const gnash::SWF::SWFHandlers& ash = gnash::SWF::SWFHandlers::instance();

    as_arg_t fmt = ARG_HEX;
    action_type	action_id = (action_type)instruction_data[0];
    unsigned char num[10];
    memset(num, 0, 10);

    std::stringstream ss;

    // Show instruction.
    if (action_id > ash.lastType()) {
	ss << "<unknown>[0x]" <<  action_id << endl;
    } else {
	ss << ash[action_id].getName();
    }

    // Show instruction argument(s).
    if (action_id & 0x80)
    {
	ss << " (";
	fmt = ash[action_id].getArgFormat();
	assert(fmt != ARG_NONE);
	int length = instruction_data[1] | (instruction_data[2] << 8);
	if (fmt == ARG_HEX)
	{
	    for (int i = 0; i < length; i++) {

		ss << "0x" << hexify((const boost::uint8_t *)&instruction_data[3 + i], 1, false) << " ";
	    }
	}
	else if (fmt == ARG_STR)
	{
	    string str;
	    for (int i = 0; i < length; i++) {
		str += instruction_data[3 + i];
	    }
	    ss << "\"" << str.c_str() << "\"";
	}
	else if (fmt == ARG_U8)
	{
	    int	val = instruction_data[3];
	    ss << " " << val;
	}
	else if (fmt == ARG_U16)
	{
	    int	val = instruction_data[3] | (instruction_data[4] << 8);
	    ss << " " << val;
	}
	else if (fmt == ARG_S16)
	{
	    int	val = instruction_data[3] | (instruction_data[4] << 8);
	    if (val & 0x8000) val |= ~0x7FFF;	// sign-extend
	    ss << " " << val;
	}
	else if (fmt == ARG_PUSH_DATA)
	{
	    int i = 0;
	    while (i < length)
	    {
		int type = instruction_data[3 + i];
		if ( i++ ) ss << ", ";

		if (type == 0)
		{
		    // string
		    string str;
		    while (instruction_data[3 + i])
		    {
			str += instruction_data[3 + i];
			i++;
		    }
		    i++;
		    ss << "\"" << str.c_str() << "\"";
		}
		else if (type == 1)
		{
		    // float (little-endian)
		    float f = convert_float_little(instruction_data + 3 + i);
		    i += 4;
		    ss << "(float) " << f;
		}
		else if (type == 2)
		{
		    ss << "NULL";
		}
		else if (type == 3)
		{
		    ss << "undef";
		}
		else if (type == 4)
		{
		    // contents of register
		    int	reg = instruction_data[3 + i];
		    i++;
		    ss << "reg[" << reg << "]";
		}
		else if (type == 5)
		{
		    int	bool_val = instruction_data[3 + i];
		    i++;
		    ss << "bool(" << bool_val << ")";
		}
		else if (type == 6)
		{
		    // double in wacky format: 45670123
		    double d = convert_double_wacky(instruction_data + 3 + i);
		    i += 8;
		    ss << "(double) " << d;
		}
		else if (type == 7)
		{
		    // boost::int32_t
		    boost::int32_t	val = instruction_data[3 + i]
			| (instruction_data[3 + i + 1] << 8)
			| (instruction_data[3 + i + 2] << 16)
			| (instruction_data[3 + i + 3] << 24);
		    i += 4;
		    ss << "(int) " << val;
		}
		else if (type == 8)
		{
		    int	id = instruction_data[3 + i];
		    i++;
		    ss << "dict_lookup[" << id << "]";
		}
		else if (type == 9)
		{
		    int	id = instruction_data[3 + i] | (instruction_data[3 + i + 1] << 8);
		    i += 2;
		    ss << "dict_lookup_lg[" << id << "]";
		}
	    }
	}
	else if (fmt == ARG_DECL_DICT)
	{
	    int	i = 0;
	    int	count = instruction_data[3 + i] | (instruction_data[3 + i + 1] << 8);
	    i += 2;
	    
	    ss << " [" << count << "] ";
	    
	    // Print strings.
	    for (int ct = 0; ct < count; ct++)
	    {
		if ( ct ) ss << ", ";

		ss << ct << ":"; 
		
		string str;
		while (instruction_data[3 + i])
		{
			// safety check.
		    if (i >= length)
		    {
			log_debug("<disasm error -- length exceeded>");
			break;
		    }
		    str += instruction_data[3 + i];
		    i++;
		}
		ss << "\"" << str.c_str() << "\"";
		i++;
	    }
	}
	else if (fmt == ARG_FUNCTION2)
	{
	    // Signature info for a function2 opcode.
	    int	i = 0;
	    const char*	function_name = (const char*) &instruction_data[3 + i];
	    i += strlen(function_name) + 1;
	    
	    int	arg_count = instruction_data[3 + i] | (instruction_data[3 + i + 1] << 8);
	    i += 2;
	    
	    int	reg_count = instruction_data[3 + i];
	    i++;

	    ss << "\tname = '" << function_name << "'"
		       << " arg_count = " << arg_count
		       << " reg_count = " << reg_count;
	    
	    boost::uint16_t	flags = (instruction_data[3 + i]) | (instruction_data[3 + i + 1] << 8);
	    i += 2;
	    
	    // @@ What is the difference between "super" and "_parent"?
	    
	    bool	preload_global = (flags & 0x100) != 0;
	    bool	preload_parent = (flags & 0x80) != 0;
	    bool	preload_root   = (flags & 0x40) != 0;
	    bool	suppress_super = (flags & 0x20) != 0;
	    bool	preload_super  = (flags & 0x10) != 0;
	    bool	suppress_args  = (flags & 0x08) != 0;
	    bool	preload_args   = (flags & 0x04) != 0;
	    bool	suppress_this  = (flags & 0x02) != 0;
	    bool	preload_this   = (flags & 0x01) != 0;
	    
	    ss << " pg=" << preload_global
		<< " pp=" << preload_parent
		<< " pr=" << preload_root
		<< " ss=" << suppress_super
		<< " ps=" << preload_super
		<< " sa=" << suppress_args
		<< " pa=" << preload_args
		<< " st=" << suppress_this
		<< " pt=" << preload_this;

	    for (int argi = 0; argi < arg_count; argi++)
	    {
		int	arg_register = instruction_data[3 + i];
		i++;
		const char*	arg_name = (const char*) &instruction_data[3 + i];
		i += strlen(arg_name) + 1;
		
		ss << "\targ[" << argi << "]"
			   << " - reg[" << arg_register << "]"
			   << " - '" << arg_name << "'";
	    }
	    
	    int	function_length = instruction_data[3 + i] | (instruction_data[3 + i + 1] << 8);
	    i += 2;
	    
	    ss << "\t\tfunction length = " << function_length;
	}
	ss << ")";
    }

    return ss.str();
}

std::string
action_buffer::disasm(size_t pc) const
{
	const unsigned char* instruction_data =
		(const unsigned char *)&m_buffer[pc];
	return disasm_instruction(instruction_data);
}

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
static float
convert_float_little(const void *p)
{
	// Hairy union for endian detection and munging
	union {
		float	f;
		boost::uint32_t i;
		struct {	// for endian detection
			boost::uint16_t s0;
			boost::uint16_t s1;
		} s;
		struct {	// for byte-swapping
			boost::uint8_t c0;
			boost::uint8_t c1;
			boost::uint8_t c2;
			boost::uint8_t c3;
		} c;
	} u;

	u.f = 1.0;
	switch (u.s.s0) {
	case 0x0000:	// little-endian host
		memcpy(&u.i, p, 4);
		break;
	case 0x3f80:	// big-endian host
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

// Read a 64-bit double from memory, stored in word-swapped little-endian
// format and return it as a host-endian double.
// "Wacky format" is 45670123.
static double
convert_double_wacky(const void *p)
{
	const boost::uint8_t *cp = (const boost::uint8_t *)p;	// Handy uchar version
	union {
		double	d;
		boost::uint64_t	i;
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

	compiler_assert(sizeof(u) == sizeof(u.i));

	// Detect endianness of doubles by storing a value that is
	// exactly representable and that has different values in the
	// four 16-bit words.
	// 0x11223344 is represented as 0x41b1 2233 4400 0000 (bigendian)
	u.d = (double) 0x11223344;
	switch (u.s.s0) {
	case 0x0000:	// pure little-endian host: swap words only.
		memcpy(&u.l.l1, cp, 4);
		memcpy(&u.l.l0, cp + 4, 4);
		break;
	case 0x41b1:	// pure big-endian host: swap contents of 32-bit words
		u.c.c0 = cp[3];
		u.c.c1 = cp[2];
		u.c.c2 = cp[1];
		u.c.c3 = cp[0];
		u.c.c4 = cp[7];
		u.c.c5 = cp[6];
		u.c.c6 = cp[5];
		u.c.c7 = cp[4];
		break;
	case 0x2233:	// word-swapped little-endian host (PDP / ARM FPA)
			// is the same as wacky format.
		memcpy(&u.i, cp, 8);
		break;
	case 0x4400:	// word-swapped big-endian host: does this exist?
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

float
action_buffer::read_float_little(size_t pc) const
{
	return(convert_float_little(&m_buffer[pc]));
}

double
action_buffer::read_double_wacky(size_t pc) const
{
	return(convert_double_wacky(&m_buffer[pc]));
}

const std::string&
action_buffer::getDefinitionURL() const
{
	return _src.get_url();
}

}

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
