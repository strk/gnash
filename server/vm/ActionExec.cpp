// ActionExec.cpp:  ActionScript execution, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

/* $Id: ActionExec.cpp,v 1.57 2007/10/09 08:12:26 strk Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ActionExec.h"
#include "action_buffer.h"
#include "swf_function.h"
#include "log.h"
#include "VM.h"
#include "GnashException.h"

#include "swf.h"
#include "ASHandlers.h"
#include "as_environment.h"
#include "debugger.h"
#include "WallClockTimer.h"

#include <typeinfo>
#include <boost/algorithm/string/case_conv.hpp>

#include <sstream>
#include <string>

#ifndef DEBUG_STACK
// temporarly disabled as will produce lots of output with -v
// we'd need another switch maybe, as -va does also produce
// too much information for my tastes. I really want just
// to see how stack changes while executing actions...
// --strk Fri Jun 30 02:28:46 CEST 2006
#define DEBUG_STACK 1
#endif

using namespace gnash;
using namespace SWF;
using std::string;
using std::endl;
using std::stringstream;


namespace gnash {

//static LogFile& dbglogfile = LogFile::getDefaultInstance();
#ifdef USE_DEBUGGER
static Debugger& debugger = Debugger::getDefaultInstance();
#endif

// External interface (to be moved under swf/ASHandlers)
fscommand_callback s_fscommand_handler = NULL;
void	register_fscommand_callback(fscommand_callback handler)
{
    s_fscommand_handler = handler;
}

ActionExec::ActionExec(const swf_function& func, as_environment& newEnv, as_value* nRetVal, as_object* this_ptr)
	:
	with_stack(),
	_scopeStack(func.getScopeStack()),
	// See comment in header
	_with_stack_limit(7),
	_function_var(func.isFunction2() ? 2 : 1),
	_func(&func),
	_this_ptr(this_ptr),
	_initial_stack_size(0),
	_initialCallStackDepth(0),
	_original_target(0),
	mTryList(),
	mReturning(false),
	_abortOnUnload(false),
	code(func.getActionBuffer()),
	pc(func.getStartPC()),
	stop_pc(pc+func.getLength()),
	next_pc(pc),
	env(newEnv),
	retval(nRetVal)
{
	//GNASH_REPORT_FUNCTION;

	// See comment in header
	if ( env.get_version() > 5 ) {
	    _with_stack_limit = 15;
	}

	// SWF version 6 and higher pushes the activation object to the scope stack
	if ( env.get_version() > 5 )
	{
		// We assume that the swf_function () operator already initialized its environment
		// so that it's activation object is now in the top element of the CallFrame stack
		//
		as_environment::CallFrame& topFrame = newEnv.topCallFrame();
		assert(topFrame.func == &func);
		_scopeStack.push_back(topFrame.locals);
	}
}

ActionExec::ActionExec(const action_buffer& abuf, as_environment& newEnv, bool abortOnUnloaded)
	:
	with_stack(),
	_scopeStack(), // TODO: initialize the scope stack somehow
	_with_stack_limit(7),
	_function_var(0),
	_func(NULL),
	_initial_stack_size(0),
	_initialCallStackDepth(0),
	_original_target(0),
	mTryList(),
	mReturning(false),
	_abortOnUnload(abortOnUnloaded),
	code(abuf),
	pc(0),
	stop_pc(code.size()),
	next_pc(0),
	env(newEnv),
	retval(0)
{
	//GNASH_REPORT_FUNCTION;

	/// See comment in header
	if ( env.get_version() > 5 ) {
	    _with_stack_limit = 15;
	}
}

void
ActionExec::operator() ()
{

#if 0
    // Check the time
    if (periodic_events.expired()) {
	periodic_events.poll_event_handlers(&env);
    }
#endif

    // Do not execute if scripts are disabled
    if ( VM::get().getRoot().scriptsDisabled() ) return;

    static const SWFHandlers& ash = SWFHandlers::instance();
		
    _original_target = env.get_target();

    _initial_stack_size = env.stack_size();

    _initialCallStackDepth = env.callStackDepth();

#if DEBUG_STACK
	IF_VERBOSE_ACTION (
        	log_action(_("at ActionExec operator() start, pc=" SIZET_FMT
		           ", stop_pc=" SIZET_FMT ", code.size=" SIZET_FMT
			   "."),
			    pc, stop_pc, code.size());
		stringstream ss;
		env.dump_stack(ss);
		env.dump_global_registers(ss);
		env.dump_local_registers(ss);
		env.dump_local_variables(ss);
		log_action("%s", ss.str().c_str());
	);
#endif

	// TODO: specify in the .gnashrc !!
	static const size_t maxBranchCount = 65536; // what's enough ?

	uint32_t timeLimit = getScriptTimeout();
	WallClockTimer timer;

	size_t branchCount = 0;
	try {
	while (1) // We might not stop at stop_pc, if we are trying.
	{
		if (!(pc < stop_pc))
		{
			// Handle try/catch/finally blocks.
			if (mTryList.empty())
				break; // No try block.
			// If we are in a try block, check to see if we have thrown.
			tryBlock& t = mTryList.back();
			if (t.mState == tryBlock::TRY_TRY)
			{
				if (env.stack_size() && env.top(0).is_exception()) // We have an exception. Catch.
				{
					as_value exc = env.pop();
					pc = t.mCatchOffset;
					// Save the exception to the requested place.
					exc.unflag_exception();
					if (t.mNamed)
						setLocalVariable(t.mName, exc);
					else
					{
						if (isFunction2() && t.mReg < env.num_local_registers())
						{
							env.local_register(t.mReg) = exc;
						}
						else if (t.mReg < 4)
						{
							env.global_register(t.mReg) = exc;
						}
					}

					// Set a new stop.
					stop_pc = t.mFinallyOffset;
					t.mState = tryBlock::TRY_CATCH;
				}
				else // No exception. Finally.
				{
					pc = t.mFinallyOffset;
					stop_pc = t.mAfterTriedOffset;
					t.mState = tryBlock::TRY_FINALLY;
				}
			}
			else if (t.mState == tryBlock::TRY_CATCH) // We've caught. Finally.
			{
				// Here's a fine mess. We've thrown, but we still need to
				// go to finally.

				if (env.stack_size() && env.top(0).is_exception())
				{
					// If we set a variable, erase it.
					if (t.mNamed)
						delVariable(t.mName);
					// Save this for 'finally'.
					t.mThrownFromCatch = env.pop();
					pc = t.mFinallyOffset;
					stop_pc = t.mAfterTriedOffset;
					t.mState = tryBlock::TRY_FINALLY;
				}
				else // No exception. Finally.
				{
					pc = t.mFinallyOffset;
					stop_pc = t.mAfterTriedOffset;
					t.mState = tryBlock::TRY_FINALLY;
				}
			}
			else // TRY_FINALLY
			{
				// No matter how we reached this, this try block is done.
				tryBlock ts = t;
				mTryList.pop_back();

				// If there is an exception, we're throwing from finally.
				if (env.stack_size() && env.top(0).is_exception())
				{
					continue; // Leaving it does right.
				}
				else
				{
					// If we have an exception from catch and no finally
					// block, throw it.
					if (ts.mThrownFromCatch.is_exception() &&
						ts.mFinallyOffset == ts.mSavedEndOffset)
					{
						env.push(ts.mThrownFromCatch);
						if (retval)
						{
							*retval = ts.mThrownFromCatch;
						}
						continue;
					}
					else
					{
						pc = ts.mAfterTriedOffset;
						stop_pc = ts.mSavedEndOffset;
						if (mReturning)
						{
							mReturning = false;
							break;
						}
					}
				}
			}
			continue; // Walk up the try chain if necessary.
		} // end of try checking.

	    // Cleanup any expired "with" blocks.
	    while ( ! with_stack.empty() && pc >= with_stack.back().end_pc() ) {
		// Drop last stack element
		assert(with_stack.back().object() == _scopeStack.back().get());
		with_stack.pop_back();
		_scopeStack.pop_back(); // hopefully nothing gets after the 'with' stack.
	    }

	// Get the opcode.
	uint8_t action_id = code[pc];
	size_t oldPc = pc;

	IF_VERBOSE_ACTION (
		// FIXME, avoid direct dbglogfile access, use log_action
		log_action("PC:" SIZET_FMT " - EX: %s", pc, code.disasm(pc).c_str());
	);

	// Set default next_pc offset, control flow action handlers
	// will be able to reset it.
	if ((action_id & 0x80) == 0) {
		// action with no extra data
		next_pc = pc+1;
	} else {
		// action with extra data
		uint16_t length = uint16_t(code.read_int16(pc+1));
		next_pc = pc + length + 3;
		if ( next_pc > stop_pc )
		{
			IF_VERBOSE_MALFORMED_SWF(
			log_swferror(_("Length %u (%d) of action tag"
			  " id %u at pc " SIZET_FMT
		          " overflows actions buffer size " SIZET_FMT),
			  length, (int)length, (unsigned)action_id, pc,
			  stop_pc);
			);
			//throw ActionException(ss.str());;
			// Give this action handler a chance anyway.
			// Maybe it will be able to do something about
			// this anyway..
		}
	}

	// Do we still need this ?
	if ( action_id == SWF::ACTION_END ) {
		// this would turn into an assertion (next_pc==stop_pc)
		//		log_msg(_("At ACTION_END next_pc=" SIZET_FMT
		//		  ", stop_pc=" SIZET_FMT), next_pc, stop_pc);
		break;
	}

	ash.execute((action_type)action_id, *this);

#if 1 // See bugs: #20974, #21069, #20996.

#if 0
	// curveball.swf and feed.swf break with this
	character* guardedChar = env.get_original_target(); // watch out : _original_target is not necessarely the same
#else
	// curveball.swf and feed.swf suggest that it is the *current* target,
	// not the *original* one that matters.
	character* guardedChar = env.get_target();
#endif

	if ( _abortOnUnload && guardedChar->isUnloaded() )
		// action_execution_order_test8.c shows that the opcode guard is not SWF version based (TODO: automate it!)
		// && VM::get().getSWFVersion() > 5 
	{
		std::stringstream ss;
		ss << "Target of action_buffer (" << guardedChar->getTarget() 
			<< " of type " << typeName(*guardedChar) << ") unloaded by execution of opcode: " << std::endl;
		dumpActions(pc, next_pc, ss);
		ss << "Discarding " << stop_pc-next_pc
			<< " bytes of remaining opcodes: " << std::endl;
		dumpActions(next_pc, stop_pc, ss);
		log_debug("%s", ss.str().c_str());
		break;
	}
#endif


#ifdef USE_DEBUGGER
 	debugger.setFramePointer(code.getFramePointer(pc));
 	debugger.setEnvStack(&env);
	if (debugger.isTracing()) {
	    debugger.disassemble();
	}
#endif
	
#if DEBUG_STACK
	IF_VERBOSE_ACTION (
		log_action(_("After execution: PC " SIZET_FMT ", next PC " SIZET_FMT ", stack follows"), pc, next_pc);
		stringstream ss;
		env.dump_stack(ss);
		env.dump_global_registers(ss);
		env.dump_local_registers(ss);
		env.dump_local_variables(ss);
		log_action("%s", ss.str().c_str());
	);
#endif

	// Control flow actions will change the PC (next_pc)
	pc = next_pc;

	// Check for script limits hit. 
	// See: http://www.gnashdev.org/wiki/index.php/ScriptLimits
	// 
#if 0
	// TODO: only check on branch-back ? (would be less aggressive..)
	// WARNING: if the movie is stopped, the wall clock continues to run !
	if ( timeLimit && timer.elapsed() > timeLimit )
	{
		char buf[256];
		snprintf(buf, 255, _("Script exceeded time limit of %u milliseconds."), timeLimit);
		throw ActionLimitException(buf);
	}
#else
	if ( pc <= oldPc )
	{
		if ( ++branchCount > maxBranchCount )
		{
			char buf[256];
			snprintf(buf, 255, _("Loop iterations count exceeded limit of " SIZET_FMT ". Last branch was from pc %d to %d."),
				maxBranchCount, oldPc, pc);
			throw ActionLimitException(buf);
		}
		//log_debug("Branch count: %u", branchCount);
	}
#endif

    }

    }
    catch (ActionLimitException& ex)
    {
	    // Here's were we should pop-up a window to prompt user about
	    // what to do next (abort or not ?)
	    //log_error("Script aborted due to exceeded limit: %s - cleaning up after run", ex.what());
            cleanupAfterRun(true); // we expect inconsistencies here
	    throw;
    }
    // TODO: catch other exceptions ?

    cleanupAfterRun();

}

/*private*/
void
ActionExec::cleanupAfterRun(bool expectInconsistencies)
{
    assert(_original_target);
    env.set_target(_original_target);
    _original_target = NULL;

    // Check the call stack depth to be the same as the one we started with
    // TODO: should this check be switched off based on GNASH_TRUST_SWF_INPUT ?
    size_t currCallStackDepth = env.callStackDepth();
    if ( currCallStackDepth != _initialCallStackDepth )
    {
	if ( currCallStackDepth > _initialCallStackDepth )
	{
		if ( ! expectInconsistencies )
		{
			// TODO: try to produce this error hitting script limits
			log_error(_("Call stack at end of ActionScript execution "
				"(" SIZET_FMT ") exceeds call stack depth at start "
				"of it (" SIZET_FMT ") - limits hit ?"),
				 currCallStackDepth, _initialCallStackDepth);
		}
		size_t diff = currCallStackDepth-_initialCallStackDepth;
		// TODO: implement dropCallFrames(diff) ?
		while (diff--) env.popCallFrame();
		assert(env.callStackDepth() == _initialCallStackDepth);
	}
	else
	{
		// TODO: make this an assertion ?
		log_error(_("Call stack at end of ActionScript execution "
			"(" SIZET_FMT ") less then call stack depth at start "
			"of it (" SIZET_FMT ") - bad bug !"),
			 currCallStackDepth, _initialCallStackDepth);
	}
    }

    // check if the stack was smashed
    if ( _initial_stack_size > env.stack_size() ) {
	log_error(_("Stack smashed (ActionScript compiler bug?)."
		    "Fixing by pushing undefined values to the missing slots, "
		    " but don't expect things to work afterwards"));
	size_t missing = _initial_stack_size - env.stack_size();
	for (size_t i=0; i<missing; ++i) {
	    env.push(as_value());
	}
    } else if ( _initial_stack_size < env.stack_size() ) {
		if ( ! expectInconsistencies )
		{
	// We can argue this would be an "size-optimized" SWF instead...
	IF_VERBOSE_MALFORMED_SWF(
	    log_swferror(_(SIZET_FMT " elements left on the stack after block execution.  "
		    "Cleaning up"), env.stack_size()-_initial_stack_size);
	    );
		}
	env.drop(env.stack_size()-_initial_stack_size);
    }

    //log_debug("After cleanup of ActionExec %p, env %p has stack size of %d and callStackDepth of %d", (void*)this, (void*)&env, env.stack_size(), env.callStackDepth());
}

