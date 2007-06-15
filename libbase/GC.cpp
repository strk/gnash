// GC.h: Garbage Collector, for Gnash
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
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/* $Id: GC.cpp,v 1.1 2007/06/15 18:16:32 strk Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "GC.h"


namespace gnash {

GC* GC::_singleton = NULL;

GC&
GC::init(GcRoot& root)
{
	assert(!_singleton);
	_singleton = new GC(root);
	return *_singleton;
}

GC&
GC::get()
{
	assert(_singleton);
	return *_singleton;
}

void
GC::cleanup()
{
	assert(_singleton);
	delete _singleton;
	_singleton = NULL;
}

} // end of namespace gnash


