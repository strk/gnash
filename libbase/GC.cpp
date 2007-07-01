// GC.h: Garbage Collector, for Gnash
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

/* $Id: GC.cpp,v 1.3 2007/07/01 10:54:06 bjacques Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "GC.h"
#include "builtin_function.h"


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

GC::~GC()
{
#ifdef GNASH_GC_DEBUG 
	log_debug(_("GC %p deleted, deleting all managed resources"), (void*)this);
#endif

#if 1
	for (ResList::iterator i=_resList.begin(), e=_resList.end(); i!=e; ++i)
	{
		delete *i;
	}
#endif
}

void
GC::cleanUnreachable()
{
#ifdef GNASH_GC_DEBUG 
	size_t deleted = 0;
	log_debug(_("GC %p: SWEEP SCAN"), (void*)this);
#endif
	for (ResList::iterator i=_resList.begin(), e=_resList.end(); i!=e; )
	{
		const GcResource* res = *i;
		if ( ! res->isReachable() )
		{
#ifdef GNASH_GC_DEBUG 
#if GNASH_GC_DEBUG > 1
			log_debug(_("GC %p: cleanUnreachable deleting object %p (%s)"),
					(void*)this, (void*)res, typeid(*res).name());
#endif
			++deleted;
#endif
			delete res;
			i = _resList.erase(i);
		}
		else
		{
			res->clearReachable();
			++i;
		}
	}
#ifdef GNASH_GC_DEBUG 
	log_debug(_("GC %p: cleanUnreachable deleted " SIZET_FMT
			" resources marked as unreachable"),
			(void*)this, deleted);
#endif
}

} // end of namespace gnash


