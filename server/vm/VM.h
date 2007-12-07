// VM.h: the Virtual Machine class, for Gnash
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

#ifndef GNASH_VM_H
#define GNASH_VM_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "smart_ptr.h" // for boost::intrusive_ptr
#include "movie_root.h" // for composition
#include "GC.h" // for ineritance of VmGcRoot
#include "string_table.h" // for the string table

#include <memory> // for auto_ptr
#include <locale>
#include <boost/cstdint.hpp> // for boost::?int??_t 

// Forward declarations
namespace gnash {
	class movie_definition;
	class builtin_function;
	class as_object;
	class Machine;
	class VirtualClock;
}

namespace gnash {

class ClassHierarchy;

/// A GC root used to mark all reachable collectable pointers
class VmGcRoot : public GcRoot 
{
	VM& _vm;

public:

	VmGcRoot(VM& vm)
		:
		_vm(vm)
	{
	}

	virtual void markReachableResources() const;
};

/// The virtual machine
//
/// This is the machine that executes all actions in the 
/// main movie, including the actions in movies loaded by
/// it.
///
/// Note that the target SWF version of the "main" movie
/// (the first movie loaded, the 'root' movie) drives
/// the operation, as depending on that version the Virtual
/// Machine acts differently, for backward compatibility.
/// 
/// The VM is initialized once for each "stage" (main movie).
/// Gnash currently only supports a *single* VM as it uses
/// gloabls a lot. Definition of this class is aimed at
/// grouping the globals into a specific VM instance that
/// we might pass around in the future to allow multiple
/// movies runs.
/// For the moment, it will be a singleton, providing one-time
/// initialization.
///
class DSOEXPORT VM {

public:

	typedef as_value (*as_c_function_ptr)(const fn_call& fn);

private:


	friend class VmGcRoot;

	/// Use VM::get() to access the singleton
	//
	/// Initializes the GC singleton
	///
	VM(movie_definition& movie, VirtualClock& clock);

	/// Don't copy
	VM(const VM&);

	/// Don't assign
	VM& operator=(const VM&);

	/// Should deinitialize the GC singleton
	/// If it doesn't is just because it corrupts memory :)
	~VM();

	// We use an auto_ptr here to allow constructing
	// the singleton when the init() function is called.
	friend class std::auto_ptr<VM>;
	static std::auto_ptr<VM> _singleton;

	/// Stage associated with this VM
	std::auto_ptr<movie_root> _root_movie;

	/// The _global ActionScript object
	boost::intrusive_ptr<as_object> _global;

	/// Target SWF version
	int _swfversion;

	/// Time when the VM get started
	boost::uint64_t _start_time;

	/// Set the _global Object for actions run by Virtual Machine
	//
	/// Will be called by the init() function
	/// 
	void setGlobal(as_object*);

#ifdef GNASH_USE_GC
	/// A vector of static GcResources (tipically used for built-in class constructors)
	//
	/// The resources in this list will always be marked as reachable
	///
	typedef std::vector< boost::intrusive_ptr<GcResource> > ResVect;
	ResVect _statics;
#endif

	typedef std::map<unsigned int, as_c_function_ptr> FuncMap;
	typedef std::map<unsigned int, FuncMap> AsNativeTable;
	AsNativeTable _asNativeTable;

	/// Mutable since it should not affect how the VM runs.
	mutable string_table mStringTable;
	/// Not mutable since changing this changes behavior of the VM.
	ClassHierarchy *mClassHierarchy;
	/// A running execution thread.
	Machine *mMachine;

	VirtualClock& _clock;

public:

	/// \brief
	/// Initialize the virtual machine singleton with the given
	/// movie definition and return a reference to it.
	//
	/// The given movie will be only used to fetch SWF version from.
	///
	/// Don't call this function twice, and make sure you have
	/// called this *before* you call VM::get()
	///
	/// @param movie
	///	The definition for the root movie, only
	///	used to fetch SWF version from.
	///	TODO: take SWF version directly ?
	///
	/// @param clock
	///	Virtual clock used as system time.
	///
	static VM& init(movie_definition& movie, VirtualClock& clock);

	/// Return true if the singleton VM has been initialized
	static bool isInitialized();

	/// Get the singleton instance of the virtual machine
	//
	/// Make sure you called VM::init() before trying to
	/// get the singleton (an assertion would fail otherwise)
	///
	/// Use isInitialized() if you're unsure.
	///
	static VM& get();

	/// Get version of the SWF movie used to initialize this VM
	//
	/// This information will drive operations of the virtual machine
	///
	int getSWFVersion() const;

	/// Get the number of milliseconds since VM was started
	unsigned long int getTime() const;

	/// Get a reference to the string table used by the VM.
	string_table& getStringTable() const { return mStringTable; }

	/// Get a pointer to the machine, if it exists.
	Machine* getMachine() const { return mMachine; }

	/// Get version of the player, in a compatible representation
	//
	/// This information will be used for the System.capabilities.version
	/// and $version ActionScript variables.
	///
	const std::string& getPlayerVersion() const;

	/// Get a pointer to this VM's Root movie (stage)
	movie_root& getRoot() const;

	/// Get a pointer to this VM's _global Object
	as_object* getGlobal() const;

	/// Get a pointer to this VM's global ClassHierarchy object.
	ClassHierarchy* getClassHierarchy() const { return mClassHierarchy; }
	
	/// Get the SWF locale to use 
	std::locale& getLocale() const;

	/// Mark all reachable resources (for GC)
	//
	/// - root movie / stage (_root_movie)
	/// - Global object (_global)
	/// - registered static GcResources (_statics)
	/// - Class Hierarchy object
	///
	///
	void markReachableResources() const;

	void registerNative(as_c_function_ptr fun, unsigned int x, unsigned int y)
	{
		assert(fun);
		assert(!_asNativeTable[x][y]);
		_asNativeTable[x][y]=fun;
	}

	/// Return a newly created builtin_function or null
	builtin_function* getNative(unsigned int x, unsigned int y);

#ifdef GNASH_USE_GC
	void addStatic(GcResource* res)
	{
		_statics.push_back(res);
	}
#else  // ndef GNASH_USE_GC
	// placeholder to avoid adding lots of
	// compile-time switches in callers
	void addStatic(as_object*) {}
#endif

};

} // namespace gnash

#endif // GNASH_VM_H

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
