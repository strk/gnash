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

#ifndef GNASH_ARRAY_H
#define GNASH_ARRAY_H

#include "as_object.h" // for inheritance
#include "smart_ptr.h" // GNASH_USE_GC
#include "namedStrings.h"
#include "Global_as.h"

#include <deque>
#include <vector>
#include <memory> // for auto_ptr
#include <string>

// Forward declarations
namespace gnash {
	class fn_call;
	class as_value;
}

namespace gnash {

/// Get the length of an object as though it were an array
//
/// It may well be an array, but this also works on normal objects with a 
/// length property
//
/// @param array    The object whose array length is needed.
/// @return         The array length of the object or 0 if no length is
///                 found.
size_t arrayLength(as_object& array);

/// Convert an integral value into an array key
//
/// NB this function adds a string value to the string_table for each separate
/// integral value. It's the way the string_table works.
//
/// @param i        The integral value to find
/// @return         The string table key to look up.
string_table::key arrayKey(string_table& st, size_t i);

/// The Array ActionScript object
class Array_as : public as_object
{

public:

	Array_as();

	~Array_as();

	as_value at(unsigned int index) const;

	unsigned int size() const;

	/// Overridden to provide array[#]=x semantic
	virtual bool set_member(string_table::key name,
		const as_value& val, string_table::key nsname=0, bool ifFound=false);

};

template<typename T>
void foreachArray(as_object& array, T& pred)
{
    size_t size = arrayLength(array);
    if (!size) return;

    string_table& st = getStringTable(array);

    for (size_t i = 0; i < static_cast<size_t>(size); ++i) {
        pred(array.getMember(arrayKey(st, i)));
    }
}

/// Initialize the global.Array object
// needed by SWFHandlers::ActionInitArray
void array_class_init(as_object& global, const ObjectURI& uri);

void registerArrayNative(as_object& global);

}

#endif
