// ActionExec.cpp:  ActionScript execution, for Gnash.
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
#include "gnashconfig.h" // USE_DEBUGGER
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
//#include "WallClockTimer.h" // will probably be used for scriptTimeout

#include <sstream>
#include <string>
#include <boost/format.hpp>

#ifndef DEBUG_STACK

// temporarly disabled as will produce lots of output with -v
// we'd need another switch maybe, as -va does also produce
// too much information for my tastes. I really want just
// to see how stack changes while executing actions...
// --strk Fri Jun 30 02:28:46 CEST 2006
# define DEBUG_STACK 1

// Max number of stack item to dump. 0 for unlimited.
# define STACK_DUMP_LIMIT 32

// Define to get debugging messages for try / catch
//#define GNASH_DEBUG_TRY 1

#endif


namespace gnash {

//static LogFile& dbglogfile = LogFile::getDefaultInstance();
#ifdef USE_DEBUGGER
static Debugger& debugger = Debugger::getDefaultInstance();
#endif

ActionExec::ActionExec(const swf_function& func, as_environment& newEnv, as_value* nRetVal, as_object* this_ptr)
    :
    _withStack(),
    _scopeStack(func.getScopeStack()),
    // See comment in header
    _withStackLimit(7),
    _func(&func),
    _this_ptr(this_ptr),
    _initialStackSize(0),
    _initialCallStackDepth(0),
    _originalTarget(0),
    _tryList(),
    _returning(false),
    _abortOnUnload(false),
    pc(func.getStartPC()),
    next_pc(pc),
    stop_pc(pc+func.getLength()),
    code(func.getActionBuffer()),
    env(newEnv),
    retval(nRetVal)
{
    assert(stop_pc < code.size());

    // See comment in header
    if ( env.get_version() > 5 ) {
        _withStackLimit = 15;
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
    _withStack(),
    _scopeStack(), // TODO: initialize the scope stack somehow
    _withStackLimit(7),
    _func(NULL),
    _initialStackSize(0),
    _initialCallStackDepth(0),
    _originalTarget(0),
    _tryList(),
    _returning(false),
    _abortOnUnload(abortOnUnloaded),
    pc(0),
    next_pc(0),
    stop_pc(abuf.size()),
    code(abuf),
    env(newEnv),
    retval(0)
{
    //GNASH_REPORT_FUNCTION;

    /// See comment in header
    if ( env.get_version() > 5 ) {
        _withStackLimit = 15;
    }
}

void
ActionExec::operator() ()
{

    // Do not execute if scripts are disabled
    if ( VM::get().getRoot().scriptsDisabled() ) return;

    static const SWF::SWFHandlers& ash = SWF::SWFHandlers::instance();
        
    _originalTarget = env.get_target();

    _initialStackSize = env.stack_size();

    _initialCallStackDepth = env.callStackDepth();

#if DEBUG_STACK
    IF_VERBOSE_ACTION (
            log_action(_("at ActionExec operator() start, pc=%d"
                   ", stop_pc=%d, code.size=%d."),
                pc, stop_pc, code.size());
        std::stringstream ss;
        env.dump_stack(ss, STACK_DUMP_LIMIT);
        env.dump_global_registers(ss);
        env.dump_local_registers(ss);
        env.dump_local_variables(ss);
        log_action("%s", ss.str());
    );
#endif

    // TODO: specify in the .gnashrc !!
    static const size_t maxBranchCount = 65536; // what's enough ?

    //WallClockTimer timer;

    // stop_pc: set to the code boundary at which we should check
    // for exceptions. If there is no exception in a TryBlock, it
    // is set to the end of that block; all the code (including catch
    // and finally) should be executed.
    // If 'try' finds an exception, stop_pc causes execution to stop
    // at 'catch', while we find whether it catches the exception, and
    // at 'finally', where we handle any exceptions thrown in the meantime.
    //
    // The stages of our TryBlock only roughly correspond to the ActionScript
    // code. There may be no catch and/or finally block, but certain operations
    // must still be carried out. 

    size_t branchCount = 0;
    try {
        while (1) // We might not stop at stop_pc, if we are trying.
        {
            if (pc >= stop_pc)
            {
                // No try blocks
                if (_tryList.empty()) {
                
                    // Check for any uncaught exceptions.
                    if (env.stack_size() && env.top(0).is_exception())
                    {
                        log_debug ("Exception on stack, no handlers left.");
                        // Stop execution (for how long?) if an exception
                        // is still on the stack and there is nothing
                        // left to catch it.
                        throw ActionScriptException();
                    }
                    break;
                }

                // If we are in a try block, check to see if we have thrown.
                TryBlock& t = _tryList.back();
                
                if (! processExceptions(t)) break;

                continue;
            }

            // Cleanup any expired "with" blocks.
            while ( ! _withStack.empty() && pc >= _withStack.back().end_pc() )
            {
           
                // Drop last stack element
                assert(_withStack.back().object() == _scopeStack.back().get());
                _withStack.pop_back();
                
                // hopefully nothing gets after the 'with' stack.
                _scopeStack.pop_back();
            }

            // Get the opcode.
            boost::uint8_t action_id = code[pc];
            size_t oldPc = pc;

            IF_VERBOSE_ACTION (
                log_action("PC:%d - EX: %s", pc, code.disasm(pc));
            );

            // Set default next_pc offset, control flow action handlers
            // will be able to reset it.
            if ((action_id & 0x80) == 0)
            {
                // action with no extra data
                next_pc = pc+1;
            }
            else
            {
                // action with extra data
                boost::uint16_t length = boost::uint16_t(code.read_int16(pc + 1));
                next_pc = pc + length + 3;
                if ( next_pc > stop_pc )
                {
                    IF_VERBOSE_MALFORMED_SWF(
                    log_swferror(_("Length %u (%d) of action tag"
                                   " id %u at pc %d"
                                   " overflows actions buffer size %d"),
                      length, static_cast<int>(length),
                      static_cast<unsigned>(action_id), pc,
                      stop_pc);
                    );
                    
                    //throw ActionException(ss.str());;
                    // no way to recover from this actually...
                    // Give this action handler a chance anyway.
                    // Maybe it will be able to do something about
                    // this anyway.
                    break; 
                }
            }

            // Do we still need this ?
            if ( action_id == SWF::ACTION_END )
            {
                break;
            }

            ash.execute(static_cast<SWF::action_type>(action_id), *this);

#if 1 // See bugs: #20974, #21069, #20996.

#if 0
            // curveball.swf and feed.swf break with this
            character* guardedChar = env.get_original_target(); // watch out : _originalTarget is not necessarely the same
#else
            // curveball.swf and feed.swf suggest that it is the *current* target,
            // not the *original* one that matters.
            character* guardedChar = env.get_target();
#endif

            if ( _abortOnUnload && guardedChar && guardedChar->isUnloaded() )
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
                log_debug("%s", ss.str());
                break;
            }
#endif


#ifdef USE_DEBUGGER
            debugger.setFramePointer(code.getFramePointer(pc));
            debugger.setEnvStack(&env);
            if (debugger.isTracing())
            {
                debugger.disassemble();
            }
#endif
        
#if DEBUG_STACK
            IF_VERBOSE_ACTION (
                log_action(_("After execution: PC %d, next PC %d, stack follows"), pc, next_pc);
                std::stringstream ss;
                env.dump_stack(ss, STACK_DUMP_LIMIT);
                env.dump_global_registers(ss);
                env.dump_local_registers(ss);
                env.dump_local_variables(ss);
                log_action("%s", ss.str());
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
                    boost::format fmt(_("Loop iterations count exceeded limit of "
                                        "%d. Last branch was from pc %d to %d."));
                    fmt % maxBranchCount % oldPc % pc;
                    throw ActionLimitException(fmt.str());
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
    catch (ActionScriptException& ex)
    {
        // An unhandled ActionScript exception was thrown.
        // Script execution should stop (for this frame only?)
        cleanupAfterRun(true);
        return;
    }
    // TODO: catch other exceptions ?

    cleanupAfterRun();

}


// Try / catch / finally rules:
//
// The actionscript code looks like this:
// try { }
// catch { }
// finally { };
//
// For functions:
//
// 1. The catch block is only executed when an exception is thrown.
// 2. The finally block is *always* executed, even when there is no exception
//    *and* a return in try!
// 3. Unhandled executions are handled the same.
// 4. If an exception is thrown in a function and not caught within that function,
//    the return value is the exception, unless the 'finally' block has its own
//    return. (In that case, the exception is never handled).

bool
ActionExec::processExceptions(TryBlock& t)
{

    switch (t._tryState)
    {

        case TryBlock::TRY_TRY:
        {
            if (env.stack_size() && env.top(0).is_exception())
            {
#ifdef GNASH_DEBUG_TRY
                as_value ex = env.top(0);
                ex.unflag_exception();
                log_debug("TRY block: Encountered exception (%s). Set PC to catch.", ex);
#endif
                // We have an exception. Don't execute any more of the try
                // block and process exception.
                pc = t._catchOffset;
                t._tryState = TryBlock::TRY_CATCH;
              
                if (!t._hasName)
                {
                    // Used when exceptions are thrown in functions.
                    // Tests in misc-mtasc.all/exception.as
                    as_value ex = env.pop();
                    ex.unflag_exception();
                    
                    env.setRegister(t._registerIndex, ex);
                }
            }
            else
            {
#ifdef GNASH_DEBUG_TRY 
                log_debug("TRY block: No exception, continuing as normal.");
#endif

                // No exception, so the try block should be executed,
                // then finally (even if a function return is in try). There
                // should be an action jump to finally at the end of try; if
                // this is missing, catch must be executed as well.

                // If there is a return in the try block, go straight to 
                // finally.
                if (_returning) pc = t._finallyOffset;
                else stop_pc = t._finallyOffset;

                t._tryState = TryBlock::TRY_FINALLY;
            }
            break;
        }


        case TryBlock::TRY_CATCH:
        {
#ifdef GNASH_DEBUG_TRY
            log_debug("CATCH: TryBlock name = %s", t._name); 
#endif               
            // If we are here, there should have been an exception
            // in 'try'. 
            
            if (env.stack_size() && env.top(0).is_exception())
            {
                // This was thrown in "try". Remove it from
                // the stack and remember it so that
                // further exceptions can be caught.
                t._lastThrow = env.pop();
                as_value ex = t._lastThrow;
                ex.unflag_exception();

#ifdef GNASH_DEBUG_TRY
                log_debug("CATCH block: top of stack is an exception (%s)", ex);
#endif

                if (t._hasName && !t._name.empty())
                {
                    // If name isn't empty, it means we have a catch block.
                    // We should set its argument to the exception value.
                    setLocalVariable(t._name, ex);
                    t._lastThrow = as_value();
#ifdef GNASH_DEBUG_TRY
                    log_debug("CATCH block: encountered exception (%s). "
                              "Assigning to catch arg %d.", ex, t._name);
#endif
                }
            }

            // Go to finally
            stop_pc = t._finallyOffset;
            t._tryState = TryBlock::TRY_FINALLY;
            break;
        }


        case TryBlock::TRY_FINALLY:
        {
            // FINALLY. This may or may not exist, but these actions
            // are carried out anyway.
#ifdef GNASH_DEBUG_TRY
            log_debug("FINALLY: TryBlock name = %s", t._name);                 
#endif
            // If the exception is here, we have thrown in catch.
            if (env.stack_size() && env.top(0).is_exception())
            {
                 
                t._lastThrow = env.pop();
#ifdef GNASH_DEBUG_TRY 
                as_value ex = t._lastThrow;
                ex.unflag_exception();
                log_debug("FINALLY: top of stack is an exception "
                          "again (%s). Replaces any previous "
                          "uncaught exceptions", ex);
#endif

                // Return any exceptions thrown in catch. This
                // can be overridden by a return in finally. 
                // Should these be thrown to the register too
                // if it is a catch-in-register case?
                if (retval) *retval = t._lastThrow;

            }
            stop_pc = t._afterTriedOffset;
            t._tryState = TryBlock::TRY_END;
            break;
        }

        case TryBlock::TRY_END:
        {
            // If there's no exception here, we can execute the
            // rest of the code. If there is, it will be caught
            // by the next TryBlock or stop execution.
            if (env.stack_size() && env.top(0).is_exception())
            {
                // Check for exception handlers straight away
                stop_pc = t._afterTriedOffset;
#ifdef GNASH_DEBUG_TRY
                as_value ex = env.top(0);
                ex.unflag_exception();
                log_debug("END: exception thrown in finally(%s). "
                          "Leaving on the stack", ex);
#endif
                _tryList.pop_back();
                return true;
            }
            else if (t._lastThrow.is_exception())
            {
                // Check for exception handlers straight away
                stop_pc = t._afterTriedOffset;                
#ifdef GNASH_DEBUG_TRY
                as_value ex = t._lastThrow;
                ex.unflag_exception();
                log_debug("END: no new exceptions thrown. Pushing "
                      "uncaught one (%s) back on stack", ex);
#endif                   

                env.push(t._lastThrow);
 

                _tryList.pop_back();
                return true;
            }
#ifdef GNASH_DEBUG_TRY
            log_debug("END: no new exceptions thrown. Continuing");
#endif
            // No uncaught exceptions left in TryBlock:
            // execute rest of code.
            stop_pc = t._savedEndOffset;
            
            // Finished with this TryBlock.
            _tryList.pop_back();
            
            // Will break out of action execution.
            if (_returning) return false;
		    
            break;
        }


    } // end switch
    return true;
}

/*private*/
void
ActionExec::cleanupAfterRun(bool expectInconsistencies)
{
    //assert(_originalTarget); // this execution context might have been started while target had a null target
    env.set_target(_originalTarget);
    _originalTarget = NULL;

    // check if the stack was smashed
    if ( _initialStackSize > env.stack_size() )
    {
#if 0
        log_error(_("Stack smashed (ActionScript compiler bug?)."
                "Fixing by pushing undefined values to the missing slots, "
                " but don't expect things to work afterwards"));
        const size_t missing = _initialStackSize - env.stack_size();

        for (size_t i = 0; i < missing; ++i)
        {
            env.push(as_value());
        }
#else
        log_error(_("Stack smashed (ActionScript compiler bug?)."
                "Taking no action to fix (as expected)."));
#endif
    }
    else if ( _initialStackSize < env.stack_size() )
    {
#if 0
            log_error(_("%d elements left on the stack after block execution.  "
                "Leaving there"), env.stack_size() - _initialStackSize);
#else
	// we need to cleanup after run, or the GC will need to scan
	// the stack as well..
        if ( ! expectInconsistencies )
        {
            // We can argue this would be an "size-optimized" SWF instead...
            IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("%d elements left on the stack after block execution.  "
                "Cleaning up"), env.stack_size() - _initialStackSize);
            );
        }
        env.drop(env.stack_size() - _initialStackSize);
#endif
    }

    // Have movie_root flush any newly pushed actions in higher priority queues
    VM::get().getRoot().flushHigherPriorityActionQueues();

//    log_debug("After cleanup of ActionExec %p, env %p has "
//        "stack size of %d and callStackDepth of %d",
//        (void*)this, (void*)&env, env.stack_size(), env.callStackDepth());
}

void
ActionExec::skip_actions(size_t offset)
{

    for(size_t i=0; i<offset; ++i)
    {
#if 1
        // we need to check at every iteration because
        // an action can be longer then a single byte
        if ( next_pc >= stop_pc )
        {
            IF_VERBOSE_MALFORMED_SWF (
                log_swferror(_("End of DoAction block hit while skipping "
                  "%d action tags (pc:%d, stop_pc:%d) "
                  "(WaitForFrame, probably)"), offset, next_pc,
                  stop_pc);
            )
            next_pc = stop_pc;
            return;
        }
#endif

        // Get the opcode.
        boost::uint8_t action_id = code[next_pc];

        // Set default next_pc offset, control flow action handlers
        // will be able to reset it.
        if ((action_id & 0x80) == 0)
        {
            // action with no extra data
            next_pc++;
        }
        else
        {
            // action with extra data
            boost::int16_t length = code.read_int16(next_pc+1);
            assert( length >= 0 );
            next_pc += length + 3;
        }
    }
}

bool
ActionExec::pushWithEntry(const with_stack_entry& entry)
{
    // See comment in header about _withStackLimit
    if (_withStack.size() >= _withStackLimit)
    {
        IF_VERBOSE_ASCODING_ERRORS (
        log_aserror(_("'With' stack depth (%d) "
            "exceeds the allowed limit for current SWF "
            "target version (%d for version %d)."
            " Don't expect this movie to work with all players."),
            _withStack.size()+1, _withStackLimit,
            env.get_version());
        );
        return false;
    }
    
    _withStack.push_back(entry);
    _scopeStack.push_back(const_cast<as_object*>(entry.object()));
    return true;
}

bool
ActionExec::delVariable(const std::string& name)
{
    return env.del_variable_raw(PROPNAME(name), getScopeStack());
}

bool
ActionExec::delObjectMember(as_object& obj, const std::string& name)
{
    string_table& st = VM::get().getStringTable();
    std::pair<bool,bool> ret = obj.delProperty(st.find(PROPNAME(name)));
    return ret.second;
}

void
ActionExec::setVariable(const std::string& name, const as_value& val)
{
    return env.set_variable(PROPNAME(name), val, getScopeStack());
}

as_value
ActionExec::getVariable(const std::string& name)
{
    return env.get_variable(PROPNAME(name), getScopeStack());
}

as_value
ActionExec::getVariable(const std::string& name, as_object** target)
{
    return env.get_variable(PROPNAME(name), getScopeStack(), target);
}

void
ActionExec::setLocalVariable(const std::string& name, const as_value& val)
{
    if ( isFunction() ) {
        // TODO: set local in the function object?
        env.set_local(PROPNAME(name), val);
    } else {
        // TODO: set target member  ?
        //       what about 'with' stack ?
        env.set_variable(PROPNAME(name), val, getScopeStack());
    }
}

void
ActionExec::setObjectMember(as_object& obj, const std::string& var, const as_value& val)
{
    string_table& st = VM::get().getStringTable();
    obj.set_member(st.find(PROPNAME(var)), val);
}

bool
ActionExec::getObjectMember(as_object& obj, const std::string& var, as_value& val)
{
    string_table& st = VM::get().getStringTable();
    return obj.get_member(st.find(PROPNAME(var)), &val);
}

/*private*/
void
ActionExec::fixStackUnderrun(size_t required)
{
    size_t slots_left = env.stack_size() - _initialStackSize;
    size_t missing = required-slots_left;

    // FIXME, the IF_VERBOSE used to be commented out.  strk, know why?
    IF_VERBOSE_ASCODING_ERRORS(
    log_aserror(_("Stack underrun: %d elements required, "
        "%d/%d available. "
        "Fixing by inserting %d undefined values on the"
        " missing slots."),
        required, _initialStackSize, env.stack_size(),
        missing);
    );

    env.padStack(_initialStackSize, missing);
}

as_object*
ActionExec::getTarget()
{
    if ( ! _withStack.empty() )
    {
        return _withStack.back().object();
    }
    else
    {
        return env.get_target();
    }
}

void
ActionExec::pushTryBlock(TryBlock& t)
{
    // The current block should end at the end of the try block.
    t._savedEndOffset = stop_pc;
    stop_pc = t._catchOffset;

    _tryList.push_back(t);

}

void
ActionExec::pushReturn(const as_value& t)
{
    if (retval)
    {
        *retval = t;
    }
    _returning = true;
}

void
ActionExec::adjustNextPC(int offset)
{
    const int tagPos = offset + static_cast<int>(pc);
    if (tagPos < 0) {
        log_unimpl(_("Jump outside DoAction tag requested (offset %d before tag start)"), -tagPos);
        return;
    }
    next_pc += offset;
}

bool
ActionExec::isFunction2() const
{
    return _func ? _func->isFunction2() : false;
}

void
ActionExec::dumpActions(size_t from, size_t to, std::ostream& os)
{
    size_t lpc = from;
    while (lpc < to)
    {
        // Get the opcode.
        boost::uint8_t action_id = code[lpc];

        os << " PC:" << lpc << " - EX: " <<  code.disasm(lpc) << std::endl;

        // Set default next_pc offset, control flow action handlers
        // will be able to reset it.
        if ((action_id & 0x80) == 0)
        {
            // action with no extra data
            lpc++;
        } else {
            // action with extra data
            boost::int16_t length = code.read_int16(lpc+1);
            assert( length >= 0 );
            lpc += length + 3;
        }

    }
}

as_object*
ActionExec::getThisPointer()
{
    return _func ? _this_ptr.get() : env.get_original_target(); 
}

} // end of namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
