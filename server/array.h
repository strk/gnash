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
//

#ifndef GNASH_ARRAY_H
#define GNASH_ARRAY_H

//#include "action.h"
#include "as_object.h" // for inheritance
#include <deque>
#include <memory> // for auto_ptr

#include <string>

// Forward declarations
namespace gnash {
	struct fn_call;
	struct as_value;
}

namespace gnash {

/// The Array ActionScript object
class as_array_object : public as_object
{

public:

	/// Sort flags
	enum SortFlags {

		/// Case-insensitive (z precedes A)
		fCaseInsensitive	= (1<<0), // 1

		/// Descending order (b precedes a)
		fDescending		= (1<<1), // 2

		/// Remove consecutive equal elements
		fUniqueSort		= (1<<2), // 4

		/// Don't modify the array, rather return
		/// a new array containing indexes into it
		/// in sorted order.
		fReturnIndexedArray	= (1<<3), // 8

		/// Numerical sort (9 preceeds 10)
		fNumeric		= (1<<4) // 16
	};

	as_array_object();

	as_array_object(const as_array_object& other);

	void push(as_value& val);

	void unshift(as_value& val);

	as_value shift();

	as_value pop();

	as_value at(unsigned int index);

	void reverse();

	std::string join(const std::string& separator);

	std::string toString();

	unsigned int size() const;

	//void resize(unsigned int);

	void concat(const as_array_object& other);

	std::auto_ptr<as_array_object> slice(
		unsigned int start, unsigned int one_past_end);

	/// Sort the array, using given values comparator
	void sort(as_function& comparator, uint8_t flags=0);

	void sort(uint8_t flags=0);

	/// Return a new array containing sorted index of this array
	//
	/// NOTE: assert(flags & Array::fReturnIndexedArray)
	std::auto_ptr<as_array_object> sorted_indexes(uint8_t flags);

	std::auto_ptr<as_array_object> sorted_indexes(as_function& comparator, uint8_t flags);

	/// Overridden to provide 'length' member
	virtual bool get_member(const tu_stringi& name, as_value* val);

	/// Overridden to provide array[#]=x semantic
	virtual void set_member(const tu_stringi& name,
		const as_value& val );

private:

	std::deque<as_value> elements;

	// this function is used internally by set_member and get_member
	// it takes a string that is the member name of the array and returns -1
	// if the string does not refer to an index, or an appropriate int if the string does refer to an index
	int index_requested(const tu_stringi& name);

};


/// Initialize the global.Array object
// needed by SWFHandlers::ActionInitArray
void array_class_init(as_object& global);

/// Constructor for ActionScript class Array.
// needed by SWFHandlers::ActionInitArray
void	array_new(const fn_call& fn);

}

#endif
