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
#include "builtin_function.h"
#include "movie_definition.h"
#include "Movie.h"
#include "movie_root.h"
#include "Global.h"
#include "rc.h" //for overriding default version string with rcfile
#include "namedStrings.h"
#include "ClassHierarchy.h"
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

	_singleton->_classHierarchy.reset(new ClassHierarchy);
	_singleton->setGlobal(
            new AVM1Global(*_singleton, _singleton->_classHierarchy.get()));

#ifdef ENABLE_AVM2
	_singleton->_machine = new Machine(*_singleton);
	assert(_singleton->getGlobal());
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

/*public*/
as_object*
VM::getGlobal() const
{
	return _global.get();
}

/*private*/
void
VM::setGlobal(as_object* o)
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

	/// Mark all static GcResources
	for (ResVect::const_iterator i=_statics.begin(), e=_statics.end(); i!=e; ++i)
	{
		(*i)->setReachable();
	}

	_classHierarchy->markReachableResources();

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
    _asNativeTable[x][y]=fun;
}

builtin_function*
VM::getNative(unsigned int x, unsigned int y)
{
	as_c_function_ptr fun = _asNativeTable[x][y];
	if ( fun ) return new builtin_function(fun);
	else return 0;
}

} // end of namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
