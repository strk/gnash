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

static SWFHandlers ash;

// Utility.  Try to convert str to a number.  If successful,
// put the result in *result, and return true.  If not
// successful, put 0 in *result, and return false.
static bool string_to_number(double* result, const char* str)
{
    char* tail = 0;
    *result = strtod(str, &tail);
    if (tail == str || *tail != 0)
	{
	    // Failed conversion to Number.
	    return false;
	}
    return true;
}

fscommand_callback s_fscommand_handler = NULL;

// External interface.
void	register_fscommand_callback(fscommand_callback handler)
{
    s_fscommand_handler = handler;
}



// Disassemble one instruction to the log.
static void
log_disasm(const unsigned char* instruction_data);

action_buffer::action_buffer()
    :
    m_decl_dict_processed_at(-1)
{
}


void
action_buffer::read(stream* in)
{
    // Read action bytes.
    for (;;) {
	int	instruction_start = m_buffer.size();
	
	int	pc = m_buffer.size();
	
	int	action_id = in->read_u8();
	m_buffer.push_back(action_id);
	
	if (action_id & 0x80) {
	    // Action contains extra data.  Read it.
	    int	length = in->read_u16();
	    m_buffer.push_back(length & 0x0FF);
	    m_buffer.push_back((length >> 8) & 0x0FF);
	    for (int i = 0; i < length; i++) {
		unsigned char b = in->read_u8();
		m_buffer.push_back(b);
	    }
	}

	dbglogfile.setStamp(false);
	log_action("PC index: %d:\t", pc);
	if (dbglogfile.getActionDump()) {
	    log_disasm(&m_buffer[instruction_start]);
	}
	
	if (action_id == 0) {
	    // end of action buffer.
	    break;
	}
    }
}