void
ActionExec::skip_actions(size_t offset)
{
	//pc = next_pc;

	for(size_t i=0; i<offset; ++i) {
#if 1
	    // we need to check at every iteration because
	    // an action can be longer then a single byte
	    if ( next_pc >= stop_pc ) {
		IF_VERBOSE_MALFORMED_SWF (
		log_swferror(_("End of DoAction block hit while skipping "
			  SIZET_FMT " action tags (pc:" SIZET_FMT
			  ", stop_pc:" SIZET_FMT ") "
			  "(WaitForFrame, probably)"), offset, next_pc,
			  stop_pc);
		)
		next_pc = stop_pc;
		return;
	    }
#endif

	    // Get the opcode.
	    uint8_t action_id = code[next_pc];

	    // Set default next_pc offset, control flow action handlers
	    // will be able to reset it.
	    if ((action_id & 0x80) == 0) {
		// action with no extra data
		next_pc++;
	    } else {
		// action with extra data
		int16_t length = code.read_int16(next_pc+1);
		assert( length >= 0 );
		next_pc += length + 3;
	    }

	    //pc = next_pc;
	}
}

bool
ActionExec::pushWithEntry(const with_stack_entry& entry)
{
	// See comment in header about _with_stack_limit
	if (with_stack.size() >= _with_stack_limit)
	{
	    IF_VERBOSE_ASCODING_ERRORS (
	    log_aserror(_("'With' stack depth (" SIZET_FMT ") "
			"exceeds the allowed limit for current SWF "
			"target version (" SIZET_FMT " for version %d)."
			" Don't expect this movie to work with all players."),
			with_stack.size()+1, _with_stack_limit,
			env.get_version());
	    );
	    return false;
	}
	
	with_stack.push_back(entry);
	_scopeStack.push_back(const_cast<as_object*>(entry.object()));
	return true;
}

