// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/* $Id: action_buffer.cpp,v 1.16 2007/04/13 07:35:56 bjacques Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "action_buffer.h"
#include "log.h"
#include "stream.h"
#include "swf.h"
#include "ASHandlers.h"
#include "as_environment.h"

#include <typeinfo> 

#include <string>

using std::string;
using std::endl;

namespace {
gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
const gnash::SWF::SWFHandlers& ash = gnash::SWF::SWFHandlers::instance();
}

namespace gnash {

action_buffer::action_buffer()
    :
    m_decl_dict_processed_at(-1)
{
//	static int count=0;
//	printf("Action buffer %d created\n", ++count);
}


void
action_buffer::read(stream* in)
{
    // Read action bytes.
    for (;;) {
#if 0
	size_t instruction_start = m_buffer.size();
	size_t pc = m_buffer.size();
#endif

	uint8_t action_id = in->read_u8();
	m_buffer.push_back(action_id);
	
	if (action_id & 0x80) {
	    // Action contains extra data.  Read it.
	    uint16_t length = in->read_u16();
	    m_buffer.push_back(length & 0x0FF);
	    m_buffer.push_back((length >> 8) & 0x0FF);
	    for (uint16_t i = 0; i < length; i++) {
		uint8_t b = in->read_u8();
		m_buffer.push_back(b);
	    }
	}
	
	if (action_id == SWF::ACTION_END)
	{
	    // end of action buffer.
	    break;
	}
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
    
    if (m_decl_dict_processed_at != -1)	{
	log_warning("process_decl_dict(" SIZET_FMT ", " SIZET_FMT "): decl_dict was already processed at %d. "
		"Skipping (or maybe we should append, or replace?).",
		  start_pc, stop_pc, m_decl_dict_processed_at);
	return;
    }
    
    m_decl_dict_processed_at = start_pc;
    
    // Actual processing.
    size_t i = start_pc;
    uint16_t length = uint16_t(read_int16(i+1));
    uint16_t count = uint16_t(read_int16(i+3)); 
    i += 2;
    
//log_msg("Start at %d, stop at %d, length read was %d, count read was %d", start_pc, stop_pc, length, count);

    assert(start_pc + 3 + length == stop_pc);
    
    m_dictionary.resize(count);
    
    // Index the strings.
    for (int ct = 0; ct < count; ct++) {
	// Point into the current action buffer.
	m_dictionary[ct] = (const char*) &m_buffer[3 + i];
	
	while (m_buffer[3 + i]) {
	    // safety check.
	    if (i >= stop_pc) {
		log_error("action buffer dict length exceeded\n");
		
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
static void
disasm(const unsigned char* instruction_data)
{    

    using namespace gnash::SWF;

    as_arg_t fmt = ARG_HEX;
    action_type	action_id = (action_type)instruction_data[0];
    unsigned char num[10];
    memset(num, 0, 10);

    dbglogfile.setStamp(false);
    // Show instruction.
    if (action_id > ash.lastType()) {
	dbglogfile << "<unknown>[0x" << action_id  << "]" << endl;
    } else {
	dbglogfile << ash[action_id].getName().c_str() << endl;
	fmt = ash[action_id].getArgFormat();
    }

    // Show instruction argument(s).
    if (action_id & 0x80) {
	assert(fmt != ARG_NONE);
	int length = instruction_data[1] | (instruction_data[2] << 8);
	if (fmt == ARG_HEX) {
	    for (int i = 0; i < length; i++) {
		hexify(num, (const unsigned char *)&instruction_data[3 + i], 1, false);
		dbglogfile << "0x" << num << " ";
//		dbglogfile << instruction_data[3 + i] << " ";
	    }
	    dbglogfile << endl;
	} else if (fmt == ARG_STR) {
	    string str;
	    for (int i = 0; i < length; i++) {
		str += instruction_data[3 + i];
	    }
	    dbglogfile << "\"" << str.c_str() << "\"" << endl;
	} else if (fmt == ARG_U8) {
	    int	val = instruction_data[3];
	    dbglogfile << " " << val << endl;
	} else if (fmt == ARG_U16) {
	    int	val = instruction_data[3] | (instruction_data[4] << 8);
	    dbglogfile << " " << val << endl;
	} else if (fmt == ARG_S16) {
	    int	val = instruction_data[3] | (instruction_data[4] << 8);
	    if (val & 0x8000) val |= ~0x7FFF;	// sign-extend
	    dbglogfile << " " << val << endl;
	} else if (fmt == ARG_PUSH_DATA) {
	    dbglogfile << endl;
	    int i = 0;
	    while (i < length) {
		int	type = instruction_data[3 + i];
		i++;
		if (type == 0) {
		    // string
		    string str;
		    while (instruction_data[3 + i]) {
			str += instruction_data[3 + i];
			i++;
		    }
		    i++;
		    dbglogfile << "\t\"" << str.c_str() << "\"" << endl;
		} else if (type == 1) {
		    // float (little-endian)
		    union {
			float	f;
			uint32_t	i;
		    } u;
		    compiler_assert(sizeof(u) == sizeof(u.i));
		    
		    memcpy(&u.i, instruction_data + 3 + i, 4);
		    u.i = swap_le32(u.i);
		    i += 4;
		    
		    dbglogfile << "(float) " << u.f << endl;
		} else if (type == 2) {
		    dbglogfile << "NULL" << endl;
		} else if (type == 3) {
		    dbglogfile << "undef" << endl;
		} else if (type == 4) {
		    // contents of register
		    int	reg = instruction_data[3 + i];
		    i++;
		    dbglogfile << "reg[" << reg << "]" << endl;
		} else if (type == 5) {
		    int	bool_val = instruction_data[3 + i];
		    i++;
		    dbglogfile << "bool(" << bool_val << ")" << endl;
		} else if (type == 6) {
		    // double
		    // wacky format: 45670123
		    union {
			double	d;
			uint64_t	i;
			struct {
			    uint32_t	lo;
			    uint32_t	hi;
			} sub;
		    } u;
		    compiler_assert(sizeof(u) == sizeof(u.i));
		    
		    memcpy(&u.sub.hi, instruction_data + 3 + i, 4);
		    memcpy(&u.sub.lo, instruction_data + 3 + i + 4, 4);
		    u.i = swap_le64(u.i);
		    i += 8;
		    
		    dbglogfile << "(double) " << u.d << endl;
		} else if (type == 7) {
		    // int32_t
		    int32_t	val = instruction_data[3 + i]
			| (instruction_data[3 + i + 1] << 8)
			| (instruction_data[3 + i + 2] << 16)
			| (instruction_data[3 + i + 3] << 24);
		    i += 4;
		    dbglogfile << "(int) " << val << endl;
		} else if (type == 8) {
		    int	id = instruction_data[3 + i];
		    i++;
		    dbglogfile << "dict_lookup[" << id << "]" << endl;
		} else if (type == 9) {
		    int	id = instruction_data[3 + i] | (instruction_data[3 + i + 1] << 8);
		    i += 2;
		    dbglogfile << "dict_lookup_lg[" << id << "]" << endl;
		}
	    }
	} else if (fmt == ARG_DECL_DICT) {
	    int	i = 0;
	    int	count = instruction_data[3 + i] | (instruction_data[3 + i + 1] << 8);
	    i += 2;
	    
	    dbglogfile << " [" << count << "]" << endl;
	    
	    // Print strings.
	    for (int ct = 0; ct < count; ct++) {
		dbglogfile << "\t" << ct << ") "; 
		
		string str;
		while (instruction_data[3 + i]) {
			// safety check.
		    if (i >= length) {
			dbglogfile << "<disasm error -- length exceeded>" << endl;
			break;
		    }
		    str += instruction_data[3 + i];
		    i++;
		}
		dbglogfile << "\"" << str.c_str() << "\"" << endl;
		i++;
	    }
	} else if (fmt == ARG_FUNCTION2) {
	    // Signature info for a function2 opcode.
	    int	i = 0;
	    const char*	function_name = (const char*) &instruction_data[3 + i];
	    i += strlen(function_name) + 1;
	    
	    int	arg_count = instruction_data[3 + i] | (instruction_data[3 + i + 1] << 8);
	    i += 2;
	    
	    int	reg_count = instruction_data[3 + i];
	    i++;

	    dbglogfile << "\t\tname = '" << function_name << "'"
		       << " arg_count = " << arg_count
		       << " reg_count = " << reg_count << endl;
	    
	    uint16_t	flags = (instruction_data[3 + i]) | (instruction_data[3 + i + 1] << 8);
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
	    
	    log_msg("\t\t        pg = %d\n"
		    "\t\t        pp = %d\n"
		    "\t\t        pr = %d\n"
		    "\t\tss = %d, ps = %d\n"
		    "\t\tsa = %d, pa = %d\n"
		    "\t\tst = %d, pt = %d\n",
		    int(preload_global),
		    int(preload_parent),
		    int(preload_root),
		    int(suppress_super),
		    int(preload_super),
		    int(suppress_args),
		    int(preload_args),
		    int(suppress_this),
		    int(preload_this));
	    
	    for (int argi = 0; argi < arg_count; argi++) {
		int	arg_register = instruction_data[3 + i];
		i++;
		const char*	arg_name = (const char*) &instruction_data[3 + i];
		i += strlen(arg_name) + 1;
		
		dbglogfile << "\t\targ[" << argi << "]"
			   << " - reg[" << arg_register << "]"
			   << " - '" << arg_name << "'" << endl;
	    }
	    
	    int	function_length = instruction_data[3 + i] | (instruction_data[3 + i + 1] << 8);
	    i += 2;
	    
	    dbglogfile << "\t\tfunction length = " << function_length << endl;
	}
    } else {
	dbglogfile << endl;
    }
    dbglogfile.setStamp(true);
}

// Disassemble one instruction to the log.
void
action_buffer::log_disasm(size_t pc) const
{    
	const unsigned char* instruction_data =
		(const unsigned char *)&m_buffer[pc];
	disasm(instruction_data);
}


float
action_buffer::read_float_little(size_t pc) const
{
	union {
		float	f;
		uint32_t	i;
	} u;
	compiler_assert(sizeof(u) == sizeof(u.i));
	memcpy(&u.i, &m_buffer[pc], 4);
	u.i = swap_le32(u.i);
	return u.f;
}

double
action_buffer::read_double_wacky(size_t pc) const
{
	// double
	// wacky format: 45670123
	union {
		double	d;
		uint64_t	i;
		struct {
			uint32_t	lo;
			uint32_t	hi;
		} sub;
	} u;

	compiler_assert(sizeof(u) == sizeof(u.i));

	// this works, but is pretty dirty
	memcpy(&u.sub.hi, &m_buffer[pc], 4);
	memcpy(&u.sub.lo, &m_buffer[pc + 4], 4);
	u.i = swap_le64(u.i);

	return u.d;
}

}


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
