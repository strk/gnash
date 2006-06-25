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

	if ( action_id == SWF::ACTION_RETURN ) {
		break;
	}

	if ( action_id == SWF::ACTION_END ) {
		log_msg("At ACTION_END pc=%d, stop_pc=%d", pc, stop_pc);
		break;
	}

	ash.execute((action_type)action_id, *this);

	// Control flow actions will change the PC (next_pc)
	pc = next_pc;

//
// The following handlers implementations must be moved
// to swf/ASHandlers. Are kept for reference
// 
#if 0 // {
	if ( ! ash.execute((action_type)action_id, *this) )
	{
	    // Action handler not yet ported to new layout
	    
#if 0
	    switch (action_id) {

	      case SWF::ACTION_GOTOFRAME:	// goto frame
	      {
		  int frame = code.read_int16(pc+3);
		  // 0-based already?
		  //// Convert from 1-based to 0-based
		  //frame--;
		  env.get_target()->goto_frame(frame);
		  break;
	      }
	      
#if 0
	      case SWF::ACTION_GETURL:	// get url
	      {
		  // If this is an FSCommand, then call the callback
		  // handler, if any.
		  
		  // Two strings as args.
		  const char*	url = code.read_string(pc+3);
		  size_t	url_len = strlen(url);
		  const char*	target = code.read_string(pc+3+url_len+1);
		  
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
//				  movie *target = env.get_target();
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
			  (*s_fscommand_handler)(env.get_target()->get_root_interface(), url + 10, target);
		      }
		  } else {
#ifdef EXTERN_MOVIE
//				      log_error("get url: target=%s, url=%s\n", target, url);
		      
		      tu_string tu_target = target;
		      movie* target_movie = env.find_target(tu_target);
		      if (target_movie != NULL) {
			  movie *root_movie = env.get_target()->get_root_movie();
			  attach_extern_movie(url, target_movie, root_movie);
		      } else {
			  log_error("get url: target %s not found\n", target);
		      }
#endif // EXTERN_MOVIE
		  }
		  
		  break;
	      }
#endif
	      
	      case SWF::ACTION_SETREGISTER:	// store_register
	      {
		  int	reg = code[pc + 3];
		  // Save top of stack in specified register.
		  if ( isFunction2() ) {
		      *(env.local_register_ptr(reg)) = env.top(0);
		      
		          log_action("-------------- local register[%d] = '%s'\n",
			  reg,
			  env.top(0).to_string());
		  } else if (reg >= 0 && reg < 4) {
		      env.m_global_register[reg] = env.top(0);
		      
			  log_action("-------------- global register[%d] = '%s'\n",
				  reg,
				  env.top(0).to_string());
		  } else {
		      log_error("store_register[%d] -- register out of bounds!", reg);
		  }
		  
		  break;
	      }
	      
#if 0
	      case SWF::ACTION_CONSTANTPOOL:	// decl_dict: declare dictionary
	      {
		  //int	i = pc;
		  //int	count = code[pc + 3] | (code[pc + 4] << 8);
		  //i += 2;
		  
		  code.process_decl_dict(pc, next_pc);
		  
		  break;
	      }
#endif
	      
#if 0
	      case SWF::ACTION_WAITFORFRAME:	// wait for frame
	      {
		  // If we haven't loaded a specified frame yet, then we're supposed to skip
		  // some specified number of actions.
		  //
		  // Since we don't load incrementally, just ignore this opcode.
		  break;
	      }
#endif
	      
	      case SWF::ACTION_SETTARGET:	// set target
	      {
		  // Change the movie we're working on.
		  const char* target_name = code.read_string(pc+3);
		  movie *new_target;
		  
		  // if the string is blank, we set target to the root movie
		  // TODO - double check this is correct?
		  if (target_name[0] == '\0')
		      new_target = env.find_target((tu_string)"/");
		  else
		      new_target = env.find_target((tu_string)target_name);
		  
		  if (new_target == NULL) {
		      log_action("ERROR: Couldn't find movie \"%s\" to set target to!"
					    " Not setting target at all...",
					    (const char *)target_name);
		  }
		  else
		      env.set_target(new_target);
		  
		  break;
	      }
	      
	      case SWF::ACTION_GOTOLABEL:	// go to labeled frame, goto_frame_lbl
	      {
		  const char* frame_label = code.read_string(pc+3);
		  movie *target = env.get_target();
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
		  env.drop(1);
		  break;
	      }
	      
