// GC.cpp: Garbage Collector, for Gnash
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

#include "GC.h"

#include <cstdlib>

#include "utility.h" // for typeName()
#include "GnashAlgorithm.h"

#ifdef GNASH_GC_DEBUG
# include "log.h"
#endif


namespace gnash {

GC::GC(GcRoot& root)
    :
    // might raise the default ...
    _maxNewCollectablesCount(64),
    _resListSize(0),
    _root(root),
    _lastResCount(0)
#ifdef GNASH_GC_DEBUG 
    , _collectorRuns(0)
#endif
{
#ifdef GNASH_GC_DEBUG 
    log_debug("GC %p created", (void*)this);
#endif
    char* gcgap = std::getenv("GNASH_GC_TRIGGER_THRESHOLD");
    if (gcgap) {
        const size_t gap = std::strtoul(gcgap, NULL, 0);
        _maxNewCollectablesCount = gap;
    }
}

GC::~GC()
{
#ifdef GNASH_GC_DEBUG 
    log_debug("GC deleted, deleting all managed resources - collector run %d times", _collectorRuns);
#endif
    for (ResList::const_iterator i = _resList.begin(), e = _resList.end();
            i != e; ++i) {
        delete *i;
    }
}

size_t
GC::cleanUnreachable()
{

#if (GNASH_GC_DEBUG > 1)
    log_debug("GC: sweep scan started");
#endif

    size_t deleted = 0;

    for (ResList::iterator i = _resList.begin(), e = _resList.end(); i != e;) {
        const GcResource* res = *i;
        if (!res->isReachable()) {

#if GNASH_GC_DEBUG > 1
            log_debug("GC: recycling object %p (%s)", res, typeName(*res));
#endif
            ++deleted;
            delete res;
            i = _resList.erase(i); // _resListSize updated at end of loop
        }
        else {
            res->clearReachable();
            ++i;
        }
    }

    _resListSize -= deleted;

#ifdef GNASH_GC_DEBUG 
    log_debug("GC: recycled %d unreachable resources - %d left",
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
    log_debug("GC: collection cycle started - %d/%d new resources "
            "allocated since last run (from %d to %d)",
            _resListSize - _lastResCount, _maxNewCollectablesCount,
            _lastResCount, _resListSize);
#endif // GNASH_GC_DEBUG

    // Mark all resources as reachable
    markReachable();

    // clean unreachable resources, and mark the others as reachable again
    cleanUnreachable();

    _lastResCount = _resListSize;

}

void
GC::countCollectables(CollectablesCount& count) const
{
    for (ResList::const_iterator i = _resList.begin(), e = _resList.end();
            i!=e; ++i) {
        ++count[typeName(**i)];
    }
}

} // end of namespace gnash


