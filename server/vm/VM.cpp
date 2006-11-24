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

//

/* $Id: VM.cpp,v 1.3 2006/11/24 17:50:47 strk Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "VM.h"
#include "movie_definition.h"
#include "sprite_instance.h"
#include "Global.h"

#include <memory>


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

	_singleton->setRoot(movie.create_instance());
	assert(_singleton->getRoot());

	_singleton->setGlobal(new gnash::Global(*_singleton));
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
	_swfversion(topmovie.get_version())
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

sprite_instance*
VM::getRoot() const
{
	return _root_movie.get();
}

/*public*/
as_object*
VM::getGlobal() const
{
	return _global.get();
}

/*private*/
void
VM::setRoot(sprite_instance* inst)
{
	assert(!_root_movie);
	_root_movie = inst;
}

/*private*/
void
VM::setGlobal(as_object* o)
{
	assert(!_global);
	_global = o;
}


} // end of namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
