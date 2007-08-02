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

/* $Id: GC.cpp,v 1.7 2007/08/02 06:14:55 strk Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "GC.h"
#include "builtin_function.h"

#ifdef GNASH_GC_DEBUG
# include "log.h"
#endif

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
	log_debug(_("GC %p deleted, deleting all managed resources - collector run " SIZET_FMT " times"), (void*)this, _collectorRuns);
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
#if (GNASH_GC_DEBUG > 1)
	log_debug(_("GC %p: SWEEP SCAN"), (void*)this);
#endif
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

void 
GC::collect()
{
	if ( (_resList.size() - _lastResCount) < maxNewCollectablesCount )
	{
#if GNASH_GC_DEBUG  > 1
		log_debug(_("Garbage collection skipped since number of collectables added since last run is too low (" SIZET_FMT ")"),
			       _resList.size() - _lastResCount);
#endif // GNASH_GC_DEBUG
		return;
	}

#ifdef GNASH_GC_DEBUG 
	++_collectorRuns;
#endif

#ifdef GNASH_GC_DEBUG 
	log_debug(_("GC %p Starting collector: " SIZET_FMT " collectables"), (void *)this, _resList.size());
#endif // GNASH_GC_DEBUG

#ifndef NDEBUG
	boost::thread self;
	assert(self == mainThread);
#endif

	// Mark all resources as reachable
	markReachable();

	// clean unreachable resources, and mark them others as reachable again
	cleanUnreachable();

	_lastResCount = _resList.size();
}

} // end of namespace gnash


