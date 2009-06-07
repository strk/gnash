// GC.h: Garbage Collector for Gnash
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

#ifndef GNASH_GC_H
#define GNASH_GC_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <list>
#include <map>
#include <string>
#include <algorithm> //for std::find

#include <boost/thread.hpp>

#include "dsodefs.h"

// Define the following macro to enable GC verbosity 
// Verbosity levels:
//   1 - print stats about how many resources are registered and how many 
//       are deleted, everytime the GC collector runs.
//   2 - print a message for every GcResource being registered and being deleted
//   3 - print info about the mark scan 
//   
//#define GNASH_GC_DEBUG 1

#ifdef GNASH_GC_DEBUG
# include "log.h"
# include <typeinfo>
#endif

#include <cassert>


namespace gnash {

class GC;

/// Abstract class to allow the GC to store "roots" into a container
//
/// Any class expected to act as a "root" for the garbage collection 
/// should derive from this class, and implement the markReachableResources()
/// method.
///
///
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
	///
	virtual void markReachableResources() const=0;

	virtual ~GcRoot() {}
};

/// Collectable resource
//
/// Instances of this class can be managed by a GC object.
///
class GcResource
{

public:

	friend class GC;

	/// Create a Garbage-collected resource.
	//
	/// The resource will be automatically registered with
	/// the garbage collector singleton.
	///
	GcResource();

	/// \brief
	/// Mark this resource as being reachable, possibly triggering
	/// further marking of all resources reachable by this object.
	//
	/// If the object wasn't reachable before, this call triggers
	/// scan of all contained objects too...
	///
	void setReachable() const
	{

		if ( _reachable )
		{
#if GNASH_GC_DEBUG > 2
			log_debug(_("Instance %p of class %s already reachable, setReachable doing nothing"),
					(void*)this, typeid(*this).name());
#endif
			return;
		}

#if GNASH_GC_DEBUG  > 2
		log_debug(_("Instance %p of class %s set to reachable, scanning reachable resources from it"),
				(void*)this, typeid(*this).name());
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
	virtual void markReachableResources() const
	{
		assert(_reachable);
#if GNASH_GC_DEBUG > 1
		log_debug(_("Class %s didn't override the markReachableResources() method"), typeid(*this).name());
#endif
	}

	/// Delete this resource.
	//
	/// This is protected to allow subclassing, but ideally it
	/// sould be private, so only the GC is allowed to delete us.
	///
	virtual ~GcResource()
	{
	}

private:

	mutable bool _reachable;

};

/// Garbage collector singleton
//
/// Instances of this class ( you' only use one, the singleton )
/// manage a list of heap pointers (collectables) deleting them
/// when no more needed/reachable.
///
/// Reachability of them is detected starting from a list
/// of "root" containers each one expected to provide a
/// function to return all reachable object.
///
/// Each "collectable" objects is also expected to be a container
/// itself.
///
///
class DSOEXPORT GC
{

public:

	/// Init the singleton instance using the given GcRoot
	//
	static GC& init(GcRoot& r);

	/// Delete the singleton. You'll need to call init() again
	/// after this call, if you want to use the singleton.
	//
	/// See init(GcRoot&)
	///
	static void cleanup();

	/// Get the singleton 
	//
	/// An assertion will fail if the GC has not been initialized yet.
	/// See init(GcRoot&).
	///
	static GC& get();

	/// Add an heap object to the list of managed collectables
	//
	/// The given object is expected not to be already in the
	/// list. Failing to do so would just decrease performances
	/// but might not be a problem. Anyway, an assertion will fail
	/// if adding an object twice.
	///
	/// PRECONDITIONS:
	///	- the object isn't already in this GC list.
	///	- the object isn't marked as reachable.
	///	- the object isn't managed by another GC (UNCHECKED)
	///
	/// @param item
	///	The item to be managed by this collector.
	///	Can't be NULL. The caller gives up ownerhip
	///	of it, which will only be deleted by this GC.
	///
	void addCollectable(const GcResource* item)
	{
	  boost::thread self;
	  // Don't add an object from a thread
	  if (self == mainThread) {	  
#ifndef NDEBUG
		assert(self == mainThread);
		assert(item);
		assert(! item->isReachable());
		// The following assertion is expensive ...
		//assert(std::find(_resList.begin(), _resList.end(), item) == _resList.end());
#endif

		_resList.push_back(item);
	  }
#if GNASH_GC_DEBUG > 1
		log_debug(_("GC %p: collectable %p added, num collectables: %d"), (void*)this, (void*)item, _resList.size());
#endif
	}


	/// Run the collector
	//
	/// Find all reachable collectables, destroy all the others.
	///
	void collect();

	typedef std::map<std::string, unsigned int> CollectablesCount;

	/// Count collectables
	void countCollectables(CollectablesCount& count) const;

private:

	/// Number of newly registered collectable since last collection run
	/// triggering next collection.
	static unsigned int maxNewCollectablesCount;

	/// Create a garbage collector, using the given root
	GC(GcRoot& root)
		:
		_root(root),
		_lastResCount(0)
#ifdef GNASH_GC_DEBUG 
		, _collectorRuns(0)
#endif
	{
#ifdef GNASH_GC_DEBUG 
		log_debug(_("GC %p created"), (void*)this);
#endif
	}

	/// Destroy the collector, releasing all collectables.
	~GC();

	/// List of collectables
	typedef std::list<const GcResource *> ResList;

	/// Mark all reachable resources
	void markReachable()
	{
#if GNASH_GC_DEBUG > 2
		log_debug(_("GC %p: MARK SCAN"), (void*)this);
#endif
		_root.markReachableResources();
	}

	/// Delete all unreachable objects, and mark the others unreachable again
	//
	/// @return number of objects deleted
	///
	size_t cleanUnreachable();

	ResList _resList;

	GcRoot& _root;

	static GC* _singleton;

	/// The thread that initialized the GC is 
	/// the only one allowed to run the collector
	/// and to register collectable objects
	boost::thread mainThread;

	/// Number of resources in collectable list at end of last
	/// collect() call.
	ResList::size_type _lastResCount;

#ifdef GNASH_GC_DEBUG 
	/// Number of times the collector runs (stats/profiling)
	size_t _collectorRuns;
#endif
};


inline GcResource::GcResource()
	:
	_reachable(false)
{
	GC::get().addCollectable(this);
}

} // namespace gnash

#endif // GNASH_GC_H
