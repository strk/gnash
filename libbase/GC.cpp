// GC.cpp: Garbage Collector, for Gnash
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

#include "GC.h"
#include "utility.h" // for typeName()

#ifdef GNASH_GC_DEBUG
#include "log.h"
#endif

#include <cstdlib>

namespace gnash {

GC* GC::_singleton = NULL;
unsigned int GC::maxNewCollectablesCount = 64;

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
    log_debug(_("GC deleted, deleting all managed resources - collector run %d times"), _collectorRuns);
#endif

    for (ResList::iterator i=_resList.begin(), e=_resList.end(); i!=e; ++i)
    {
        delete *i;
    }
}

size_t
GC::cleanUnreachable()
{
    size_t deleted = 0;

#if (GNASH_GC_DEBUG > 1)
    log_debug(_("GC: sweep scan started"));
#endif

    for (ResList::iterator i=_resList.begin(), e=_resList.end(); i!=e; )
    {
        const GcResource* res = *i;
        if ( ! res->isReachable() )
        {
#if GNASH_GC_DEBUG > 1
            log_debug(_("GC: recycling object %p (%s)"), res, typeName(*res));
#endif
            ++deleted;
            delete res;
            i = _resList.erase(i); // _resListSize updated at end of loop
        }
        else
        {
            res->clearReachable();
            ++i;
        }
    }

    _resListSize -= deleted;

#ifdef GNASH_GC_DEBUG 
    log_debug(_("GC: recycled %d unreachable resources - %d left"),
            deleted, _resListSize);
#endif


    return deleted;
}

void 
GC::runCycle()
{
    //
    // Collection cycle
    //

#ifdef GNASH_GC_DEBUG 
    ++_collectorRuns;
#endif

#ifdef GNASH_GC_DEBUG 
    log_debug(_("GC: collection cycle started - %d/%d new resources allocated since last run (from %d to %d)"), _resListSize-_lastResCount, maxNewCollectablesCount, _lastResCount, _resListSize);
#endif // GNASH_GC_DEBUG

#ifndef NDEBUG
    boost::thread self;
    assert(self == mainThread);
#endif

    // Mark all resources as reachable
    markReachable();

    // clean unreachable resources, and mark the others as reachable again
    cleanUnreachable();

    _lastResCount = _resListSize;

    //assert(_lastResCount == _resList.size()); // O(n)...

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


