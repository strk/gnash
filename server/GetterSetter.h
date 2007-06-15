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

// 
//

#ifndef GNASH_GETTERSETTER_H
#define GNASH_GETTERSETTER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


// Forward declarations
namespace gnash {
	class as_function;
	class as_object;
	class as_value;
}

namespace gnash {


/// 
class GetterSetter
{
	as_function* _getter;

	as_function* _setter;

public:

	/// Copy constructor
	//
	/// updates ref count of getter/setter.
	GetterSetter(const GetterSetter& s);

	/// Assignment operator
	//
	/// updates ref count of getter/setter.
	/// checks for self-assignment
	///
	GetterSetter& operator=(const GetterSetter& s);

	/// Construct a getter/setter parameter
	//
	/// @param getter
	///	getter function, takes no args, returns an as_value
	///	add_ref() will be called on it
	///
	/// @param setter
	///	setter function, takes one arg, returns nothing
	///	add_ref() will be called on it
	///
	GetterSetter(as_function& getter, as_function& setter);

	/// call drop_ref on both getter/setter as_functions
	~GetterSetter();

	/// invoke the getter function
	as_value getValue(as_object* this_ptr) const;

	/// invoke the setter function
	void setValue(as_object* this_ptr, const as_value& val) const;

	/// Mark both getter and setter as being reachable (for GC)
	void setReachable() const;
};


} // namespace gnash

#endif // GNASH_GETTERSETTER_H
