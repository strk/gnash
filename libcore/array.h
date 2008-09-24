// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

struct indexed_as_value : public as_value
{
	int vec_index;

	indexed_as_value(const as_value& val, int index)
	: as_value(val)
	{
		vec_index = index;
	}
};

template <class T>
struct ContainerFiller {
	T& cont;
	ContainerFiller(T& c): cont(c) {}
	void visit(as_value& v) { cont.push_back(v); }
};

struct blank {};

/// The Array ActionScript object
class Array_as : public as_object
{

    /// Other classes shouldn't care about this, but should rather use
    /// the public iterator / const_iterator members.
	typedef boost::numeric::ublas::mapped_vector<as_value> ArrayContainer;

public:

	typedef ArrayContainer::const_iterator const_iterator;
	typedef ArrayContainer::iterator iterator;

	typedef std::list<as_value> ValueList;


	enum { itemBlank, itemValue };

	/// Visit all elements 
	//
	/// The visitor class will have to expose a visit(as_value&) method
	///
	template<class V> void visitAll(V& v)
	{
		// NOTE: we copy the elements as the visitor might call arbitrary code
		//       possibly modifying the container itself.
		ArrayContainer copy = elements;

		// iterating this way will skip holes
		for (Array_as::iterator i=copy.begin(), ie=copy.end(); i!=ie; ++i)
			v.visit(*i);
	}

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

	Array_as(const Array_as& other);

	~Array_as();

	std::deque<indexed_as_value> get_indexed_elements();

	Array_as::const_iterator begin();

	Array_as::const_iterator end();

	/// Push an element to the end of the array
	//
	/// @param val
	/// 	The element to add 
	///
	void push(const as_value& val);

	void unshift(const as_value& val);

	as_value shift();

	as_value pop();

	as_value at(unsigned int index) const;

	Array_as* get_indices(std::deque<indexed_as_value> origElems);

	void reverse();

	void set_indexed(unsigned int index, const as_value &v);

	/// @param env
	///	If not-null will be used to properl invoke the toString()
	///	method against member values.
	///
	std::string join(const std::string& separator, as_environment* env) const;

	/// @param env
	///	If not-null will be used to properly invoke the toString()
	///	method against member values.
	///
	std::string toString(as_environment* env=NULL) const;

	// override from as_object
	std::string get_text_value() const
	{
		return toString();
	}

	unsigned int size() const;

	void resize(unsigned int);

	void concat(const Array_as& other);

	/// \brief
	/// Return a newly created array containing elements
	/// from 'start' up to but not including 'end'.
	//
	///
	/// NOTE: assertions are:
	///
	///	assert(one_past_end >= start);
	///	assert(one_past_end <= size());
	///	assert(start <= size());
	///
	/// @param start
	///	index to first element to include in result
	///	0-based index.
	///
	/// @param one_past_end
	///	index to one-past element to include in result
	///	0-based index.
	///
	boost::intrusive_ptr<Array_as> slice(
		unsigned int start, unsigned int one_past_end);

	/// Remove first element matching the given value
	//
	/// Return true if any element was removed, false otherwise
	///
	/// NOTE: if an element is removed, holes in the array will be
	///       filled.
	///
	/// @param v
	///	The value to compare elements against
	///
	/// @param env
	///	The environment to use when comparing (needed by as_value::equals)
	///
	bool removeFirst(const as_value& v);

	/// \brief
	/// Replace count elements from start with given values, optionally
	/// returning the erased ones.
	//
	/// @param start
	///	First element to remove. Will abort if invalid.
	///
	/// @param count
	///	Number of elements to remove. Will abort if > then available.
	///
	/// @param replace
	///	If not null, use as a replacement for the cutted values
	///
	/// @param copy
	///	If not null, an array to push cutted values to.
	///
	void splice(unsigned int start, unsigned int count, 
			const std::vector<as_value>* replace=NULL,
			Array_as* copy=NULL);

