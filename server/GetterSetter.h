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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
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

	/// copy operator
	//
	/// updates ref count of getter/setter.
	/// checks for self-assignment
	GetterSetter& operator==(const GetterSetter& s);

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
	void getValue(as_object* this_ptr, as_value& ret) const;

	/// invoke the setter function
	void setValue(as_object* this_ptr, const as_value& val) const;
};


} // namespace gnash

#endif // GNASH_GETTERSETTER_H
