// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

#ifndef __DEBUGGER_H__
#define __DEBUGGER_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_DEBUGGER

#include <string>
#include <map>

#include "as_environment.h"

namespace gnash 
{

class Debugger {
public:
    Debugger();
    ~Debugger();
    // Return the default instance of Debugger
    static Debugger& getDefaultInstance();

    typedef enum {NONE, BREAK, CONTINUE, STOP} debug_state_e;
    typedef enum {BOTH, WRITES, READS} watch_state_e;

    void usage();
//    bool command(const char cmd, as_environment &env);

    void dissasemble(unsigned char *data);
    void dissasemble();
    void setBreakPoint(std::string &var);
    void removeBreakPoint(std::string &var);
    void dumpBreakPoints();

    /// Set a watchpoint of a variable. Gnash stops and generates a
    /// command prompt when there is a match.
    void setWatchPoint(std::string &var, watch_state_e state);
    void removeWatchPoint(std::string &var);
    void dumpWatchPoints();

    /// What to do when we've hit a breakpoint
    void hitBreak() { _skipb--; };
    
    /// Execute the movie, skipping x breaks
    void go(int x) { _skipb = x; };
    bool isContinuing() { return (_skipb > 0) ? true : false; };

    // These functions manipulate the environment stack
    void dumpStackFrame(as_environment &env);
    void dumpStackFrame();
    void dumpLocalRegisters(as_environment &env);
    void dumpLocalRegisters();
    void dumpGlobalRegisters(as_environment &env);
    void dumpGlobalRegisters();
    void dumpLocalVariables(as_environment &env);
    void dumpLocalVariables();
    void setEnvStack(as_environment *x) { _env = x; };
    as_environment *getEnvStack() { return _env; };

    // 
    void setFramePointer(unsigned char *x) { _pc = x; };
    unsigned char *getFramePointer() { return _pc; };

    void console(as_environment &env);
    void console();
    
    /// Are the debugger features enabled ?
    void enabled(bool x) { _enabled = x; };
    bool isEnabled() { return _enabled; };

    /// Print out a dissasembly as each opcode gets executed. This is
    /// equivalant to -va, excpet it uses debugger syntax instead of
    /// the more compact Debug log syntax.
    void traceMode(bool x) { _tracing = x; };
    /// is Trace Mode enabled ?
    bool isTracing() { return _tracing; };

    /// Are there any watchpoints set ?
    int anyWatchPoints() { return _watchpoints.size(); };
    /// Does the variable name match any watchpoints ?
    bool matchWatchPoint(std::string &var, watch_state_e state);
    
    /// Are there any breakpoints set ?
    int anyBreakPoints() { return _breakpoints.size(); };

    // Manipulate a symbol table
    void addSymbol(void *ptr, std::string name);
    void *lookupSymbol(std::string &name);
    std::string lookupSymbol(void *ptr);
    void dumpSymbols();
    
    debug_state_e state() { return _state; };
private:
    bool                        _enabled;
    bool			_tracing;
    debug_state_e               _state;
    int				_skipb;
    as_environment	       *_env;
    unsigned char 	       *_pc;
    std::map<std::string, watch_state_e> _watchpoints;
    std::map<std::string, bool> _breakpoints;
    std::map<void *, std::string> _symbols;
};

} // end of gnash namespace

#endif // end of USE_DEBUGGER

// __DEBUGGER_H__
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
