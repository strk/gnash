//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#include <string>
#include <sstream>
#include <map>
#include <boost/algorithm/string/case_conv.hpp>

#include "smart_ptr.h" // GNASH_USE_GC
#include "VM.h"
#include "rc.h"
#include "debugger.h"
#include "log.h"
#include "as_value.h"
#include "as_environment.h"
#include "SWF.h"
#include "ASHandlers.h"
#include "movie_root.h"

namespace {
gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
}

using namespace std;
using namespace gnash::SWF;

namespace gnash
{

const char *as_arg_strs[] = {
    "ARG_NONE",
    "ARG_STR",
    "ARG_HEX",
    "ARG_U8",
    "ARG_U16",
    "ARG_S16",
    "ARG_PUSH_DATA",
    "ARG_DECL_DICT",
    "ARG_FUNCTION2"
};
const char *state_strs[] = { "rw", "w", "r" };

Debugger::Debugger()
    : _enabled(false), _tracing(false), _state(NONE), _skipb(0), _env(0), _pc(0)
{
//    GNASH_REPORT_FUNCTION;
}

Debugger::~Debugger()
{
//    GNASH_REPORT_FUNCTION;
}

Debugger&
Debugger::getDefaultInstance()
{
	static Debugger dbg;
	return dbg;
}

void
Debugger::usage()
{
//    GNASH_REPORT_FUNCTION;
    cerr << "Gnash Debugger" << endl;
    cerr << "\t? - help" << endl;
    cerr << "\tq - Quit" << endl;
    cerr << "\tt - Toggle Trace Mode" << endl;
    cerr << "\tc - Continue" << endl;
    cerr << "\td - Dissasemble current line" << endl;
    // info commands
    cerr << "\ti i - Dump Movie Info" << endl;
    cerr << "\ti f - Dump Stack Frame" << endl;
    cerr << "\ti s - Dump symbols" << endl;
    cerr << "\ti g - Dump Global Regs" << endl;
    cerr << "\ti r - Dump Local Regs" << endl;
    cerr << "\ti l - Dump Local Variables" << endl;
    cerr << "\ti w - Dump watch points" << endl;
    cerr << "\ti b - Dump break points" << endl;
    cerr << "\ti c - Dump Function Call Stack" << endl;
    // break and watch points
    cerr << "\tw name [r:w:b] - set variable watchpoint" << endl;
    cerr << "\tw name d - delete variable watchpoint" << endl;
    cerr << "\tb name [t:f]- enable/disable function break point" << endl;
    cerr << "\tb name d - delete function break point" << endl;
    // change data
    cerr << "\tset var [name] [value] - set a local variable" << endl;
    cerr << "\tset stack [index] [value] - set a stack entry" << endl;
    cerr << "\tset reg [index] [value] - set a local register" << endl;
    cerr << "\tset global [index] [value] - set a global register" << endl;
}

void
Debugger::console()
{
//    GNASH_REPORT_FUNCTION;
    console(*_env);
}

// Open a user console for debugging commands. The abbreviations and
// commands are roughly based on GDBs.
void
Debugger::console(as_environment &env)
{
//    GNASH_REPORT_FUNCTION;

    // If the debugger isn't enabled, there is nothing to do.
    if (!this->isEnabled()) {
 	return;
    }

    if (!_env) {
	cerr << "WARNING: environment not set yet";
	cerr << "\nOnly watch point commands will work untill after you continue." << endl;
    }
//     if (this->isContinuing()) {
//  	this->hitBreak();
//     } else {
    string action;
    string var, val, sstate;
    int index;
    Debugger::watch_state_e wstate;
    bool keep_going = true;

    log_debug (_("Debugger enabled >> "));
    while (keep_going) {
	cerr << "gnashdbg> ";
	cin >> action;
	switch (action[0]) {
	    // Quit Gnash.
	  case 'Q':
	  case 'q':
	      exit(EXIT_SUCCESS);
	      break;
	      // Continue executing.
	  case 'c':
	      this->go(10);
	      keep_going = false;
	      break;
	      // Change the value of a variable on the stack
	  case 's':
	      if (action == "set") {
		  cin >> var;
		  as_value asval;
		  switch(var[0]) {
		        // change a parameter on the stack
		    case 's':
			cin >> index >> val;
			asval.set_string(val);
			this->changeStackValue(index, asval);
			break;
			// change a local variable
		    case 'v':
			cin >> var >> val;
			asval.set_string(val);
			this->changeLocalVariable(var, asval);
			break;
			// change a local register
		    case 'r':
			cin >> index >> val;
			asval.set_string(val);
			this->changeLocalRegister(index, asval);
			break;
			// change a global register
		    case 'g':
			cin >> index >> val;
			asval.set_string(val);
			this->changeGlobalRegister(index, asval);
			break;
		    default:
			break;
		  }
	      }
	      break;
	      // Informational commands.
	  case 'i':
	      cin >> var;
	      switch (var[0]) {
		case 'd':
		    this->disassemble();
		  break;
		case 'i':
		    this->dumpMovieInfo();
		    break;
		case 'b':
		    this->dumpBreakPoints();
		    break;
		case 'w':
		    this->dumpWatchPoints();
		    break;
		case 'r':
		    this->dumpLocalRegisters(env);
		    break;
		case 'g':
		    this->dumpGlobalRegisters(env);
		    break;
		case 'l':
		    this->dumpLocalVariables(env);
		    break;
		case 'f':
		    this->dumpStackFrame(env);
		    break;
		case 's':
		    this->dumpSymbols();
		    break;
		case 'c':
		    this->callStackDump();
		    break;
	      };
	      break;
	      // Tracing mode. This prints a disasembly every time
	      // Gnash stops at a watch or break point.
	  case 't':
	      if (this->isTracing()) {
		  this->traceMode(false);
	      } else {
		  this->traceMode(true);
	      }
	      break;
	      // Print a help screen
	  case '?':
	      this->usage();
	      break;
	      // Set a watchpoint
	  case 'b':
	      cin >> var >> sstate;
	      switch (sstate[0]) {
		case 't':
		    this->setBreakPoint(var, true);
		    break;
		case 'f':
		    this->setBreakPoint(var, false);
		    break;
		case 'd':
		    this->removeBreakPoint(var);
		    break;
	      };
	      break;
	  case 'w':
	      cin >> var >> sstate;
	      switch (sstate[0]) {
		  // Break on reads
		case 'r':
		    wstate = Debugger::READS;
		    break;
		    // Break on writes
		case 'w':
		    wstate = Debugger::WRITES;
		    break;
		    // Break on any accesses
		case 'b':
		default:
		    wstate = Debugger::BOTH;
		    break;
	      };
	      // Delete a watch point
	      if (sstate[0] == 'd') {
		  this->removeWatchPoint(var);
	      } else {
		  this->setWatchPoint(var, wstate);
	      }
	      sstate.erase();
	      break;
	  default:
	      break;
	};
    }
}

void
Debugger::callStackDump()
{
//    GNASH_REPORT_FUNCTION;
    vector<string>::const_iterator it;
    for (it=_callstack.begin(); it!=_callstack.end(); it++) {
	string str = *it;
	void *addr = this->lookupSymbol(str);
	cerr << "\t=> " << *it << "() <" << addr << ">" << endl;
    }
}

void
Debugger::dumpMovieInfo()
{
//    GNASH_REPORT_FUNCTION;
    if (VM::isInitialized()) {
	VM& vm = VM::get();
	movie_root &mr = vm.getRoot();
	int x, y, buttons;
	mr.get_mouse_state(x, y, buttons);

	cerr << "Movie is Flash v" << vm.getSWFVersion() << endl;
	cerr << "Mouse coordinates are: X=" << x << ", Y=" << y << endl;
	reinterpret_cast<as_object *>(vm.getGlobal())->dump_members();
    }
}

void
Debugger::disassemble()
{
//    GNASH_REPORT_FUNCTION;
    this->disassemble(_pc);
}

void
Debugger::disassemble(const unsigned char *data)
{
//    GNASH_REPORT_FUNCTION;
    ArgumentType fmt = ARG_HEX;
    ActionType action_id = static_cast<ActionType>(data[0]);
    int val = 0;
    string str;
    unsigned char num[10];
    memset(num, 0, 10);

    const gnash::SWF::SWFHandlers& ash = gnash::SWF::SWFHandlers::instance();

    if (_pc == 0) {
    }

    // Show instruction.
    if (action_id > ash.lastType()) {
	cerr << "WARNING: <unknown>[0x" << action_id  << "]" << endl;
    } else {
	if (ash[action_id].getName().size() > 0) {
	    cerr << "Action: " << (void *)action_id << ": " << ash[action_id].getName().c_str() << endl;
	} else {
	    cerr << "Action: " << (void *)action_id << ": " << "WARNING: unknown ID" << endl;
	}
	fmt = ash[action_id].getArgFormat();
    }

    // If we get a ActionPushData opcode, get the parameters
    if (action_id & 0x80) {
	int length = data[1] | (data[2] << 8);
	cerr << "\tArg format is: " << as_arg_strs[fmt] << " Length is: " << length << endl;
	switch (fmt) {
	  case ARG_NONE:
	      log_error (_("No format flag"));
	      break;
	  case ARG_STR:
	      if ((length == 1) && (data[3] == 0)) {
		  str = "null";
	      } else {
		  for (int i = 0; i < length; i++) {
		      if (data[3 + i] != 0) {
			  str += data[3 + i];
		      } else {
			  break;
		      }
		  }
	      }
	      cerr << "Got string (" << length << " bytes): " << "\"" << str << "\"" << endl;
	      break;
	  case ARG_HEX:
              cerr << hexify((const unsigned char *)&data[3], length, false) << endl;
	      break;
	  case ARG_U8:
	      val = data[3];
//	      cerr << "FIXME: Got u8: " << val << endl;
	      break;
	  case ARG_U16:
	      val = data[3] | (data[4] << 8);
//	      cerr << "FIXME: Got u16: " << val << endl;
	      break;
	  case ARG_S16:
	      val = data[3] | (data[4] << 8);
	      if (val & 0x8000) {
		  val |= ~0x7FFF;	// sign-extend
	      }
//	      cerr << "FIXME: Got s16: " << val << endl;
	      break;
	  case ARG_PUSH_DATA:
	      break;
	  case ARG_DECL_DICT:
	      break;
	  case ARG_FUNCTION2:
	      break;
	  default:
	      log_error (_("No format flag"));
	      break;
	} // end of switch(fmt)
    }
}

void
Debugger::setBreakPoint(const std::string &func, bool enabled)
{
//    GNASH_REPORT_FUNCTION;
    _breakpoints[func] = enabled;
}

void
Debugger::removeBreakPoint(const std::string &func)
{
//    GNASH_REPORT_FUNCTION;
    string name;
    std::map<std::string, bool>::const_iterator it;

    it = _breakpoints.find(func);
    if (it != _breakpoints.end()) {
	_breakpoints.erase(func);
    }
}

void
Debugger::dumpBreakPoints()
{
//    GNASH_REPORT_FUNCTION;
    string name;
    bool val;
    std::map<std::string, bool>::const_iterator it;

    int index = 0;
    for (it=_breakpoints.begin(); it != _breakpoints.end(); it++) {
	name = it->first;
	val = it->second;
	if (name.size()) {
	    string str = (val) ? " is enabled" : " is disabled";
	    cerr << "\tbreak #" << index++ << ": " << name << str << endl;
	}
    }
}

bool
Debugger::matchBreakPoint(const std::string &func, bool state)
{
//    GNASH_REPORT_FUNCTION;
    std::map<std::string, bool>::const_iterator it;
    it =_breakpoints.find(func);
    if (it == _breakpoints.end()) {
//	log_debug ("No Match for variable \"%s\"", var);
 	return false;
    } else {
	if (state == _breakpoints[func]) {
//	    log_debug ("Matched for Function \"%s\"", func);
	    this->console();
	    return true;
	}
	return false;
    }
}

void
Debugger::setWatchPoint(const std::string &var, watch_state_e state)
{
//    GNASH_REPORT_FUNCTION;
    _watchpoints[var] = state;
    log_debug (_("Setting watchpoint for variable: \"%s\""), var.c_str());
}

void
Debugger::removeWatchPoint(const std::string &var)
{
//    GNASH_REPORT_FUNCTION;
    string name;
    std::map<std::string, watch_state_e>::const_iterator it;

    it = _watchpoints.find(var);
    if (it != _watchpoints.end()) {
	_watchpoints.erase(var);
    }
}

void
Debugger::dumpWatchPoints()
{
//    GNASH_REPORT_FUNCTION;
    string name;
    watch_state_e state;
    int index = 0;
    std::map<std::string, watch_state_e>::const_iterator it;

    for (it=_watchpoints.begin(); it != _watchpoints.end(); it++) {
	name = it->first;
	state = it->second;
	index++;
	if (name.size()) {
	    cerr << "\twatch #" << index << ": " << name
		 << " \"" << state_strs[state] << "\"" << endl;
	}
    }
}

bool
Debugger::matchWatchPoint(const std::string &var, watch_state_e state)
{
//    GNASH_REPORT_FUNCTION;
    std::map<std::string, watch_state_e>::const_iterator it;
    it =_watchpoints.find(var);
    if (it == _watchpoints.end()) {
//	log_debug ("No Match for variable \"%s\"", var);
 	return false;
    } else {
	if (state == _watchpoints[var]) {
	    log_debug (_("Matched for variable \"%s\": \"%s\""), var.c_str(),
		       state_strs[state]);
	    this->console();
	    return true;
	}
	return false;
    }
}

// These functions manipulate the environment stack
void
Debugger::dumpStackFrame()
{
//    GNASH_REPORT_FUNCTION;
    if (_env == 0) {
	log_error (_("WARNING: environment not set in %s"), __PRETTY_FUNCTION__);
	return;
    }
    this->dumpStackFrame(*_env);
}

// Change the value of a parameter on the stack
void
Debugger::changeStackValue(unsigned int index, as_value &val)
{
//    GNASH_REPORT_FUNCTION;
    changeStackValue(*_env, index, val);
}

void
Debugger::changeStackValue(as_environment &env, unsigned int index, as_value &val)
{
//    GNASH_REPORT_FUNCTION;
    if (!_env) {
	log_error (_("WARNING: environment not set in %s"), __PRETTY_FUNCTION__);
	return;
    }
    if (env.stack_size()) {
	env.bottom(index) = val;
    }
}

void
Debugger::dumpStackFrame(as_environment &env)
{
//    GNASH_REPORT_FUNCTION;
    if (!_env) {
	log_error (_("WARNING: environment not set in %s"), __PRETTY_FUNCTION__);
	return;
    }
    if (env.stack_size()) {
        log_debug (_("Stack Dump of: %p"), (void *)&env);
        for (unsigned int i=0, n=env.stack_size(); i<n; i++) {
	    // FIXME, shouldn't these go to the log as well as to cerr?
            cerr << "\t" << i << ": ";
	    as_value val = env.bottom(i);
// FIXME: we want to print the name of the function
//  	    if (val.is_function()) {
// //		cerr << val.get_symbol_handle() << endl;
// 		string name = this->lookupSymbol(val.to_object(getGlobal(fn)));
// 		if (name.size()) {
// 		    cerr << name << " ";
// 		}
// 	    }
        cerr << env.bottom(i);

	    if (val.is_object()) {
		boost::intrusive_ptr<as_object> o = val.to_object(getGlobal(env));
		string name = lookupSymbol(o.get());
		if (name.size()) {
		    cerr << " \"" << name << "\"";
		}
#ifndef GNASH_USE_GC
		cerr << " has #" << o->get_ref_count() << " references";
#endif
	    }
	    cerr << endl;
	}
    }
    else {
	log_debug (_("Stack Dump of 0x%p: empty"), (void *)&env);
    }
}

void
Debugger::dumpLocalRegisters()
{
//    GNASH_REPORT_FUNCTION;
    this->dumpLocalRegisters(*_env);
}

void
Debugger::dumpLocalRegisters(as_environment &env)
{
//    GNASH_REPORT_FUNCTION;
	env.dump_local_registers(cerr);
}

void
Debugger::dumpGlobalRegisters()
{
//    GNASH_REPORT_FUNCTION;
    this->dumpGlobalRegisters(*_env);
}

void
Debugger::dumpGlobalRegisters(as_environment &env)
{
//    GNASH_REPORT_FUNCTION;
    if (!_env) {
	log_error (_("WARNING: environment not set in %s"), __PRETTY_FUNCTION__);
	return;
    }
    std::string registers;
    stringstream ss;
    log_debug (_("Global Registers Dump:"));
    for (unsigned int i=0; i<4; ++i) {
	ss << "\treg #" << i << ": \"";
	ss << env.global_register(i) << "\"" << endl;
    }
    cerr << ss.str().c_str() << endl;
}