bool
ActionExec::delVariable(const std::string& name)
{
	VM& vm = VM::get(); // cache this ?
	std::string namei = name;
	if ( vm.getSWFVersion() < 7 ) {
	    boost::to_lower(namei, vm.getLocale());
	}
	
	return env.del_variable_raw(namei, getScopeStack());
}

bool
ActionExec::delObjectMember(as_object& obj, const std::string& name)
{
	VM& vm = VM::get();

	std::string namei = name;

	if ( vm.getSWFVersion() < 7 ) {
	    boost::to_lower(namei, vm.getLocale());
	} 

	std::pair<bool,bool> ret = obj.delProperty(vm.getStringTable().find(namei));
	return ret.second;
}

void
ActionExec::setVariable(const std::string& name, const as_value& val)
{
	VM& vm = VM::get(); // cache this ?
	std::string namei = name;
	if ( vm.getSWFVersion() < 7 ) {
	    boost::to_lower(namei, vm.getLocale());
	}
	
	return env.set_variable(namei, val, getScopeStack());
}

as_value
ActionExec::getVariable(const std::string& name)
{
	VM& vm = VM::get();

	std::string namei = name;
	if ( vm.getSWFVersion() < 7 ) {
	    boost::to_lower(namei, vm.getLocale());
	}
	
	return env.get_variable(namei, getScopeStack());
}

