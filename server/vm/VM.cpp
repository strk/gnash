// VM.cpp: the Virtual Machine class, for Gnash
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

/* $Id: VM.cpp,v 1.17 2007/09/13 01:12:20 nihilus Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "VM.h"
#include "movie_definition.h"
#include "movie_instance.h"
#include "movie_root.h"
#include "Global.h"
#include <ctime> // for std::clocks()
#include "rc.h" //for overriding default version string with rcfile

#include <memory>

namespace {
gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
}

namespace gnash {

// Pointer to our singleton
std::auto_ptr<VM> VM::_singleton;

VM&
VM::init(movie_definition& movie)
{
	// Don't call more then once !
	assert(!_singleton.get());

	_singleton.reset(new VM(movie));

	assert(_singleton.get());

	_singleton->setGlobal(new Global(*_singleton));
	assert(_singleton->getGlobal());

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

VM::VM(movie_definition& topmovie)
	:
	_root_movie(new movie_root()),
	_swfversion(topmovie.get_version()),
	_start_time(std::clock())
{
}

VM::~VM()
{
	// nothing to do atm, but we'll likely
	// have to deregister lots of stuff when
	// things are setup
}

std::locale&
VM::getLocale() const
{
	// TODO: some research about what we should be using.
	//       IIRC older SWF contained some tags for this,
	//       while new SWF uses UNICODE...
	static std::locale loc("C");
	return loc;
}

int
VM::getSWFVersion() const
{
	return _swfversion;
}

const std::string&
VM::getPlayerVersion() const
{
	//From rcfile
	static const std::string version(rcfile.getFlashVersionString());
	return version;
}

movie_root&
VM::getRoot() const
{
	return *_root_movie;
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

uint64_t
VM::getTime()
{
  return  (std::clock() -  _start_time);
}

void
VM::markReachableResources() const
{
#ifdef GNASH_USE_GC

	_root_movie->markReachableResources();

	_global->setReachable();

	/// Mark all static GcResources
	for (ResVect::const_iterator i=_statics.begin(), e=_statics.end(); i!=e; ++i)
	{
		(*i)->setReachable();
	}
#endif

}

void
VmGcRoot::markReachableResources() const
{
	_vm.markReachableResources();
}

} // end of namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