/*private*/
// Interpret the decl_dict opcode.  Don't read stop_pc or
// later.  A dictionary is some static strings embedded in the
// action buffer; there should only be one dictionary per
// action buffer.
//
// NOTE: Normally the dictionary is declared as the first
// action in an action buffer, but I've seen what looks like
// some form of copy protection that amounts to:
//
// <start of action buffer>
//          push true
//          branch_if_true label
//          decl_dict   [0]   // this is never executed, but has lots of orphan data declared in the opcode
// label:   // (embedded inside the previous opcode; looks like an invalid jump)
//          ... "protected" code here, including the real decl_dict opcode ...
//          <end of the dummy decl_dict [0] opcode>
//
// So we just interpret the first decl_dict we come to, and
// cache the results.  If we ever hit a different decl_dict in
// the same action_buffer, then we log an error and ignore it.
void
action_buffer::process_decl_dict(int start_pc, int stop_pc)
{
    assert(stop_pc <= (int) m_buffer.size());
    
    if (m_decl_dict_processed_at == start_pc) {
	// We've already processed this decl_dict.
	int	count = m_buffer[start_pc + 3] | (m_buffer[start_pc + 4] << 8);
	assert((int) m_dictionary.size() == count);
	UNUSED(count);
	return;
    }
    
    if (m_decl_dict_processed_at != -1)	{
	log_error("process_decl_dict(%d, %d): decl_dict was already processed at %d\n",
		  start_pc,
		  stop_pc,
		  m_decl_dict_processed_at);
	return;
    }
    
    m_decl_dict_processed_at = start_pc;
    
    // Actual processing.
    int	i = start_pc;
    int	length = m_buffer[i + 1] | (m_buffer[i + 2] << 8);
    int	count = m_buffer[i + 3] | (m_buffer[i + 4] << 8);
    i += 2;
    
    UNUSED(length);
    
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


// Interpret the actions in this action buffer, and evaluate
// them in the given environment.  Execute our whole buffer,
// without any arguments passed in.
void
action_buffer::execute(as_environment* env)
{
    int	local_stack_top = env->get_local_frame_top();
    env->add_frame_barrier();
    
    std::vector<with_stack_entry> empty_with_stack;
    execute(env, 0, m_buffer.size(), NULL, empty_with_stack, false /* not function2 */);
    
    env->set_local_frame_top(local_stack_top);
}

#if 1
/*private*/
void
action_buffer::doActionDefineFunction(as_environment* env,
		std::vector<with_stack_entry>& with_stack,
		size_t pc,
		size_t* next_pc)
{

    // Create a new function_as_object
    function_as_object* func = new function_as_object(this, env, *next_pc, with_stack);

    size_t	i = pc;
    i += 3;

    // Extract name.
    // @@ security: watch out for possible missing terminator here!
    tu_string	name = (const char*) &m_buffer[i];
    i += name.length() + 1;

    // Get number of arguments.
    int	nargs = m_buffer[i] | (m_buffer[i + 1] << 8);
    i += 2;

    // Get the names of the arguments.
    for (int n = 0; n < nargs; n++) {
	// @@ security: watch out for possible missing terminator here!
	func->add_arg(0, (const char*) &m_buffer[i]);
	i += func->m_args.back().m_name.length() + 1;
    }
    
    // Get the length of the actual function code.
    int	length = m_buffer[i] | (m_buffer[i + 1] << 8);
    i += 2;
    func->set_length(length);

    // Skip the function body (don't interpret it now).
    *next_pc += length;

    // If we have a name, then save the function in this
    // environment under that name.
    as_value	function_value(func);
    if (name.length() > 0) {
	// @@ NOTE: should this be m_target->set_variable()???
	env->set_member(name, function_value);
    }
    
    // Also leave it on the stack.
    env->push_val(function_value);
}

/*private*/
void
action_buffer::doActionDefineFunction2(as_environment* env,
		std::vector<with_stack_entry>& with_stack,
		size_t pc,
		size_t* next_pc)
{
    function_as_object*	func = new function_as_object(this, env, *next_pc, with_stack);
    func->set_is_function2();

    size_t i = pc;
    i += 3;

    // Extract name.
    // @@ security: watch out for possible missing terminator here!
    tu_string	name = (const char*) &m_buffer[i];
    i += name.length() + 1;

    // Get number of arguments.
    int	nargs = m_buffer[i] | (m_buffer[i + 1] << 8);
    i += 2;

    // Get the count of local registers used by this function.
    uint8	register_count = m_buffer[i];
    i += 1;
    func->set_local_register_count(register_count);

    // Flags, for controlling register assignment of implicit args.
    uint16	flags = m_buffer[i] | (m_buffer[i + 1] << 8);
    i += 2;
    func->set_function2_flags(flags);

    // Get the register assignments and names of the arguments.
    for (int n = 0; n < nargs; n++) {
	int	arg_register = m_buffer[i];
	i++;
	
	// @@ security: watch out for possible missing terminator here!
	func->add_arg(arg_register, (const char*) &m_buffer[i]);
	    i += func->m_args.back().m_name.length() + 1;
    }

    // Get the length of the actual function code.
    int	length = m_buffer[i] | (m_buffer[i + 1] << 8);
    i += 2;
    func->set_length(length);

    // Skip the function body (don't interpret it now).
    *next_pc += length;

    // If we have a name, then save the function in this
    // environment under that name.
    as_value	function_value(func);
    if (name.length() > 0) {
	// @@ NOTE: should this be m_target->set_variable()???
	env->set_member(name, function_value);
    }
    
    // Also leave it on the stack.
    env->push_val(function_value);
}
#endif

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
    size_t exec_bytes,
    as_value* /* retval */ , //should we drop this parameter ?
    const std::vector<with_stack_entry>& initial_with_stack,
    bool is_function2)
{
    action_init();	// @@ stick this somewhere else; need some global static init function

    assert(env);

    std::vector<with_stack_entry>	with_stack(initial_with_stack);

#if 0
    // Check the time
    if (periodic_events.expired()) {
	periodic_events.poll_event_handlers(env);
    }
#endif
		
    movie*	original_target = env->get_target();
    UNUSED(original_target);		// Avoid warnings.

    size_t stop_pc = start_pc + exec_bytes;

    for (size_t pc = start_pc; pc < stop_pc; ) {
	// Cleanup any expired "with" blocks.
	while ( with_stack.size() > 0
	       && pc >= with_stack.back().end_pc() ) {
	    // Drop this stack element
	    with_stack.resize(with_stack.size() - 1);
	}
	
	// Get the opcode.
	int	action_id = m_buffer[pc];
	if ((action_id & 0x80) == 0) {
	    if (dbglogfile.getActionDump()) {
		log_action("\nEX:\t");
		log_disasm(&m_buffer[pc]);
	    }
	    
	    // IF_VERBOSE_ACTION(log_msg("Action ID is: 0x%x\n", action_id));
	    
	    ash.execute((action_type)action_id, *env);
	    pc++;	// advance to next action.
	} else {

	    if (dbglogfile.getActionDump()) {
		log_action("\nEX:\t");
		log_disasm(&m_buffer[pc]);
	    }
	    
	    // Action containing extra data.
	    size_t length = m_buffer[pc + 1] | (m_buffer[pc + 2] << 8);
	    size_t next_pc = pc + length + 3;
	    
	    switch (action_id) {
	      default:
		  break;
		  
	      case SWF::ACTION_GOTOFRAME:	// goto frame
	      {
		  int	frame = m_buffer[pc + 3] | (m_buffer[pc + 4] << 8);
		  // 0-based already?
		  //// Convert from 1-based to 0-based
		  //frame--;
		  env->get_target()->goto_frame(frame);
		  break;
	      }
	      
	      case SWF::ACTION_GETURL:	// get url
	      {
		  // If this is an FSCommand, then call the callback
		  // handler, if any.
		  
		  // Two strings as args.
		  const char*	url = (const char*) &(m_buffer[pc + 3]);
		  size_t	url_len = strlen(url);
		  const char*	target = (const char*) &(m_buffer[pc + 3 + url_len + 1]);
		  
		  // If the url starts with an "http" or "https",
		  // then we want to load it into a web browser.
		  if (strncmp(url, "http", 4) == 0) {
// 				  if (windowid) {
// 				      Atom mAtom = 486;
// 				      Display *mDisplay = XOpenDisplay(NULL);
// 				      XLockDisplay(mDisplay);
// 				      XChangeProperty (mDisplay, windowid, mAtom,
// 						       XA_STRING, 8, PropModeReplace,
// 						       (unsigned char *)url,
// 						       url_len);
		      
// 				      XUnlockDisplay(mDisplay);
// 				      XCloseDisplay(mDisplay);
// 				  } else {
		      string command = "firefox -remote \"openurl(";
		      command += url;
		      command += ")\"";
		      dbglogfile << "Launching URL... " << command << endl;
//				  movie *target = env->get_target();
//				  target->get_url(url);
		      system(command.c_str());
//				  }
		      break;
		  }
		  
		  // If the url starts with "FSCommand:", then this is
		  // a message for the host app.
		  if (strncmp(url, "FSCommand:", 10) == 0) {
		      if (s_fscommand_handler) {
			  // Call into the app.
			  (*s_fscommand_handler)(env->get_target()->get_root_interface(), url + 10, target);
		      }
		  } else {
#ifdef EXTERN_MOVIE
//				      log_error("get url: target=%s, url=%s\n", target, url);
		      
		      tu_string tu_target = target;
		      movie* target_movie = env->find_target(tu_target);
		      if (target_movie != NULL) {
			  movie *root_movie = env->get_target()->get_root_movie();
			  attach_extern_movie(url, target_movie, root_movie);
		      } else {
			  log_error("get url: target %s not found\n", target);
		      }
#endif // EXTERN_MOVIE
		  }
		  
		  break;
	      }
	      
	      case SWF::ACTION_SETREGISTER:	// store_register
	      {
		  int	reg = m_buffer[pc + 3];
		  // Save top of stack in specified register.
		  if (is_function2) {
		      *(env->local_register_ptr(reg)) = env->top(0);
		      
		          log_action("-------------- local register[%d] = '%s'\n",
			  reg,
			  env->top(0).to_string());
		  } else if (reg >= 0 && reg < 4) {
		      env->m_global_register[reg] = env->top(0);
		      
			  log_action("-------------- global register[%d] = '%s'\n",
				  reg,
				  env->top(0).to_string());
		  } else {
		      log_error("store_register[%d] -- register out of bounds!", reg);
		  }
		  
		  break;
	      }
	      
	      case SWF::ACTION_CONSTANTPOOL:	// decl_dict: declare dictionary
	      {
		  //int	i = pc;
		  //int	count = m_buffer[pc + 3] | (m_buffer[pc + 4] << 8);
		  //i += 2;
		  
		  process_decl_dict(pc, next_pc);
		  
		  break;
	      }
	      
	      case SWF::ACTION_WAITFORFRAME:	// wait for frame
	      {
		  // If we haven't loaded a specified frame yet, then we're supposed to skip
		  // some specified number of actions.
		  //
		  // Since we don't load incrementally, just ignore this opcode.
		  break;
	      }
	      
	      case SWF::ACTION_SETTARGET:	// set target
	      {
		  // Change the movie we're working on.
		  const char* target_name = (const char*) &m_buffer[pc + 3];
		  movie *new_target;
		  
		  // if the string is blank, we set target to the root movie
		  // TODO - double check this is correct?
		  if (target_name[0] == '\0')
		      new_target = env->find_target((tu_string)"/");
		  else
		      new_target = env->find_target((tu_string)target_name);
		  
		  if (new_target == NULL) {
		      log_action("ERROR: Couldn't find movie \"%s\" to set target to!"
					    " Not setting target at all...",
					    (const char *)target_name);
		  }
		  else
		      env->set_target(new_target);
		  
		  break;
	      }
	      
	      case SWF::ACTION_GOTOLABEL:	// go to labeled frame, goto_frame_lbl
	      {
		  char*	frame_label = (char*) &m_buffer[pc + 3];
		  movie *target = env->get_target();
		  target->goto_labeled_frame(frame_label);
		  break;
	      }
	      
	      case SWF::ACTION_WAITFORFRAMEEXPRESSION:	// wait for frame expression (?)
	      {
		  // Pop the frame number to wait for; if it's not loaded skip the
		  // specified number of actions.
		  //
		  // Since we don't support incremental loading, pop our arg and
		  // don't do anything.
		  env->drop(1);
		  break;
	      }
	      
	      case SWF::ACTION_DEFINEFUNCTION2: // 0x8E
		  doActionDefineFunction2(env, with_stack, pc, &next_pc);
		  break;
		  
	      case SWF::ACTION_WITH:	// with
	      {
		  int	frame = m_buffer[pc + 3] | (m_buffer[pc + 4] << 8);
		  UNUSED(frame);
		  log_action("-------------- with block start: stack size is %zd\n", with_stack.size());
		  if (with_stack.size() < 8) {
		      int	block_length = m_buffer[pc + 3] | (m_buffer[pc + 4] << 8);
		      int	block_end = next_pc + block_length;
		      as_object*	with_obj = env->top(0).to_object();
		      with_stack.push_back(with_stack_entry(with_obj, block_end));
		  }
		  env->drop(1);
		  break;
	      }
	      case SWF::ACTION_PUSHDATA:	// push_data
	      {
		  size_t i = pc;
		  while (i - pc < length) {
		      int	type = m_buffer[3 + i];
		      i++;
		      if (type == 0) {
			  // string
			  const char*	str = (const char*) &m_buffer[3 + i];
			  i += strlen(str) + 1;
			  env->push(str);
			  
			  log_action("-------------- pushed '%s'", str);
		      } else if (type == 1) {
			  // float (little-endian)
			  union {
			      float	f;
			      uint32_t	i;
			  } u;
			  compiler_assert(sizeof(u) == sizeof(u.i));
			  
			  memcpy(&u.i, &m_buffer[3 + i], 4);
			  u.i = swap_le32(u.i);
			  i += 4;
			  
			  env->push(u.f);
			  log_action("-------------- pushed '%g'", u.f);
		      } else if (type == 2) {
			  as_value nullvalue;
			  nullvalue.set_null();
			  env->push(nullvalue);	
			  
			  log_action("-------------- pushed NULL");
		      } else if (type == 3) {
			  env->push(as_value());
			  
			  log_action("-------------- pushed UNDEFINED");
		      } else if (type == 4) {
			  // contents of register
			  int	reg = m_buffer[3 + i];
			  UNUSED(reg);
			  i++;
			  if (is_function2) {
			      env->push(*(env->local_register_ptr(reg)));
			      log_action("-------------- pushed local register[%d] = '%s'\n",
					  reg,
					  env->top(0).to_string());
			  } else if (reg < 0 || reg >= 4) {
			      env->push(as_value());
			      log_error("push register[%d] -- register out of bounds!\n", reg);
			  } else {
			      env->push(env->m_global_register[reg]);
			      log_action("-------------- pushed global register[%d] = '%s'\n",
					  reg,
					  env->top(0).to_string());
			  }
			  
		      } else if (type == 5) {
			  bool	bool_val = m_buffer[3 + i] ? true : false;
			  i++;
//							log_msg("bool(%d)\n", bool_val);
			  env->push(bool_val);
			  
			  log_action("-------------- pushed %s",
				     (bool_val ? "true" : "false"));
		      } else if (type == 6) {
			  // double
			  // wacky format: 45670123
			  union {
			      double	d;
			      uint64	i;
			      struct {
				  uint32_t	lo;
				  uint32_t	hi;
			      } sub;
			  } u;
			  compiler_assert(sizeof(u) == sizeof(u.i));
			  
			  memcpy(&u.sub.hi, &m_buffer[3 + i], 4);
			  memcpy(&u.sub.lo, &m_buffer[3 + i + 4], 4);
			  u.i = swap_le64(u.i);
			  i += 8;
			  
			  env->push(u.d);
			  
			  log_action("-------------- pushed double %g", u.d);
		      } else if (type == 7) {
			  // int32
			  int32_t	val = m_buffer[3 + i]
			      | (m_buffer[3 + i + 1] << 8)
			      | (m_buffer[3 + i + 2] << 16)
			      | (m_buffer[3 + i + 3] << 24);
			  i += 4;
			  
			  env->push(val);
			  
			  log_action("-------------- pushed int32 %d",val);
		      } else if (type == 8) {
			  int	id = m_buffer[3 + i];
			  i++;
			  if (id < (int) m_dictionary.size()) {
			      env->push(m_dictionary[id]);
			      
			      log_action("-------------- pushed '%s'",
					 m_dictionary[id]);
			  } else {
			      log_error("dict_lookup(%d) is out of bounds!\n", id);
			      env->push(0);
			      log_action("-------------- pushed 0");
			  }
		      } else if (type == 9) {
			  int	id = m_buffer[3 + i] | (m_buffer[4 + i] << 8);
			  i += 2;
			  if (id < (int) m_dictionary.size()) {
			      env->push(m_dictionary[id]);
			      log_action("-------------- pushed '%s'\n", m_dictionary[id]);
			  } else {
			      log_error("dict_lookup(%d) is out of bounds!\n", id);
			      env->push(0);
			      
			      log_action("-------------- pushed 0");
			  }
		      }
		  }
		  
		  break;
	      }
	      case SWF::ACTION_BRANCHALWAYS:	// branch always (goto)
	      {
		  int16_t	offset = m_buffer[pc + 3] | (m_buffer[pc + 4] << 8);
		  next_pc += offset;
		  // @@ TODO range checks
		  break;
	      }
	      case SWF::ACTION_GETURL2:	// get url 2
	      {
		  int	method = m_buffer[pc + 3];
		  UNUSED(method);
		  
		  const char*	target = env->top(0).to_string();
		  const char*	url = env->top(1).to_string();
		  
		  // If the url starts with "FSCommand:", then this is
		  // a message for the host app.
		  if (strncmp(url, "FSCommand:", 10) == 0) {
		      if (s_fscommand_handler) {
			  // Call into the app.
			  (*s_fscommand_handler)(env->get_target()->get_root_interface(), url + 10, target);
		      }
		  } else {
#ifdef EXTERN_MOVIE
//            log_error("get url2: target=%s, url=%s\n", target, url);
		      
		      movie* target_movie = env->find_target(env->top(0));
		      if (target_movie != NULL) {
			  movie*	root_movie = env->get_target()->get_root_movie();
			  attach_extern_movie(url, target_movie, root_movie);
		      } else {
			  log_error("get url2: target %s not found\n", target);
		      }
#endif // EXTERN_MOVIE
		  }
		  env->drop(2);
		  break;
	      }
	      
	      case SWF::ACTION_DEFINEFUNCTION: // declare function
		  doActionDefineFunction(env, with_stack, pc, &next_pc);
		  break;
		  
	      case SWF::ACTION_BRANCHIFTRUE:	// branch if true
	      {
		  int16_t	offset = m_buffer[pc + 3] | (m_buffer[pc + 4] << 8);
		  
		  bool	test = env->top(0).to_bool();
		  env->drop(1);
		  if (test) {
		      next_pc += offset;
		      
		      if (next_pc > stop_pc) {
			  log_error("branch to offset %d -- this section only runs to %d\n",
				    next_pc,
				    stop_pc);
		      }
		  }
		  break;
	      }
	      case SWF::ACTION_CALLFRAME:	// call frame
	      {
		  // Note: no extra data in this instruction!
		  assert(env->m_target);
		  env->m_target->call_frame_actions(env->top(0));
		  env->drop(1);
		  
		  break;
	      }
	      
	      case SWF::ACTION_GOTOEXPRESSION:	// goto frame expression, goto_frame_exp
	      {
		  // From Alexi's SWF ref:
		  //
		  // Pop a value or a string and jump to the specified
		  // frame. When a string is specified, it can include a
		  // path to a sprite as in:
		  // 
		  //   /Test:55
		  // 
		  // When f_play is ON, the action is to play as soon as
		  // that frame is reached. Otherwise, the
		  // frame is shown in stop mode.
		  
		  unsigned char	play_flag = m_buffer[pc + 3];
		  movie::play_state	state = play_flag ? movie::PLAY : movie::STOP;
		  
		  movie* target = env->get_target();
		  bool success = false;
		  
		  if (env->top(0).get_type() == as_value::UNDEFINED) {
		      // No-op.
		  } else if (env->top(0).get_type() == as_value::STRING) {
		      // @@ TODO: parse possible sprite path...
		      
		      // Also, if the frame spec is actually a number (not a label), then
		      // we need to do the conversion...
		      
		      const char* frame_label = env->top(0).to_string();
		      if (target->goto_labeled_frame(frame_label)) {
			  success = true;
		      } else {
			  // Couldn't find the label.  Try converting to a number.
			  double num;
			  if (string_to_number(&num, env->top(0).to_string())) {
			      int frame_number = int(num);
			      target->goto_frame(frame_number);
			      success = true;
			  }
			  // else no-op.
		      }
		  } else if (env->top(0).get_type() == as_value::OBJECT) {
		      // This is a no-op; see test_goto_frame.swf
		  } else if (env->top(0).get_type() == as_value::NUMBER) {
		      // Frame numbers appear to be 0-based!  @@ Verify.
		      int frame_number = int(env->top(0).to_number());
		      target->goto_frame(frame_number);
		      success = true;
		  }
		  
		  if (success) {
		      target->set_play_state(state);
		  }
		  
		  env->drop(1);  
		  break;
	      }	      
	    }
	    pc = next_pc;
	}
    }
    
    env->set_target(original_target);
}

//
// Disassembler
//

// Disassemble one instruction to the log.
void
log_disasm(const unsigned char* instruction_data)
{    
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
		hexify(num, (const unsigned char *)&instruction_data[3 + i], 1);
		dbglogfile << "0x" << num << " ";
//		dbglogfile << instruction_data[3 + i] << " ";
	    }
	    dbglogfile << endl;
	} else if (fmt == ARG_STR) {
	    string str;
	    for (int i = 0; i < length; i++) {
		str = instruction_data[3 + i];
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
			uint64	i;
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
		    // int32
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
		dbglogfile << "\t" << endl;	// indent
		
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
	    
	    uint16	flags = (instruction_data[3 + i]) | (instruction_data[3 + i + 1] << 8);
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



};


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
