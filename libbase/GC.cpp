// GC.cpp: Garbage Collector, for Gnash
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

#include "GC.h"
#include "utility.h" // for typeName()

#ifdef GNASH_GC_DEBUG
#include "log.h"
#endif

#include <cstdlib>

namespace gnash {

GC* GC::_singleton = NULL;
unsigned int GC::maxNewCollectablesCount = 50;

GC&
GC::init(GcRoot& root)
{
	assert(!_singleton);
	_singleton = new GC(root);
	char *gcgap = std::getenv("GNASH_GC_TRIGGER_THRESHOLD");
	if ( gcgap )
	{
		unsigned int gap = strtoul(gcgap, NULL, 0);
		_singleton->maxNewCollectablesCount = gap;
	}
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
	log_debug(_("GC %p deleted, deleting all managed resources - collector run %d times"), (void*)this, _collectorRuns);
#endif

#if 1
	for (ResList::iterator i=_resList.begin(), e=_resList.end(); i!=e; ++i)
	{
		delete *i;
	}
#endif
}

size_t
GC::cleanUnreachable()
{
	size_t deleted = 0;

#if (GNASH_GC_DEBUG > 1)
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
					(void*)this, (void*)res, typeName(*res).c_str());
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
	log_debug(_("GC %p: cleanUnreachable deleted %d"
			" resources marked as unreachable"),
			(void*)this, deleted);
#endif

	return deleted;
}

void 
GC::collect()
{
	size_t curResSize = _resList.size(); // this is O(n) on GNU stdc++ lib !
	if ( (curResSize - _lastResCount) < maxNewCollectablesCount )
	{
#if GNASH_GC_DEBUG  > 1
		log_debug(_("Garbage collection skipped since number of collectables added since last run is too low (%d)"),
			       curResSize - _lastResCount);
#endif // GNASH_GC_DEBUG
		return;
	}

#ifdef GNASH_GC_DEBUG 
	++_collectorRuns;
#endif

#ifdef GNASH_GC_DEBUG 
	log_debug(_("GC %p Starting collector: %d collectables"), (void *)this, curResSize);
#endif // GNASH_GC_DEBUG

#ifndef NDEBUG
	boost::thread self;
	assert(self == mainThread);
#endif

	// Mark all resources as reachable
	markReachable();

	// clean unreachable resources, and mark them others as reachable again
	_lastResCount = curResSize - cleanUnreachable();
}

void
GC::countCollectables(CollectablesCount& count) const
{
	for (ResList::const_iterator i=_resList.begin(), e=_resList.end(); i!=e; ++i)
	{
		const GcResource* res = *i;
		std::string type = typeName(*res);
		count[type]++;
	}
}

} // end of namespace gnash


