// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

//

#ifndef GNASH_VM_H
#define GNASH_VM_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <smart_ptr.h> // for boost::intrusive_ptr

#include <memory> // for auto_ptr
#include <locale>

// Forward declarations
namespace gnash {
	class movie_definition;
	class sprite_instance;
	class as_object;
}

namespace gnash {

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
/// The VM is initialized once for each *main* movie.
/// Gnash currently only supports a *single* VM as it uses
/// gloabls a lot. Definition of this class is aimed at
/// grouping the globals into a specific VM instance that
/// we might pass around in the future to allow multiple
/// movies runs.
/// For the moment, it will be a singleton, providing one-time
/// initialization.
///
class VM {

	/// Use VM::get() to access the singleton
	VM(movie_definition& movie);

	/// Don't copy
	VM(const VM&);

	/// Don't assign
	VM& operator=(const VM&);

	~VM();

	// We use an auto_ptr here to allow constructing
	// the singleton when the init() function is called.
	friend class std::auto_ptr<VM>;
	static std::auto_ptr<VM> _singleton;

	/// \brief
	/// Root movie, will be instanciated from the definition
	/// given to the init() function.
	boost::intrusive_ptr<sprite_instance> _root_movie;

	/// The _global ActionScript object
	boost::intrusive_ptr<as_object> _global;

	/// Target SWF version
	int _swfversion;

	/// Set the current Root movie.
	//
	/// Will be called by the init() function
	/// 
	void setRoot(sprite_instance*);

	/// Set the _global Object for actions run by Virtual Machine
	//
	/// Will be called by the init() function
	/// 
	void setGlobal(as_object*);

public:

	/// \brief
	/// Initialize the virtual machine singleton with the given
	/// movie definition and return a reference to it.
	//
	/// An instance of the given movie will become the absolute
	/// root of the application (ActionScript's _level0)
	///
	/// Don't call this function twice, and make sure you have
	/// called this *before* you call VM::get()
	///
	/// @param movie
	///	The definition for the root movie.
	///
	static VM& init(movie_definition& movie);

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

	/// Get a pointer to this VM's Root movie 
	sprite_instance* getRoot() const;

	/// Get a pointer to this VM's _global Object
	as_object* getGlobal() const;

	/// Get the SWF locale to use 
	std::locale& getLocale() const;

};

} // namespace gnash

#endif // GNASH_VM_H

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