#if 0
	      case SWF::ACTION_DEFINEFUNCTION2: // 0x8E
		  code.doActionDefineFunction2(env, with_stack, pc, &next_pc);
		  break;
#endif
		  
	      case SWF::ACTION_WITH:	// with
	      {
		  int frame = code.read_int16(pc+3); 
		  UNUSED(frame);
		  log_action("-------------- with block start: stack size is %zd\n", with_stack.size());
		  if (with_stack.size() < 8) {
		      int	block_length = code.read_int16(pc+3);
		      int	block_end = next_pc + block_length;
		      as_object*	with_obj = env.top(0).to_object();
		      with_stack.push_back(with_stack_entry(with_obj, block_end));
		  }
		  env.drop(1);
		  break;
	      }
#if 0
	      case SWF::ACTION_PUSHDATA:	// push_data
	      {
		  size_t i = pc;
		  while (i - pc < length) {
		      int	type = code[3 + i];
		      i++;
		      if (type == 0) {
			  // string
			  const char* str = code.read_string(i+3);
			  i += strlen(str) + 1;
			  env.push(str);
			  
			  log_action("----string---- pushed '%s'", str);
		      } else if (type == 1) {
			
			  float f = code.read_float_little(i+3);
			  i += 4;
			  env.push(f);
			  log_action("----float----- pushed '%g'", f);
		      } else if (type == 2) {
			  as_value nullvalue;
			  nullvalue.set_null();
			  env.push(nullvalue);	
			  
			  log_action("----null------ pushed NULL");
		      } else if (type == 3) {
			  env.push(as_value());
			  
			  log_action("----undef----- pushed UNDEFINED");
		      } else if (type == 4) {
			  // contents of register
			  int	reg = code[3 + i];
			  i++;
			  if ( isFunction2() ) {
			      env.push(*(env.local_register_ptr(reg)));
			      log_action("-------------- pushed local register[%d] = '%s'\n",
					  reg,
					  env.top(0).to_string());
			  } else if (reg < 0 || reg >= 4) {
			      env.push(as_value());
			      log_error("push register[%d] -- register out of bounds!\n", reg);
			  } else {
			      env.push(env.m_global_register[reg]);
			      log_action("-------------- pushed global register[%d] = '%s'\n",
					  reg,
					  env.top(0).to_string());
			  }
			  
		      } else if (type == 5) {
			  bool	bool_val = code[i+3] ? true : false;
			  i++;
//			  log_msg("bool(%d)\n", bool_val);
			  env.push(bool_val);
			  
			  log_action("---bool------- pushed %s",
				     (bool_val ? "true" : "false"));
		      } else if (type == 6) {
			  double d = code.read_double_wacky(i+3);
			  i += 8;
			  env.push(d);
			  
			  log_action("-------------- pushed double %g", u.d);
		      } else if (type == 7) {
			  // int32
			  int32_t val = code.read_int32(i+3);
			  i += 4;
			  
			  env.push(val);
			  
			  log_action("-------------- pushed int32 %d",val);
		      } else if (type == 8) {
			  int id = code[3 + i];
			  i++;
			  if (id < (int) code.m_dictionary.size()) {
			      env.push(code.m_dictionary[id]);
			      
			      log_action("----dict------ pushed '%s'",
					 code.m_dictionary[id]);
			  } else {
			      log_error("dict_lookup(%d) is out of bounds!\n", id);
			      env.push(0);
			      log_action("-------------- pushed 0");
			  }
		      } else if (type == 9) {
			  int	id = code.read_int16(i+3);
			  i += 2;
			  if (id < (int) code.m_dictionary.size()) {
			      env.push(code.m_dictionary[id]);
			      log_action("-------------- pushed '%s'\n", code.m_dictionary[id]);
			  } else {
			      log_error("dict_lookup(%d) is out of bounds!\n", id);
			      env.push(0);
			      
			      log_action("-------------- pushed 0");
			  }
		      }
		  }
		  
		  break;
	      }