    // Change the value of a local variable
void
Debugger::changeLocalVariable(const std::string &var, as_value &val)
{
//    GNASH_REPORT_FUNCTION;
    changeLocalVariable(*_env, var, val);
}

void
Debugger::changeLocalVariable(as_environment &env, const std::string &var, as_value &val)
{
//    GNASH_REPORT_FUNCTION;
    env.set_local(var, val);
}

// Change the value of a local variable
void
Debugger::changeLocalRegister(unsigned index, as_value &val)
{
//    GNASH_REPORT_FUNCTION;
    changeLocalRegister(*_env, index, val);
}

void
Debugger::changeLocalRegister(as_environment &env, unsigned index, as_value &val)
{
//    GNASH_REPORT_FUNCTION;
	env.set_local_register(index, val);
}

// Change the value of a global variable
void
Debugger::changeGlobalRegister(unsigned index, as_value &val)
{
//    GNASH_REPORT_FUNCTION;
    this->changeLocalRegister(*_env, index, val);
}

void
Debugger::changeGlobalRegister(as_environment &env, unsigned index, as_value &val)
{
//    GNASH_REPORT_FUNCTION;
    env.set_global_register(index, val);
}

void
Debugger::dumpLocalVariables()
{
//    GNASH_REPORT_FUNCTION;
    this->dumpLocalVariables(*_env);
}

void
Debugger::dumpLocalVariables(as_environment &env)
{
	env.dump_local_variables(cerr);
}

/// Get the address associated with a name
void *
Debugger::lookupSymbol(std::string &name)
{
//    GNASH_REPORT_FUNCTION;
    if (_symbols.size()) {
	VM& vm = VM::get(); // cache this ?
	std::string namei = name;
	std::map<void *, std::string>::const_iterator it;
	for (it=_symbols.begin(); it != _symbols.end(); it++) {
	    if (it->second == namei) {
//		log_debug ("Found symbol %s at address %p", namei.c_str(),
//			   it->first);
		return it->first;
	    }
	}
    }
    return NULL;
}

void
Debugger::addSymbol(void *ptr, std::string name)
{
//    GNASH_REPORT_FUNCTION;
    VM& vm = VM::get(); // cache this ?
    std::string namei = name;
    if (namei.size() > 1)
    {
//	log_debug ("Adding symbol %s at address: %p", namei, ptr);
	_symbols[ptr] = namei;
    }
}

/// Get the name associated with an address
std::string
Debugger::lookupSymbol(void *ptr)
{
//    GNASH_REPORT_FUNCTION;

    string str;
    if (_symbols.size()) {
	std::map<void *, std::string>::const_iterator it;
	it = _symbols.find(ptr);
//	dbglogfile.setStamp(false);
	if (it != _symbols.end()) {
//	    log_debug ("Found symbol %s at address: %p", it->second.c_str(), ptr);
	    str = it->second;
// 	} else {
// 	    log_debug ("No symbol found for address %p", ptr);
	}
    }
//    dbglogfile.setStamp(false);
    return str;
}

void
Debugger::dumpSymbols()
{
//    GNASH_REPORT_FUNCTION;
    int index = 0;
    std::map<void *, std::string>::const_iterator it;

    for (it=_symbols.begin(); it != _symbols.end(); it++) {
	string name = it->second;
	void *addr = it->first;
	if (name.size()) {
	    cerr << "\tsym #" << index << ": " << name << " <" << addr << ">" << endl;
	}
	index++;
    }
}

} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
