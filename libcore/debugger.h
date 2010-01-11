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


#ifndef __DEBUGGER_H__
#define __DEBUGGER_H__

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#ifdef USE_DEBUGGER

#include <string>
#include <map>

#include "dsodefs.h"
#include "as_environment.h"

namespace gnash 
{

class Debugger {
public:
    DSOEXPORT Debugger();
    DSOEXPORT ~Debugger();
    // Return the default instance of Debugger
    DSOEXPORT static Debugger& getDefaultInstance();

    typedef enum {NONE, BREAK, CONTINUE, STOP} debug_state_e;
    typedef enum {BOTH, WRITES, READS} watch_state_e;

    void usage();
//    bool command(const char cmd, as_environment &env);

    void dumpMovieInfo();

    void disassemble(const unsigned char *data);
    void disassemble();
    void setBreakPoint(const std::string &var, bool enabled);
    void removeBreakPoint(const std::string &var);
    void dumpBreakPoints();
    /// Does the function name match any breakpoints ?
    bool matchBreakPoint(const std::string &var, bool);

    /// Set a watchpoint of a variable. Gnash stops and generates a
    /// command prompt when there is a match.
    void setWatchPoint(const std::string &var, watch_state_e state);
    void removeWatchPoint(const std::string &var);
    void dumpWatchPoints();

    /// What to do when we've hit a breakpoint
    void hitBreak() { _skipb--; };
    
    /// Execute the movie, skipping x breaks
    void go(int x) { _skipb = x; };
    bool isContinuing() { return (_skipb > 0) ? true : false; };

    // These functions manipulate the environment stack
    void dumpStackFrame(as_environment &env);
    void dumpStackFrame();
    static void dumpLocalRegisters(as_environment &env);
    void dumpLocalRegisters();
    void dumpGlobalRegisters(as_environment &env);
    void dumpGlobalRegisters();
    static void dumpLocalVariables(as_environment &env);
    void dumpLocalVariables();
    void setEnvStack(as_environment *x) { _env = x; };
    as_environment *getEnvStack() { return _env; };

    // 
    void setFramePointer(const unsigned char *x) { _pc = x; };
    const unsigned char *getFramePointer() const { return _pc; };

    DSOEXPORT void console(as_environment &env);
    DSOEXPORT void console();
    
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
    bool matchWatchPoint(const std::string &var, watch_state_e state);
    
    /// Are there any breakpoints set ?
    int anyBreakPoints() { return _breakpoints.size(); };

    // Manipulate a symbol table
    void addSymbol(void *ptr, std::string name);
    void *lookupSymbol(std::string &name);
    std::string lookupSymbol(void *ptr);
    void dumpSymbols();

    // Change the value of a parameter on the stack
    void changeStackValue(unsigned int index, as_value &val);
    void changeStackValue(as_environment &env, unsigned int index, as_value &val);
    
    // Change the value of a local variable
    void changeLocalVariable(const std::string &var, as_value &val);
    void changeLocalVariable(as_environment &env, const std::string &var, as_value &val);
    
    // Change the value of a local variable
    void changeLocalRegister(unsigned index, as_value &val);
    void changeLocalRegister(as_environment &env, unsigned index, as_value &val);
    
    // Change the value of a local variable
    void changeGlobalRegister(unsigned index, as_value &val);
    void changeGlobalRegister(as_environment &env, unsigned index, as_value &val);

    void callStackPush(const std::string &str) { _callstack.push_back(str); };
    void callStackPop() { if ( ! _callstack.empty() ) _callstack.pop_back(); };
    void callStackDump();
    std::string &callStackFrame() { return _callstack.back(); };
    
    debug_state_e state() { return _state; };

private:
    bool                        _enabled;
    bool			_tracing;
    debug_state_e               _state;
    int				_skipb;
    as_environment		*_env;
    const unsigned char		*_pc;
    std::map<std::string, watch_state_e> _watchpoints;
    std::map<std::string, bool> _breakpoints;
    std::map<void *, std::string> _symbols;
    std::vector<std::string>    _callstack;
};

} // end of gnash namespace

#endif // end of USE_DEBUGGER

// __DEBUGGER_H__
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