#endif
	      case SWF::ACTION_BRANCHALWAYS:	// branch always (goto)
	      {
		  int16_t offset = code.read_int16(pc+3);
		  next_pc += offset;
		  // @@ TODO range checks
		  break;
	      }
#if 0
	      case SWF::ACTION_GETURL2:	// get url 2
	      {
		  int	method = code[pc + 3];
		  UNUSED(method);
		  
		  const char*	target = env.top(0).to_string();
		  const char*	url = env.top(1).to_string();
		  
		  // If the url starts with "FSCommand:", then this is
		  // a message for the host app.
		  if (strncmp(url, "FSCommand:", 10) == 0) {
		      if (s_fscommand_handler) {
			  // Call into the app.
			  (*s_fscommand_handler)(env.get_target()->get_root_interface(), url + 10, target);
		      }
		  } else {
#ifdef EXTERN_MOVIE
//            log_error("get url2: target=%s, url=%s\n", target, url);
		      
		      movie* target_movie = env.find_target(env.top(0));
		      if (target_movie != NULL) {
			  movie*	root_movie = env.get_target()->get_root_movie();
			  attach_extern_movie(url, target_movie, root_movie);
		      } else {
			  log_error("get url2: target %s not found\n", target);
		      }
#endif // EXTERN_MOVIE
		  }
		  env.drop(2);
		  break;
	      }
#endif
	      
#if 0
	      case SWF::ACTION_DEFINEFUNCTION: // declare function
		  doActionDefineFunction(env, with_stack, pc, &next_pc);
		  break;
#endif
		  
	      case SWF::ACTION_BRANCHIFTRUE:	// branch if true
	      {
		  int16_t offset = code.read_int16(pc+3);
		  
		  bool	test = env.top(0).to_bool();
		  env.drop(1);
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
		  assert(env.m_target);
		  env.m_target->call_frame_actions(env.top(0));
		  env.drop(1);
		  
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
		  
		  unsigned char	play_flag = code[pc + 3];
		  movie::play_state	state = play_flag ? movie::PLAY : movie::STOP;
		  
		  movie* target = env.get_target();
		  bool success = false;
		  
		  if (env.top(0).get_type() == as_value::UNDEFINED) {
		      // No-op.
		  } else if (env.top(0).get_type() == as_value::STRING) {
		      // @@ TODO: parse possible sprite path...
		      
		      // Also, if the frame spec is actually a number (not a label), then
		      // we need to do the conversion...
		      
		      const char* frame_label = env.top(0).to_string();
		      if (target->goto_labeled_frame(frame_label)) {
			  success = true;
		      } else {
			  // Couldn't find the label.  Try converting to a number.
			  double num;
			  if (string_to_number(&num, env.top(0).to_string())) {
			      int frame_number = int(num);
			      target->goto_frame(frame_number);
			      success = true;
			  }
			  // else no-op.
		      }
		  } else if (env.top(0).get_type() == as_value::OBJECT) {
		      // This is a no-op; see test_goto_frame.swf
		  } else if (env.top(0).get_type() == as_value::NUMBER) {
		      // Frame numbers appear to be 0-based!  @@ Verify.
		      int frame_number = int(env.top(0).to_number());
		      target->goto_frame(frame_number);
		      success = true;
		  }
		  
		  if (success) {
		      target->set_play_state(state);
		  }
		  
		  env.drop(1);  
		  break;
	      }	      

	      default:
			log_error("Missing handler for action %d", action_id);
		  break;
		  
	    }
#endif
	}
#endif // } // End of kept-for reference block

    }
    
    env.set_target(original_target);
}


};


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
