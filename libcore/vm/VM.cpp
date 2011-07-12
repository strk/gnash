// VM.cpp: the Virtual Machine class, for Gnash
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "VM.h"

#include <ostream>
#include <memory>
#include <boost/random.hpp> // for random generator
#include <cstdlib> 
#include <cmath>
#ifdef HAVE_SYS_UTSNAME_H
# include <sys/utsname.h> // For system information
#endif

#include "SharedObject_as.h" // for SharedObjectLibrary
#include "NativeFunction.h"
#include "movie_definition.h"
#include "Movie.h"
#include "movie_root.h"
#include "Global_as.h"
#include "rc.h" 
#include "namedStrings.h"
#include "VirtualClock.h" // for getTime()
#include "GnashNumeric.h"

namespace {
gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
}

namespace gnash {

VM::VM(movie_root& root, VirtualClock& clock)
	:
	_rootMovie(root),
	_global(new Global_as(*this)),
	_swfversion(6),
	_clock(clock),
	_stack(),
    _shLib(new SharedObjectLibrary(*this)),
    _rng(clock.elapsed()),
    _constantPool(0)
{
	NSV::loadStrings(_stringTable);
    _global->registerClasses();
	_clock.restart();
}

VM::~VM()
{
}

void
VM::setSWFVersion(int v) 
{
	_swfversion = v;
}

VM::RNG&
VM::randomNumberGenerator() 
{
	return _rng;
}

const std::string&
VM::getPlayerVersion() const
{
	//From rcfile
	static const std::string version(rcfile.getFlashVersionString());
	return version;
}

std::string
VM::getOSName() const
{

	// The directive in gnashrc must override OS detection.
	if (rcfile.getFlashSystemOS() != "") {
		return rcfile.getFlashSystemOS();
	}
	else {
#ifdef HAVE_SYS_UTSNAME_H
// For Linux- or UNIX-based systems (POSIX 4.4 conformant)

		struct utsname osname;
		std::string tmp;
		
		uname(&osname);
		
		tmp = osname.sysname;
		tmp += " ";
		tmp += osname.release;
		
		return tmp;
#else
// Last resort, hard-coded from compile-time options

		return DEFAULT_FLASH_SYSTEM_OS;
#endif
	}
}

std::string
VM::getSystemLanguage() const
{

	char *loc;
	
	// Try various environment variables. These should
	// be in the standard form "de", "de_DE" or "de_DE.utf8"
	// This should work on most UNIX-like systems.
	// Callers should work out what to do with it.
	// TODO: Other OSs.
	if ((loc = std::getenv("LANG")) || (loc = std::getenv("LANGUAGE")) ||
		(loc = std::getenv("LC_MESSAGES"))) {
        return loc;
	}
	
    return std::string();
}

movie_root&
VM::getRoot() const
{
	return _rootMovie;
}

Global_as*
VM::getGlobal() const
{
	return _global;
}

unsigned long int
VM::getTime() const
{
	return _clock.elapsed();
}

void
VM::markReachableResources() const
{
    std::for_each(_globalRegisters.begin(), _globalRegisters.end(), 
            std::mem_fun_ref(&as_value::setReachable));

	_global->setReachable();

    if (_shLib.get()) _shLib->markReachableResources();

#ifdef ALLOW_GC_RUN_DURING_ACTIONS_EXECUTION
    /// Mark all (including unreachable) stack elements
    for (SafeStack<as_value>::StackSize i=0, n=_stack.totalSize(); i<n; ++i)
    {
        _stack.at(i).setReachable();
    }

    /// Mark call stack 
    for (CallStack::size_type i=0, n=_callStack.size(); i<n; ++i)
    {
        _callStack[i].markReachableResources();
    }

#else
    assert (_callStack.empty());
    assert (_stack.totalSize() == 0);
#endif

}

const as_value*
VM::getRegister(size_t index)
{
    // If there is a call frame and it has registers, the value must be
    // sought there.
    if (!_callStack.empty()) {
        const CallFrame& fr = currentCall();
        if (fr.hasRegisters()) return fr.getLocalRegister(index);
    }

    // Otherwise it can be in the global registers.
    if (index < _globalRegisters.size()) return &_globalRegisters[index];
    return 0;
}

void
VM::setRegister(size_t index, const as_value& val)
{
    // If there is a call frame and it has registers, the value must be
    // set there.
    if (!_callStack.empty()) {
        CallFrame& fr = currentCall();
        if (fr.hasRegisters()) {
            currentCall().setLocalRegister(index, val);
            return;
        }
    }

    // Do nothing if the index is out of bounds.
    if (index < _globalRegisters.size()) _globalRegisters[index] = val;

    IF_VERBOSE_ACTION(
        log_action(_("-------------- global register[%d] = '%s'"),
            index, val);
    );

}

CallFrame&
VM::currentCall()
{
    assert(!_callStack.empty());
    return _callStack.back();
}

CallFrame&
VM::pushCallFrame(UserFunction& func)
{

    // The stack size can be changed by the ScriptLimits
    // tag. There is *no* difference between SWF versions.
    // TODO: override from gnashrc.
    
    // A stack size of 0 is apparently legitimate.
    const boost::uint16_t recursionLimit = getRoot().getRecursionLimit();

    // Don't proceed if local call frames would reach the recursion limit.
    if (_callStack.size() + 1 >= recursionLimit) {

        std::ostringstream ss;
        ss << boost::format(_("Recursion limit reached (%u)")) % recursionLimit;

        // throw something
        throw ActionLimitException(ss.str()); 
    }

    _callStack.push_back(CallFrame(&func));
    return _callStack.back();
}

void 
VM::popCallFrame()
{
    assert(!_callStack.empty());
    _callStack.pop_back();
}

void
VM::registerNative(Global_as::ASFunction fun, unsigned int x, unsigned int y)
{
    assert(fun);
    assert(!_asNativeTable[x][y]);
    _asNativeTable[x][y] = fun;
}

NativeFunction*
VM::getNative(unsigned int x, unsigned int y) const
{
    AsNativeTable::const_iterator row = _asNativeTable.find(x);
    if (row == _asNativeTable.end()) return 0;
    FuncMap::const_iterator col = row->second.find(y);
    if (col == row->second.end()) return 0;
    Global_as::ASFunction fun = col->second;

    NativeFunction* f = new NativeFunction(*_global, fun);
    
    Global_as& gl = *_global;

    as_function* func = getOwnProperty(gl, NSV::CLASS_FUNCTION).to_function();
    if (func) {
        const int flags = as_object::DefaultFlags | PropFlags::onlySWF6Up;
        f->init_member(NSV::PROP_uuPROTOuu, getMember(*func,
                    NSV::PROP_PROTOTYPE), flags);
        f->init_member(NSV::PROP_CONSTRUCTOR, func);
    }
    return f;
}

void
VM::dumpState(std::ostream& out, size_t limit)
{

    // Dump stack:
    size_t si = 0;
    const size_t n = _stack.size();

    if (limit && n > limit) {
        si = n - limit;
        out << "Stack (last " << limit << " of " << n << " items): ";
    }
    else {
        out << "Stack: ";
    }

    for (size_t i = si; i < n; ++i) {
        if (i != si) out << " | ";
        out << '"' << _stack.value(i) << '"';
    }
    out << "\n";

    out << "Global registers: ";
    for (GlobalRegisters::const_iterator it = _globalRegisters.begin(),
            e = _globalRegisters.end(); it != e; ++it) {
        const as_value& v = *it;
        if (v.is_undefined()) continue;
        if (it != _globalRegisters.begin()) out <<  ", ";

        out << (it - _globalRegisters.begin()) << ":" << v;
    }
    out << "\n";

    if ( _constantPool ) {
        out << "Constant pool: " << *_constantPool  << "\n";
    }

    // Now local registers and variables from the call stack.
    if (_callStack.empty()) return;

    out << "Local registers: ";
    for (CallStack::const_iterator it = _callStack.begin(),
            e = _callStack.end(); it != e; ++it) {
        if (it != _callStack.begin()) out << " | ";
        out << *it;
    }
    out << "\n";

}


///////////////////////////////////////////////////////////////////////
//
// Value ops
//
///////////////////////////////////////////////////////////////////////

void
newAdd(as_value& op1, const as_value& op2, const VM& vm)
{
    // We can't change the original value.
    as_value r(op2);

    // The order of the operations is important: op2 is converted to
    // primitive before op1.

    /// It doesn't matter if either of these fail.
	try { convertToPrimitive(r, vm); }
	catch (ActionTypeError& e) {}
	
    try { convertToPrimitive(op1, vm); }
	catch (ActionTypeError& e) {}

#if GNASH_DEBUG
	log_debug(_("(%s + %s) [primitive conversion done]"), op1, r);
#endif

	if (op1.is_string() || r.is_string()) {

		// use string semantic
		const int version = vm.getSWFVersion();
		convertToString(op1, vm);
		op1.set_string(op1.to_string(version) + r.to_string(version));
        return;
	}

    // Otherwise use numeric semantic
    const double num1 = toNumber(op1, vm);
    const double num2 = toNumber(r, vm);
    op1.set_double(num2 + num1); 

}

void
subtract(as_value& op1, const as_value& op2, const VM& vm)
{
	const double num2 = toNumber(op2, vm);
	const double num1 = toNumber(op1, vm);
	op1.set_double(num1 - num2);
}

as_value
newLessThan(const as_value& op1, const as_value& op2, const VM& vm)
{

    as_value operand1(op1);
    as_value operand2(op2);

    try { operand1 = op1.to_primitive(as_value::NUMBER); }
    catch (const ActionTypeError& e) {}
    
    if (operand1.is_object() && !operand1.is_sprite()) {
        return false;
    }

    try { operand2 = op2.to_primitive(as_value::NUMBER); }
    catch (const ActionTypeError& e) {}
    
    if (operand2.is_object() && !operand2.is_sprite()) {
        return false;
    }

    if (operand1.is_string() && operand2.is_string())
    {
        const std::string& s1 = operand1.to_string();
        const std::string& s2 = operand2.to_string();
        if (s1.empty()) return false;
        if (s2.empty()) return true;
        return as_value(s1 < s2);
    }

    const double num1 = toNumber(operand1, vm);
    const double num2 = toNumber(operand2, vm);

    if (isNaN(num1) || isNaN(num2)) {
        return as_value();
    }
    return as_value(num1 < num2);
}

bool
equals(const as_value& a, const as_value& b, const VM& vm)
{
    return a.equals(b, vm.getSWFVersion());
}

bool
toBool(const as_value& v, const VM& vm)
{
    return v.to_bool(vm.getSWFVersion());
}

double
toNumber(const as_value& v, const VM& vm)
{
    return v.to_number(vm.getSWFVersion());
}

as_object*
toObject(const as_value& v, VM& vm)
{
    return v.to_object(vm);
}

boost::int32_t
toInt(const as_value& v, const VM& vm)
{
    const double d = v.to_number(vm.getSWFVersion());

    if (!isFinite(d)) return 0;

    if (d < 0) {   
        return - static_cast<boost::uint32_t>(std::fmod(-d, 4294967296.0));
    }
    
    return static_cast<boost::uint32_t>(std::fmod(d, 4294967296.0));
}

/// Force type to number.
as_value&
convertToNumber(as_value& v, const VM& vm)
{
    v.set_double(v.to_number(vm.getSWFVersion()));
    return v;
}

/// Force type to string.
as_value&
convertToString(as_value& v, const VM& vm)
{
    v.set_string(v.to_string(vm.getSWFVersion()));
    return v;
}

/// Force type to bool.
as_value&
convertToBoolean(as_value& v, const VM& vm)
{
    v.set_bool(v.to_bool(vm.getSWFVersion()));
    return v;
}

as_value&
convertToPrimitive(as_value& v, const VM& vm)
{
    const as_value::AsType t(v.defaultPrimitive(vm.getSWFVersion()));
    v = v.to_primitive(t);
    return v;
}


} // end of namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
