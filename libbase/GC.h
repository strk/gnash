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

#ifndef GNASH_GC_H
#define GNASH_GC_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <list>

// Define the following macro to enable GC verbosity 
#define GNASH_GC_DEBUG 1

#ifdef GNASH_GC_DEBUG
# include "log.h"
# include <typeinfo>
#endif

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
#ifdef GNASH_GC_DEBUG 
			log_debug("Instance %p of class %s already reachable, setReachable doing nothing",
					(void*)this, typeid(*this).name());
#endif
			return;
		}

#ifdef GNASH_GC_DEBUG 
		log_debug("Instance %p of class %s set to reachable, scanning reachable resources from it",
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
#ifdef GNASH_GC_DEBUG 
		log_debug("Class %s didn't override the markReachableResources() method", typeid(*this).name());
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
class GC
{

public:

	friend class GcResource;

	/// Get the singleton instance of the Garbage Collector
	static GC& getInstance()
	{
		static GC singleton;
		return singleton;
	}

	/// Add a root resource to use for mark scanning
	//
	/// @param root
	///     A GcResource to use as a root item to scan
	///     during the mark phase.
	///
	void addRoot(GcRoot* root)
	{
		assert(root);
		_roots.push_back(root);
#ifdef GNASH_GC_DEBUG
		log_debug("GC %p: root %p added, num roots: " SIZET_FMT, (void*)this, (void*)root, _roots.size());
#endif
	}


	/// Run the collector
	//
	/// Find all reachable collectables, destroy all the others.
	///
	void collect()
	{
#ifdef GNASH_GC_DEBUG 
		log_debug("Starting collector: " SIZET_FMT " roots, " SIZET_FMT " collectables", _roots.size(), _resList.size());
#endif // GNASH_GC_DEBUG

		// Mark all resources as reachable
		markReachable();

		// clean unreachable resources, and mark them others as reachable again
		cleanUnreachable();

	}

private:

	/// List of collectables
	typedef std::list<const GcResource *> ResList;

	/// List of roots
	typedef std::list<const GcRoot *> RootList;

	/// Create a garbage collector
	GC()
	{
#ifdef GNASH_GC_DEBUG 
		log_debug("GC %p created", (void*)this);
#endif
	}

	/// Destroy the collector, releasing all collectables.
	~GC()
	{
#ifdef GNASH_GC_DEBUG 
		log_debug("GC %p deleted, cleaning up all managed resources", (void*)this);
#endif
		for (ResList::iterator i=_resList.begin(), e=_resList.end(); i!=e; ++i)
		{
			delete *i;
		}
	}

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
		assert(item);
		assert(! item->isReachable());
		assert(std::find(_resList.begin(), _resList.end(), item) == _resList.end());

		_resList.push_back(item);
#ifdef GNASH_GC_DEBUG 
		log_debug("GC %p: collectable %p added, num collectables: " SIZET_FMT, (void*)this, (void*)item, _resList.size());
#endif
	}

	/// Mark all reachable resources
	void markReachable()
	{
		/// By marking the roots as reachable, a chain effect should be
		/// engaged so that every reachable resource is marked
		for (RootList::iterator i=_roots.begin(), e=_roots.end(); i!=e; ++i)
		{
			const GcRoot* root = *i;
			root->markReachableResources(); 
		}
	}

	/// Delete all unreachable objects, and mark the others unreachable again
	void cleanUnreachable()
	{
#ifdef GNASH_GC_DEBUG 
		size_t deleted = 0;
#endif
		for (ResList::iterator i=_resList.begin(), e=_resList.end(); i!=e; )
		{
			const GcResource* res = *i;
			if ( ! res->isReachable() )
			{
#ifdef GNASH_GC_DEBUG 
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
		log_debug("GC %p: cleanUnreachable deleted " SIZET_FMT
				" resources marked as unreachable",
				(void*)this, deleted);
#endif
	}

	ResList _resList;

	RootList _roots;

};


inline GcResource::GcResource()
	:
	_reachable(false)
{
	GC::getInstance().addCollectable(this);
}

} // namespace gnash

#endif // GNASH_GC_H
