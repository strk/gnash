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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ActionExec.h"
#include "action_buffer.h"
#include "Function.h" // for function_as_object
#include "log.h"
#include "stream.h"

#include "swf.h"
#include "ASHandlers.h"
#include "as_environment.h"

#include <typeinfo> 

#if !defined(_WIN32) && !defined(WIN32)
# include <pthread.h> 
#endif

#include <string>
#include <stdlib.h> // for strtod

using namespace gnash;
using namespace SWF;
using std::string;
using std::endl;


namespace gnash {

static const SWFHandlers& ash = SWFHandlers::instance();

// External interface (to be moved under swf/ASHandlers)
fscommand_callback s_fscommand_handler = NULL;
void	register_fscommand_callback(fscommand_callback handler)
{
    s_fscommand_handler = handler;
}

ActionExec::ActionExec(const action_buffer& abuf, as_environment& newEnv,
		size_t nStartPC, size_t exec_bytes, as_value* retval, 
		const std::vector<with_stack_entry>& initial_with_stack,
		bool nIsFunction2)
	:
	code(abuf),
	pc(nStartPC),
	stop_pc(nStartPC+exec_bytes),
	next_pc(nStartPC),
	env(newEnv),
	retval(retval),
	with_stack(initial_with_stack),
	_function2_var(nIsFunction2)
{
}

ActionExec::ActionExec(const action_buffer& abuf, as_environment& newEnv)
	:
	code(abuf),
	pc(0),
	stop_pc(code.size()),
	next_pc(0),
	env(newEnv),
	retval(0),
	with_stack(),
	_function2_var(false)
{
}

void
ActionExec::operator() ()
{
    action_init();	// @@ stick this somewhere else; need some global static init function

#if 0
    // Check the time
    if (periodic_events.expired()) {
	periodic_events.poll_event_handlers(&env);
    }
#endif
		
    movie*	original_target = env.get_target();
    UNUSED(original_target);		// Avoid warnings.

    while (pc<stop_pc)
    {

	// Cleanup any expired "with" blocks.
	while ( with_stack.size() > 0
	       && pc >= with_stack.back().end_pc() ) {
	    // Drop this stack element
	    with_stack.resize(with_stack.size() - 1);
	}
	
	// Get the opcode.
	uint8_t action_id = code[pc];

	if (dbglogfile.getActionDump()) {
		log_action("\nEX:\t");
		code.log_disasm(pc);
	}

	// Set default next_pc offset, control flow action handlers
	// will be able to reset it. 
	if ((action_id & 0x80) == 0) {
		// action with no extra data
		next_pc = pc+1;
	} else {
		// action with extra data
		int16_t length = code.read_int16(pc+1);
		assert( length >= 0 );
		next_pc = pc + length + 3;
	}

	// Do we still need this ?
	if ( action_id == SWF::ACTION_END ) {
		// this would turn into an assertion (next_pc==stop_pc)
		log_msg("At ACTION_END next_pc=%d, stop_pc=%d", next_pc, stop_pc);
		break;
	}

	ash.execute((action_type)action_id, *this);

	// Control flow actions will change the PC (next_pc)
	pc = next_pc;

    }
    
    env.set_target(original_target);
}


};


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
