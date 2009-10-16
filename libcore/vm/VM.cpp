// VM.cpp: the Virtual Machine class, for Gnash
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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
#include "flash/net/SharedObject_as.h" // for SharedObjectLibrary
#include "smart_ptr.h" // GNASH_USE_GC
#include "NativeFunction.h"
#include "builtin_function.h"
#include "movie_definition.h"
#include "Movie.h"
#include "movie_root.h"
#include "Globals.h"
#include "Global_as.h"
#include "rc.h" 
#include "namedStrings.h"
#include "VirtualClock.h" // for getTime()

#ifdef ENABLE_AVM2
# include "Machine.h"
#endif

#ifdef HAVE_SYS_UTSNAME_H
# include <sys/utsname.h> // For system information
#endif

#include <memory>
#include <boost/random.hpp> // for random generator
#include <cstdlib> // std::getenv

namespace {
gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
}

namespace gnash {

// Pointer to our singleton
std::auto_ptr<VM> VM::_singleton;

VM&
VM::init(int version, movie_root& root, VirtualClock& clock)
{
	// Don't call more then once !
	assert(!_singleton.get());

	_singleton.reset(new VM(version, root, clock));

	assert(_singleton.get());
	NSV::loadStrings(_singleton->_stringTable, _singleton->getSWFVersion());

    AVM1Global* gl(new AVM1Global(*_singleton));

	_singleton->setGlobal(gl);
    gl->registerClasses();

#ifdef ENABLE_AVM2
	_singleton->_machine = new Machine(*_singleton);
    _singleton->_machine->init();
#endif

	return *_singleton;
}

VM&
VM::get()
{
	// Did you call VM::init ?
	assert(_singleton.get());
	return *_singleton;
}

bool
VM::isInitialized()
{
	return _singleton.get();
}

VM::VM(int version, movie_root& root, VirtualClock& clock)
	:
	_rootMovie(root),
	_global(0),
	_swfversion(version),
#ifdef ENABLE_AVM2
    _machine(0),
#endif
	_clock(clock),
	_stack(),
    _shLib(new SharedObjectLibrary(*this)),
    _avmVersion(AVM1)
{
	_clock.restart();
}

VM::~VM()
{
#ifdef ENABLE_AVM2
    delete _machine;
#endif
}

void
VM::clear()
{
    /// Reset the SharedObjectLibrary, so that SOLs are flushed.
    _shLib.reset();
}

int
VM::getSWFVersion() const
{
	return _swfversion;
}

void
VM::setSWFVersion(int v) 
{
	_swfversion=v;
}

VM::RNG&
VM::randomNumberGenerator() const
{

	static RNG rnd(_clock.elapsed());
	return rnd;
}

const std::string&
VM::getPlayerVersion() const
{
	//From rcfile
	static const std::string version(rcfile.getFlashVersionString());
	return version;
}

const std::string
VM::getOSName()
{

	// The directive in gnashrc must override OS detection.
	if (rcfile.getFlashSystemOS() != "")
	{
		return rcfile.getFlashSystemOS();
	}
	else
	{
#ifdef HAVE_SYS_UTSNAME_H
// For Linux- or UNIX-based systems (POSIX 4.4 conformant)

		struct utsname osname;
		std::string tmp;
		
		uname (&osname);
		
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

const std::string
VM::getSystemLanguage()
{

	char *loc;
	
	// Try various environment variables. These should
	// be in the standard form "de", "de_DE" or "de_DE.utf8"
	// This should work on most UNIX-like systems.
	// Callers should work out what to do with it.
	// TODO: Other OSs.
	if ((loc = std::getenv("LANG")) ||
		(loc = std::getenv("LANGUAGE")) ||
		(loc = std::getenv("LC_MESSAGES"))
		)
	{
		std::string lang = loc;
		return lang;
	}
	
	else
	{
		// No string found
		return "";
	}
}

movie_root&
VM::getRoot() const
{
	return _rootMovie;
}

Global_as*
VM::getGlobal() const
{
#if ENABLE_AVM2
    if (getAVMVersion() == VM::AVM2) return _machine->global();
#endif
	return _global;
}

void
VM::setGlobal(Global_as* o)
{
	assert(!_global);
	_global = o;
}

unsigned long int
VM::getTime() const
{
	return _clock.elapsed();
}

void
VM::markReachableResources() const
{
#ifdef GNASH_USE_GC

	_rootMovie.markReachableResources();

	_global->setReachable();

#if ENABLE_AVM2
    _machine->markReachableResources();
#endif

	/// Mark all static GcResources
	for (ResVect::const_iterator i=_statics.begin(), e=_statics.end(); i!=e; ++i)
	{
		(*i)->setReachable();
	}

    if (_shLib.get()) _shLib->markReachableResources();
#endif

}

void
VmGcRoot::markReachableResources() const
{
	_vm.markReachableResources();
}

void
VM::registerNative(as_c_function_ptr fun, unsigned int x, unsigned int y)
{
    //log_debug("Registering function %p as ASnative(%d, %d) ", (void*)fun, x, y);
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
    as_c_function_ptr fun = col->second;

    NativeFunction* f = new NativeFunction(*_global, fun);
    f->init_member(NSV::PROP_CONSTRUCTOR,
            as_function::getFunctionConstructor());
    return f;
}

///////////////////////////////////////////////////////////////////////
//
// Value ops
//
///////////////////////////////////////////////////////////////////////

void
newAdd(as_value& op1, const as_value& op2, VM& vm)
{
    // We can't change the original value.
    as_value r(op2);

    // The order of the operations is important: op2 is converted to
    // primitive before op1.

	try { r = r.to_primitive(); }
	catch (ActionTypeError& e)
	{
        log_debug(_("%s.to_primitive() threw an error during "
                "ActionNewAdd"), r);
	}
	
    try { op1 = op1.to_primitive(); }
	catch (ActionTypeError& e)
	{
        log_debug(_("%s.to_primitive() threw an error during "
                "ActionNewAdd"), op1);
	}

#if GNASH_DEBUG
	log_debug(_("(%s + %s) [primitive conversion done]"), op1, r);
#endif

	if (op1.is_string() || r.is_string()) {

		// use string semantic
		const int version = vm.getSWFVersion();
		convertToString(op1, vm);
		op1.string_concat(r.to_string_versioned(version));
        return;
	}

    // Otherwise use numeric semantic
    const double num1 = op1.to_number();
    const double num2 = r.to_number();
    op1.set_double(num2 + num1); 

}

void
subtract(as_value& op1, const as_value& op2, VM& /*vm*/)
{
	const double num2 = op2.to_number();
	const double num1 = op1.to_number();
	op1.set_double(num1 - num2);
}

as_value
newLessThan(const as_value& op1, const as_value& op2, VM& /*vm*/)
{

    as_value operand1(op1);
    as_value operand2(op2);

    try { operand1 = op1.to_primitive(as_value::NUMBER); }
    catch (ActionTypeError& e)
    {
        log_debug("%s.to_primitive() threw an error during ActionNewLessThen",
            op1);
    }
    
    if (operand1.is_object() && !operand1.is_sprite()) {
        return false;
    }

    try { operand2 = op2.to_primitive(as_value::NUMBER); }
    catch (ActionTypeError& e)
    {
        log_debug("%s.to_primitive() threw an error during ActionNewLessThen",
            op2);
    }
    
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

    const double num1 = operand1.to_number();
    const double num2 = operand2.to_number();

    if (isNaN(num1) || isNaN(num2)) {
        return as_value();
    }
    return as_value(num1 < num2);
}


} // end of namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