as_value
ActionExec::getVariable(const std::string& name, as_object** target)
{
	VM& vm = VM::get();

	std::string namei = name;
	if ( vm.getSWFVersion() < 7 ) {
	    boost::to_lower(namei, vm.getLocale());
	}
	
	return env.get_variable(namei, getScopeStack(), target);
}

void
ActionExec::setLocalVariable(const std::string& name_, const as_value& val)
{
	VM& vm = VM::get(); // cache this ?

	std::string name = name_;
	if ( vm.getSWFVersion() < 7 ) {
	    boost::to_lower(name, vm.getLocale());
	}

	if ( isFunction() ) {
	    // TODO: set local in the function object?
	    env.set_local(name, val);
	} else {
	    // TODO: set target member  ?
	    //       what about 'with' stack ?
	    env.set_variable(name, val);
	}
}

void
ActionExec::setObjectMember(as_object& obj, const std::string& var, const as_value& val)
{
	VM& vm = VM::get();

	if ( vm.getSWFVersion() < 7 ) {
	    std::string vari = var;
	    boost::to_lower(vari, vm.getLocale());
	    obj.set_member(vm.getStringTable().find(vari), val);
	} else {
	    obj.set_member(vm.getStringTable().find(var), val);
	}
	
}

bool
ActionExec::getObjectMember(as_object& obj, const std::string& var, as_value& val)
{
	VM& vm = VM::get();

	if ( vm.getSWFVersion() < 7 ) {
	    std::string vari = var;
	    boost::to_lower(vari, vm.getLocale());
	    return obj.get_member(vm.getStringTable().find(vari), &val);
	} else {
	    return obj.get_member(vm.getStringTable().find(var), &val);
	}

}

