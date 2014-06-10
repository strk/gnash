// GC.h: Garbage Collector for Gnash
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#ifndef GNASH_GC_H
#define GNASH_GC_H

// Define the following macro to enable GC verbosity 
// Verbosity levels:
//   1 - print stats about how many resources are registered and how many 
//       are deleted, everytime the GC collector runs.
//   2 - print a message for every GcResource being registered and being deleted
//   3 - print info about the mark scan 
//   
//#define GNASH_GC_DEBUG 1

#include <forward_list>
#include <map>
#include <string>
#include <cassert>

#include "dsodefs.h"
#ifdef GNASH_GC_DEBUG
# include "log.h"
# include "utility.h"
#endif

// Forward declarations.
namespace gnash {
    class GC;
}

namespace gnash {

/// Abstract class to allow the GC to store "roots" into a container
//
/// Any class expected to act as a "root" for the garbage collection 
/// should derive from this class, and implement the markReachableResources()
/// function.
class GcRoot
{
public:

    /// Scan all GC resources reachable by this instance.
    //
    /// This function is invoked on roots registered to
    /// the collector.
    ///
    /// Use setReachable() on the resources stored in this
    /// container.
    virtual void markReachableResources() const = 0;

    virtual ~GcRoot() {}
};

/// Collectable resource
//
/// Instances of this class can be managed by a GC object.
class GcResource
{
public:

    friend class GC;

    /// Create a Garbage-collected resource associated with a GC
    //
    /// @param gc   The GC to register the resource with.
    GcResource(GC& gc);

    /// Mark this resource as being reachable
    //
    /// This can trigger further marking of all resources reachable by this
    /// object.
    //
    /// If the object wasn't reachable before, this call triggers
    /// scan of all contained objects too.
    void setReachable() const {

        if (_reachable) {

#if GNASH_GC_DEBUG > 2
            log_debug(_("Instance %p of class %s already reachable, "
                    "setReachable doing nothing"), (void*)this,
                    typeName(*this));
#endif
            return;
        }

#if GNASH_GC_DEBUG  > 2
        log_debug(_("Instance %p of class %s set to reachable, scanning "
                "reachable resources from it"), (void*)this,
                typeName(*this));
#endif

        _reachable = true;
        markReachableResources();
    }

    /// Return true if this object is marked as reachable
    bool isReachable() const { return _reachable; }

    /// Clear the reachable flag
    void clearReachable() const { _reachable = false; }

protected:

    /// Scan all GC resources reachable by this instance.
    //
    /// This function is invoked everytime this object
    /// switches from unreachable to reachable, and is
    /// used to recursively mark all contained resources
    /// as reachable.
    ///
    /// See setReachable(), which is the function to invoke
    /// against all reachable methods.
    ///
    /// Feel free to assert(_reachable) in your implementation.
    ///
    /// The default implementation doesn't mark anything.
    ///
    virtual void markReachableResources() const {
        assert(_reachable);
#if GNASH_GC_DEBUG > 1
        log_debug(_("Class %s didn't override the markReachableResources() "
                    "method"), typeName(*this));
#endif
    }

    /// Delete this resource.
    //
    /// This is protected to allow subclassing, but ideally it
    /// sould be private, so only the GC is allowed to delete us.
    ///
    virtual ~GcResource() {}

private:

    mutable bool _reachable;

};

/// Garbage collector singleton
//
/// Instances of this class manage a list of heap pointers (collectables),
/// deleting them when no more needed/reachable.
///
/// Their reachability is detected starting from a root, which in turn
/// marks all reachable resources.
class DSOEXPORT GC
{

public:

    /// Create a garbage collector using the given root
    //
    /// @param root     The top level of the GC, which takes care of marking
    ///                 all further resources.
    GC(GcRoot& root);

    /// Destroy the collector, releasing all collectables.
    ~GC();

    /// Add an object to the list of managed collectables
    //
    /// The given object is expected not to be already in the
    /// list. Failing to do so would just decrease performances
    /// but might not be a problem. Anyway, an assertion will fail
    /// if adding an object twice.
    ///
    /// PRECONDITIONS:
    /// - the object isn't already in this GC list.
    /// - the object isn't marked as reachable.
    /// - the object isn't managed by another GC (UNCHECKED)
    ///
    /// @param item
    /// The item to be managed by this collector.
    /// Can't be NULL. The caller gives up ownerhip
    /// of it, which will only be deleted by this GC.
    void addCollectable(const GcResource* item) {

#ifndef NDEBUG
        assert(item);
        assert(!item->isReachable());
#endif

        _resList.emplace_front(item); ++_resListSize;

#if GNASH_GC_DEBUG > 1
        log_debug(_("GC: collectable %p added, num collectables: %d"), item, 
                _resListSize);
#endif
    }

    /// Run the collector, if worth it
    void fuzzyCollect() {

        // Heuristic to decide wheter or not to run the collection cycle
        //
        //
        // Things to consider:
        //
        //  - Cost 
        //      - Depends on the number of reachable collectables
        //      - Depends on the frequency of runs
        //
        //  - Advantages 
        //      - Depends on the number of unreachable collectables
        //
        //  - Cheaply computable informations
        //      - Number of collectables (currently O(n) but can be optimized)
        //      - Total heap-allocated memory (currently unavailable)
        //
        // Current heuristic:
        //
        //  - We run the cycle again if X new collectables were allocated
        //    since last cycle run. X defaults to maxNewCollectablesCount
        //    and can be changed by user (GNASH_GC_TRIGGER_THRESHOLD env
        //    variable).
        //
        // Possible improvements:
        //
        //  - Adapt X (maxNewCollectablesCount) based on cost/advantage
        //    runtime analisys
        //

        if (_resListSize <  _lastResCount + _maxNewCollectablesCount) {
#if GNASH_GC_DEBUG  > 1
            log_debug(_("GC: collection cycle skipped - %d/%d new resources "
                        "allocated since last run (from %d to %d)"),
                    _resListSize-_lastResCount, _maxNewCollectablesCount,
                    _lastResCount, _resListSize);
#endif // GNASH_GC_DEBUG
            return;
        }

        runCycle();
    }

    /// Run the collection cycle
    //
    /// Find all reachable collectables, destroy all the others.
    ///
    void runCycle();

    typedef std::map<std::string, unsigned int> CollectablesCount;

    /// Count collectables
    void countCollectables(CollectablesCount& count) const;

private:

    /// List of collectables
    typedef std::forward_list<const GcResource*> ResList;

    /// Mark all reachable resources
    void markReachable() {
#if GNASH_GC_DEBUG > 2
        log_debug(_("GC %p: MARK SCAN"), (void*)this);
#endif
        _root.markReachableResources();
    }

    /// Delete all unreachable objects, and mark the others unreachable again
    //
    /// @return number of objects deleted
    size_t cleanUnreachable();

    /// Number of newly registered collectable since last collection run
    /// triggering next collection.
    size_t _maxNewCollectablesCount;

    /// List of collectable resources
    ResList _resList;

    /// Size of the ResList to avoid the cost of computing it
    ResList::size_type _resListSize;

    /// The GcRoot.
    GcRoot& _root;

    /// Number of resources in collectable list at end of last
    /// collect() call.
    ResList::size_type _lastResCount;

#ifdef GNASH_GC_DEBUG 
    /// Number of times the collector runs (stats/profiling)
    size_t _collectorRuns;
#endif
};


inline GcResource::GcResource(GC& gc)
    :
    _reachable(false)
{
    gc.addCollectable(this);
}

} // namespace gnash

#endif // GNASH_GC_H
