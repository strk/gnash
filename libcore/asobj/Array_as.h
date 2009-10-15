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
#include <boost/numeric/ublas/vector_sparse.hpp>

#include <string>

// Forward declarations
namespace gnash {
	class fn_call;
	class as_value;
}

namespace gnash {

string_table::key arrayKey(string_table& st, size_t i);

/// The Array ActionScript object
class Array_as : public as_object
{

    /// Other classes shouldn't care about this, but should rather use
    /// the public iterator / const_iterator members.
	typedef boost::numeric::ublas::mapped_vector<as_value> ArrayContainer;

public:

	typedef ArrayContainer::const_iterator const_iterator;
	typedef ArrayContainer::iterator iterator;

    // see dox in as_object.h
	virtual void visitPropertyValues(AbstractPropertyVisitor& visitor) const;

    // see dox in as_object.h
	virtual void visitNonHiddenPropertyValues(AbstractPropertyVisitor& visitor) const;

	/// Sort flags
	enum SortFlags {

		/// Case-insensitive (z precedes A)
		fCaseInsensitive	= (1<<0), // 1

		/// Descending order (b precedes a)
		fDescending		= (1<<1), // 2

		/// If two or more elements in the array
		/// have identical sort fields, return 0
		/// and don't modify the array.
		/// Otherwise proceed to sort the array.
		fUniqueSort		= (1<<2), // 4

		/// Don't modify the array, rather return
		/// a new array containing indexes into it
		/// in sorted order.
		fReturnIndexedArray	= (1<<3), // 8

		/// Numerical sort (9 preceeds 10)
		fNumeric		= (1<<4) // 16
	};

	Array_as();

	~Array_as();

    /// Return true if this is a strict array
    //
    /// Strict arrays are those whose enumerable
    /// properties are only valid positive integer.
    /// Telling strict apart from non-strict is needed
    /// for AMF encoding in remoting.
    ///
    bool isStrict() const;

	Array_as::const_iterator begin();

	Array_as::const_iterator end();

	as_value at(unsigned int index) const;

	unsigned int size() const;

	void resize(unsigned int);

    /// Why is this overridden?
	virtual bool get_member(string_table::key name, as_value* val,
		string_table::key nsname = 0);

	/// Overridden to provide array[#]=x semantic
	virtual bool set_member(string_table::key name,
		const as_value& val, string_table::key nsname=0, bool ifFound=false);

	/// Overridden to deal with indexed elements
	virtual std::pair<bool,bool> delProperty(string_table::key name, string_table::key nsname = 0);

	/// Overridden to expose indexed elements
	virtual bool hasOwnProperty(string_table::key name, string_table::key nsname = 0);

	/// Enumerate elements
	//
	/// See as_object::enumerateNonProperties(as_environment&) for more info.
	///
	virtual void enumerateNonProperties(as_environment&) const;

protected:

#ifdef GNASH_USE_GC
	/// Mark array-specific reachable resources and invoke
	/// the parent's class version (markAsObjectReachable)
	//
	/// array-specific reachable resources are:
	/// 	- The elements values (elements)
	///
	virtual void markReachableResources() const;
#endif // GNASH_USE_GC

private:

	ArrayContainer elements;

	// this function is used internally by set_member and get_member
	// it takes a string that is the member name of the array and returns -1
	// if the string does not refer to an index, or an appropriate int if the string does refer to an index
	int index_requested(string_table::key name);

};

template<typename T>
bool foreachArray(as_object& array, T& pred)
{
    as_value length;
    if (!array.get_member(NSV::PROP_LENGTH, &length)) return false;
    
    const int size = length.to_int();
    if (size < 0) return false;

    string_table& st = getStringTable(array);

    for (size_t i = 0; i < static_cast<size_t>(size); ++i) {
        pred(array.getMember(arrayKey(st, i)));
    }
    return true;
}

/// Initialize the global.Array object
// needed by SWFHandlers::ActionInitArray
void array_class_init(as_object& global, const ObjectURI& uri);

void registerArrayNative(as_object& global);

}

#endif