/*private*/
void
ActionExec::fixStackUnderrun(size_t required)
{
	size_t slots_left = env.stack_size() - _initial_stack_size;
	size_t missing = required-slots_left;

	// FIXME, the IF_VERBOSE used to be commented out.  strk, know why?
	IF_VERBOSE_ASCODING_ERRORS(
	log_aserror(_("Stack underrun: " SIZET_FMT " elements required, "
		SIZET_FMT "/" SIZET_FMT " available. "
		"Fixing by inserting " SIZET_FMT " undefined values on the"
		" missing slots."),
		required, _initial_stack_size, env.stack_size(),
		missing);
	);

	env.padStack(_initial_stack_size, missing);
}

as_object*
ActionExec::getTarget()
{
	if ( ! with_stack.empty() )
	{
		//return const_cast<as_object*>(with_stack.back().object());
		return with_stack.back().object();
	}
	else
	{
		return env.get_target();
	}
}

void
ActionExec::pushTryBlock(tryBlock& t)
{
	// The current block should end at the end of the try block.
	t.mSavedEndOffset = stop_pc;
	stop_pc = t.mCatchOffset;

	mTryList.push_back(t);
}

void
ActionExec::pushReturn(const as_value& t)
{
	if (retval)
	{
    	*retval = t;
	}
    mReturning = true;
}

void
ActionExec::dumpActions(size_t from, size_t to, ostream& os)
{
	size_t lpc = from;
	while (lpc < to)
	{
	    // Get the opcode.
	    uint8_t action_id = code[lpc];

	    os << " PC:" << lpc << " - EX: " <<  code.disasm(lpc) << std::endl;

	    // Set default next_pc offset, control flow action handlers
	    // will be able to reset it.
	    if ((action_id & 0x80) == 0) {
		// action with no extra data
		lpc++;
	    } else {
		// action with extra data
		int16_t length = code.read_int16(lpc+1);
		assert( length >= 0 );
		lpc += length + 3;
	    }

	}
}

as_object*
ActionExec::getThisPointer()
{
	return _function_var ? _this_ptr.get() : env.get_original_target(); 
}

uint32_t
ActionExec::getScriptTimeout()
{
	// TODO1: allow specifying this in the .gnashrc file
	// TODO2: possibly use the SWF tag for this
	return 15000;
	//return 2000;
}

} // end of namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