	/// \brief
	/// Sort the array, using given values comparator
	///
	/// @param avc
	///	boolean functor or function comparing two as_value& objects
	///
	template <class AVCMP>
	void sort(AVCMP avc)
	{
		// IMPORTANT NOTE
		//
		// As for ISO/IEC 14882:2003 - 23.2.2.4.29 
		// the sort algorithm relies on the assumption
		// that the comparator function implements
		// a Strict Weak Ordering operator:
		// http://www.sgi.com/tech/stl/StrictWeakOrdering.html
		//
		// Invalid comparator can lead to undefined behaviour,
		// including invalid memory access and infinite loops.
		//
		// Pragmatically, it seems that std::list::sort is
		// more robust in this reguard, so we'll sort a list
		// instead of the queue. We want to sort a copy anyway
		// to avoid the comparator changing the original container.
		//
		ValueList nelem;
		ContainerFiller<ValueList> filler(nelem);
		visitAll(filler);

		size_t oldSize = elements.size(); // custom comparator might change input size
		nelem.sort(avc);
		elements.resize(oldSize, false);
		size_t idx=0;
		for (ValueList::iterator i=nelem.begin(), e=nelem.end(); i!=e; ++i)
		{
			elements[idx++] = *i;
		};
	}

	/// \brief
	/// Attempt to sort the array using given values comparator, avc.
	/// If two or more elements in the array are equal, as determined
	/// by the equality comparator ave, then the array is not sorted
	/// and 0 is returned. Otherwise the array is sorted and returned.
	///
	/// @param avc
	///	boolean functor or function comparing two as_value& objects
	///     used to determine sort-order
	///
	/// @param ave
	///	boolean functor or function comparing two as_value& objects
	///     used to determine equality
	///
	template <class AVCMP, class AVEQ>
	as_value sort(AVCMP avc, AVEQ ave)
	{
		// IMPORTANT NOTE
		//
		// As for ISO/IEC 14882:2003 - 23.2.2.4.29 
		// the sort algorithm relies on the assumption
		// that the comparator function implements
		// a Strict Weak Ordering operator:
		// http://www.sgi.com/tech/stl/StrictWeakOrdering.html
		//
		// Invalid comparator can lead to undefined behaviour,
		// including invalid memory access and infinite loops.
		//
		// Pragmatically, it seems that std::list::sort is
		// more robust in this reguard, so we'll sort a list
		// instead of the queue. We want to sort a copy anyway
		// to avoid the comparator changing the original container.
		//

		typedef std::list<as_value> ValueList;
		ValueList nelem;
		ContainerFiller<ValueList> filler(nelem);
		visitAll(filler);

		size_t oldSize = elements.size(); // custom comparator might change input size

		nelem.sort(avc);

		if (std::adjacent_find(nelem.begin(), nelem.end(), ave) != nelem.end() )
			return as_value(0);

		elements.resize(oldSize, false);
		size_t idx=0;
		for (ValueList::iterator i=nelem.begin(), e=nelem.end(); i!=e; ++i)
		{
			elements[idx++] = *i;
		};

		return as_value(this);
	}

	/// \brief
	/// Return a new array containing sorted index of this array
	///
	/// @param avc
	///	boolean functor or function comparing two as_value& objects
	///
	template <class AVCMP>
	Array_as* sort_indexed(AVCMP avc)
	{
		std::deque<indexed_as_value> ielem = get_indexed_elements();
		std::sort(ielem.begin(), ielem.end(), avc);
		return get_indices(ielem);
	}

	/// \brief
	/// Return a new array containing sorted index of this array.
	/// If two or more elements in the array are equal, as determined
	/// by the equality comparator ave, then 0 is returned instead.
	///
	/// @param avc
	///	boolean functor or function comparing two as_value& objects
	///     used to determine sort-order
	///
	/// @param ave
	///	boolean functor or function comparing two as_value& objects
	///     used to determine equality
	///
	template <class AVCMP, class AVEQ>
	as_value sort_indexed(AVCMP avc, AVEQ ave)
	{
		std::deque<indexed_as_value> ielem = get_indexed_elements();

		std::sort(ielem.begin(), ielem.end(), avc);

		if (std::adjacent_find(ielem.begin(), ielem.end(), ave) != ielem.end() )
			return as_value(0);

		return get_indices(ielem);
	}

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

	/// Shift all elements to the left by count positions
	//
	/// Pre-condition: size of the array must be >= count
	/// Post-condition: size of the array will reduce by 'count'
	///
	void shiftElementsLeft(unsigned int count);

	/// Shift all elements to the right by count positions
	//
	/// Pre-condition: none
	/// Post-condition: size of the array will incremented by 'count'
	///
	void shiftElementsRight(unsigned int count);
};


/// Initialize the global.Array object
// needed by SWFHandlers::ActionInitArray
void array_class_init(as_object& global);

/// Constructor for ActionScript class Array.
// needed by SWFHandlers::ActionInitArray
as_value	array_new(const fn_call& fn);

}

#endif
