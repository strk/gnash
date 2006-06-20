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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//
//

#ifndef GNASH_ACTION_BUFFER_H
#define GNASH_ACTION_BUFFER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "gnash.h"
//#include "as_object.h"
#include "types.h"
#include <wchar.h>

#include "container.h"
#include "smart_ptr.h"
//#include "log.h"
#include "with_stack_entry.h"

namespace gnash {

// Forward declarations
//struct movie;
struct as_environment;
//class as_object;
struct as_value;
//class function_as_object;

/// Base class for actions.
class action_buffer
{

public:

	action_buffer();

	/// Read action bytes from input stream
	void	read(stream* in);

	/// \brief
	/// Interpret the actions in this action buffer, and evaluate
	/// them in the given environment. 
	//
	/// Execute our whole buffer,
	/// without any arguments passed in.
	///
	void	execute(as_environment* env);

	/// Interpret the specified subset of the actions in our buffer.
	//
	/// Caller is responsible for cleaning up our local
	/// stack frame (it may have passed its arguments in via the
	/// local stack frame).
	void	execute(
		as_environment* env,
		size_t start_pc,
		size_t exec_bytes,
		as_value* retval, // we should probably drop this parameter
		const std::vector<with_stack_entry>& initial_with_stack,
		bool is_function2);

	bool	is_null()
	{
		return m_buffer.size() < 1 || m_buffer[0] == 0;
	}

	int	get_length() const { return m_buffer.size(); }

private:
	// Don't put these as values in std::vector<>!  They contain
	// internal pointers and cannot be moved or copied.
	// If you need to keep an array of them, keep pointers
	// to new'd instances.
	action_buffer(const action_buffer& a) { assert(0); }
	void operator=(const action_buffer& a) { assert(0); }

	void	process_decl_dict(int start_pc, int stop_pc);

	// data:
	std::vector<unsigned char>	m_buffer;
	std::vector<const char*>	m_dictionary;
	int	m_decl_dict_processed_at;

	void doActionNew(as_environment* env, 
		std::vector<with_stack_entry>& with_stack);

	void doActionInstanceOf(as_environment* env);

	void doActionCast(as_environment* env);

	void doActionCallMethod(as_environment* env);

	void doActionCallFunction(as_environment* env,
		std::vector<with_stack_entry>& with_stack);

	void doActionDefineFunction(as_environment* env,
		std::vector<with_stack_entry>& with_stack,
		size_t pc, size_t* next_pc);

	void doActionDefineFunction2(as_environment* env,
		std::vector<with_stack_entry>& with_stack,
		size_t pc, size_t* next_pc);

	void doActionGetMember(as_environment* env);

	void doActionStrictEquals(as_environment* env);

	void doActionEquals(as_environment* env);

	void doActionDelete(as_environment* env,
		std::vector<with_stack_entry>& with_stack);

	void doActionDelete2(as_environment* env,
		std::vector<with_stack_entry>& with_stack);

};


}	// end namespace gnash


#endif // GNASH_ACTION_BUFFER_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
