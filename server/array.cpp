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
//
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// This needs to be included first for NetBSD systems or we get a weird
// problem with pthread_t being defined too many times if we use any
// STL containers.
#ifdef HAVE_PTHREADS
#include <pthread.h>
#endif

#include "as_value.h"
#include "array.h"
#include "log.h"
#include "builtin_function.h" // for Array class
#include "as_function.h" // for sort user-defined comparator
#include "fn_call.h"
#include "GnashException.h"

#include <string>
#include <algorithm>
#include <memory> // for auto_ptr

namespace gnash {

static as_object* getArrayInterface();

// Default as_value strict weak comparator (string based)
class AsValueLessThen
{
public:
	bool operator() (const as_value& a, const as_value& b)
	{
		return ( a.to_tu_string() < b.to_tu_string() );
	}
};

// Default descending as_value strict weak comparator (string based)
class AsValueLessThenDesc
{
public:
	bool operator() (const as_value& a, const as_value& b)
	{
		return ( a.to_string() > b.to_string() );
	}
};

// Case-insensitive as_value strict weak comparator (string)
class AsValueLessThenNoCase
{
public:
	bool operator() (const as_value& a, const as_value& b)
	{
		return ( a.to_tu_stringi() < b.to_tu_stringi() );
	}
};

// Descending Case-insensitive as_value strict weak comparator (string)
class AsValueLessThenDescNoCase
{
public:
	bool operator() (const as_value& a, const as_value& b)
	{
		return ( a.to_tu_stringi() > b.to_tu_stringi() );
	}
};

// Numeric as_value strict weak comparator 
class AsValueLessThenNumeric
{
public:
	bool operator() (const as_value& a, const as_value& b)
	{
		return ( a.to_number() < b.to_number() );
	}
};

// Descending Numeric as_value strict weak comparator 
class AsValueLessThenDescNumeric
{
public:
	bool operator() (const as_value& a, const as_value& b)
	{
		return ( a.to_number() > b.to_number() );
	}
};


// Custom (ActionScript) comparator 
class AsValueFuncComparator
{
public:
	as_function& _comp;

	AsValueFuncComparator(as_function& comparator)
		:
		_comp(comparator)
	{
	}

	bool operator() (const as_value& a, const as_value& b)
	{
		// Ugly, but I can't see another way to 
		// provide fn_call a stack to work on
		as_environment env;
		env.push(a);
		env.push(b);

		as_value ret(false); // bool value
		fn_call fn(&ret, NULL, &env, 2, 0);
		_comp(fn);
		return ( ret.to_bool() );
	}
};

// @@ TODO : implement as_array_object's unimplemented functions

as_array_object::as_array_object()
	:
	as_object(getArrayInterface()), // pass Array inheritance
	elements(0)
{
    //log_action("%s : %p\n", __FUNCTION__, (void*)this);
}

as_array_object::as_array_object(const as_array_object& other)
	:
	as_object(other),
	elements(other.elements)
{
    //log_action("%s : %p\n", __FUNCTION__, (void*)this);
}

int
as_array_object::index_requested(const std::string& name)
{
	as_value temp;
	temp.set_string(name.c_str());
	double value = temp.to_number();

	// if we were sent a string that can't convert like "asdf", it returns as NaN. -1 means invalid index
	if (isnan(value)) return -1;

	// TODO / WARNING: because to_number returns a double and we're
	// converting to an int,
	// I want to make sure we're above any "grey area" when we we round down
	// by adding a little to the number before we round it.
	// We don't want to accidentally look to index-1!
	return int(value + 0.01);
}

void
as_array_object::push(as_value& val)
{
	elements.push_back(val);
}

void
as_array_object::unshift(as_value& val)
{
	elements.push_front(val);
}

as_value
as_array_object::pop()
{
	// If the array is empty, report an error and return undefined!
	if (elements.size() <= 0)
	{
	    log_warning("tried to pop element from back of empty array, returning undef!\n");
		return as_value(); // undefined
	}

	as_value ret = elements.back();
	elements.pop_back();

	return ret;
}

as_value
as_array_object::shift()
{
	// If the array is empty, report an error and return undefined!
	if (elements.size() <= 0)
	{
		log_warning("tried to shift element from front of empty array, returning undef!\n");
		return as_value(); // undefined
	}

	as_value ret = elements.front();
	elements.pop_front();

	return ret;
}

void
as_array_object::reverse()
{
	// Reverse the deque elements
	std::reverse(elements.begin(), elements.end());
}

std::string
as_array_object::join(const std::string& separator) const
{
	// TODO - confirm this is the right format!
	// Reportedly, flash version 7 on linux, and Flash 8 on IE look like
	// "(1,2,3)" and "1,2,3" respectively - which should we mimic?
	// Using no parentheses until confirmed for sure
	//
	// We should change output based on SWF version --strk 2006-04-28

	std::string temp;
	//std::string temp = "("; // SWF > 7

	if ( ! elements.empty() ) 
	{
		std::deque<as_value>::const_iterator
			it=elements.begin(),
			itEnd=elements.end();

		// print first element w/out separator prefix
		temp += (*it++).to_string();

		// print subsequent elements with separator prefix
		while ( it != itEnd )
		{
			temp += separator + (*it++).to_string();
		}
	}

	// temp += ")"; // SWF > 7

	return temp;

}

void
as_array_object::concat(const as_array_object& other)
{
	elements.insert(elements.end(), other.elements.begin(),
		other.elements.end());
}

std::string
as_array_object::toString() const
{
	return join(",");
}

unsigned int
as_array_object::size() const
{
	return elements.size();
}

as_value
as_array_object::at(unsigned int index)
{
	if ( index > elements.size()-1 )
	{
		return as_value();
	}
	else
	{
		return elements[index];
	}
}

std::auto_ptr<as_array_object>
as_array_object::slice(unsigned int start, unsigned int one_past_end)
{
	assert(one_past_end >= start);
	assert(one_past_end <= size());
	assert(start <= size());

	std::auto_ptr<as_array_object> newarray(new as_array_object);

	log_msg("Array.slice(%u, %u) called", start, one_past_end);

	size_t newsize = one_past_end - start;
	newarray->elements.resize(newsize);

	// maybe there's a standard algorithm for this ?
	for (unsigned int i=start; i<one_past_end; ++i)
	{
		newarray->elements[i-start] = elements[i];
	}

	return newarray;

}

/* virtual public, overriding as_object::get_member */
bool
as_array_object::get_member(const std::string& name, as_value *val)
{
	if ( name == "length" ) 
	{
		val->set_double((double)size());
		return true;
	}

	// an index has been requested
	int index = index_requested(name);
	if ( index >= 0 && (unsigned int)index < elements.size() )
	{
		*val = elements[index];
		return true;
	}

	return get_member_default(name, val);
}

void
as_array_object::resize(unsigned int newsize)
{
	elements.resize(newsize);
}

/* virtual public, overriding as_object::set_member */
void
as_array_object::set_member(const std::string& name,
		const as_value& val )
{
	if ( name == "length" ) 
	{
		//log_warning("Attempt to assign to Array.length - ignored");
		resize(unsigned(val.to_number()));
		return;
	}

	int index = index_requested(name);

	// if we were sent a valid array index and not a normal member
	if (index >= 0)
	{
		if (index >= int(elements.size()))
		{
			// if we're setting index (x), the vector
			// must be size (x+1)
			elements.resize(index+1);
		}

		// set the appropriate index and return
		elements[index] = val;
		return;
	}

	as_object::set_member_default(name,val);
}

std::auto_ptr<as_array_object>
as_array_object::sorted_indexes(uint8_t flags)
{
	assert(flags & as_array_object::fReturnIndexedArray);
	log_error("Array.sorted_index() method not implemented yet!\n");
	return std::auto_ptr<as_array_object>(NULL);
}

void
as_array_object::sort(uint8_t flags)
{

	// use sorted_index to use this flag
	assert( ! (flags & as_array_object::fReturnIndexedArray) );

	bool do_unique = (flags & as_array_object::fUniqueSort);

	// strip the UniqueSort flag, we'll use the do_unique later
	flags &= ~(as_array_object::fUniqueSort);

	switch ( flags )
	{
		case 0: // default sorting
			//log_msg("Default sorting");
			std::sort(elements.begin(), elements.end(),
				AsValueLessThen());
			break;

		case as_array_object::fDescending:
			//log_msg("Default descending");
			std::sort(elements.begin(), elements.end(),
				AsValueLessThenDesc());
			break;

		case as_array_object::fCaseInsensitive: 
			//log_msg("case insensitive");
			std::sort(elements.begin(), elements.end(),
				AsValueLessThenNoCase());
			break;

		case as_array_object::fCaseInsensitive | as_array_object::fDescending:
			//log_msg("case insensitive descending");
			std::sort(elements.begin(), elements.end(),
				AsValueLessThenDescNoCase());
			break;

		case as_array_object::fNumeric: 
			//log_msg("numeric");
			std::sort(elements.begin(), elements.end(),
				AsValueLessThenNumeric());
			break;

		case as_array_object::fNumeric | as_array_object::fDescending:
			//log_msg("numeric descending");
			std::sort(elements.begin(), elements.end(),
				AsValueLessThenDescNumeric());
			break;

		default:
			log_error("Unhandled sort flags: %d (0x%X)", flags, flags);
			break;
	}

	// do the unique step afterwards to simplify code
	// (altought it's slower, but we can take care of this later)
	// TODO: use the do_unique variable inside the switch cases
	// to either use std::sort or std::uniq or similar
	if ( do_unique )
	{
		log_msg("Should unique now");
	}
}

void
as_array_object::sort(as_function& comparator, uint8_t flags)
{

	// use sorted_index to use this flag
	assert( ! (flags & as_array_object::fReturnIndexedArray) );

	// Other flags are simply NOT used
	// (or are them ? the descending one could be!)
	std::sort(elements.begin(), elements.end(),
		AsValueFuncComparator(comparator));

}

static as_array_object *
ensureArray(as_object* obj)
{
	as_array_object* ret = dynamic_cast<as_array_object*>(obj);
	if ( ! ret )
	{
		throw ActionException("builtin method or gettersetter for Array objects called against non-Array instance");
	}
	return ret;
}

static void
array_splice(const fn_call& fn)
{
	as_array_object* array = ensureArray(fn.this_ptr);
	UNUSED(array);

	log_error("FIXME: Array.splice() method not implemented yet!\n");
	fn.result->set_undefined();
}

static void
array_sort(const fn_call& fn)
{
	as_array_object* array = ensureArray(fn.this_ptr);

	uint8_t flags;

	if ( fn.nargs == 1 && fn.arg(0).is_number() )
	{
		flags=static_cast<uint8_t>(fn.arg(0).to_number());
	}
	else if ( fn.nargs == 0 )
	{
		flags=0;
	}
	else
	{
		log_error("Array.sort(comparator) method not implemented!\n");
		fn.result->set_undefined();
		return;
	}

	array->sort(flags);
	fn.result->set_undefined(); // returns void
	return;

}

static void
array_sortOn(const fn_call& fn)
{
	as_array_object* array = ensureArray(fn.this_ptr);
	UNUSED(array);

	log_error("FIXME: Array.sortOn() method not implemented yet!");
	fn.result->set_undefined();
}

// Callback to push values to the back of an array
static void
array_push(const fn_call& fn)
{
	as_array_object* array = ensureArray(fn.this_ptr);

		IF_VERBOSE_ACTION (
	log_action("calling array push, pushing %d values onto back of array",fn.nargs);
		);

	for (int i=0;i<fn.nargs;i++)
		array->push(fn.arg(i));

	fn.result->set_int(array->size());
}

// Callback to push values to the front of an array
static void
array_unshift(const fn_call& fn)
{
	as_array_object* array = ensureArray(fn.this_ptr);

		IF_VERBOSE_ACTION (
	log_action("calling array unshift, pushing %d values onto front of array",fn.nargs);
		);

	for (int i=fn.nargs-1;i>=0;i--)
		array->unshift(fn.arg(i));

	fn.result->set_int(array->size());
}

// Callback to pop a value from the back of an array
static void
array_pop(const fn_call& fn)
{
	as_array_object* array = ensureArray(fn.this_ptr);

	// Get our index, log, then return result
	(*fn.result) = array->pop();

	IF_VERBOSE_ACTION (
	log_action("calling array pop, result:%s, new array size:%d",
		fn.result->to_string(), array->size());
	);
}

// Callback to pop a value from the front of an array
static void
array_shift(const fn_call& fn)
{
	as_array_object* array = ensureArray(fn.this_ptr);

	// Get our index, log, then return result
	(*fn.result) = array->shift();

	IF_VERBOSE_ACTION (
	log_action("calling array shift, result:%s, new array size:%d",
		fn.result->to_string(), array->size());
	);
}

// Callback to reverse the position of the elements in an array
static void
array_reverse(const fn_call& fn)
{
	as_array_object* array = ensureArray(fn.this_ptr);

	array->reverse();

	fn.result->set_as_object(array);

	IF_VERBOSE_ACTION (
	log_action("called array reverse, result:%s, new array size:%d",
		fn.result->to_string(), array->size());
	);
	
}

// Callback to convert array to a string with optional custom separator (default ',')
static void
array_join(const fn_call& fn)
{
	as_array_object* array = ensureArray(fn.this_ptr);

	std::string separator = ",";

	if (fn.nargs > 0)
		separator = fn.arg(0).to_string();

	std::string ret = array->join(separator);

	fn.result->set_string(ret.c_str());
}

static void
array_size(const fn_call& fn)
{
	as_array_object* array = ensureArray(fn.this_ptr);

	fn.result->set_int(array->size());
}

// Callback to convert array to a string
// TODO CHECKME: rely on Object.toString  ? (
static void
array_to_string(const fn_call& fn)
{
	as_array_object* array = ensureArray(fn.this_ptr);

	std::string ret = array->toString();

		IF_VERBOSE_ACTION
		(
	log_action("array_to_string called, nargs = %d, "
			"this_ptr = %p",
			fn.nargs, (void*)fn.this_ptr);
	log_action("to_string result is: %s", ret.c_str());
		);

	fn.result->set_string(ret.c_str());
}

/// concatenates the elements specified in the parameters with
/// the elements in my_array, and creates a new array. If the
/// value parameters specify an array, the elements of that
/// array are concatenated, rather than the array itself. The
/// array my_array is left unchanged.
static void
array_concat(const fn_call& fn)
{
	as_array_object* array = ensureArray(fn.this_ptr);

	// use copy ctor
	as_array_object* newarray = new as_array_object(*array);

	for (int i=0; i<fn.nargs; i++)
	{
		// Array args get concatenated by elements
		if ( as_array_object* other = dynamic_cast<as_array_object*>(fn.arg(i).to_object()) )
		{
			assert(other);
			newarray->concat(*other);
		}
		else
		{
			newarray->push(fn.arg(i));
		}
	}

	fn.result->set_as_object(newarray);		
}

// Callback to slice part of an array to a new array
// without changing the original
static void
array_slice(const fn_call& fn)
{
	as_array_object* array = ensureArray(fn.this_ptr);

	// start and end index of the part we're slicing
	int startindex, endindex;

	if (fn.nargs > 2)
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_warning("More than 2 arguments sent to slice, "
			"and I don't know what to do with them!\n"
			"Ignoring them as we continue...\n");
		);
	}

	// They passed no arguments: simply duplicate the array
	// and return the new one
	if (fn.nargs < 1)
	{
		as_array_object* newarray = new as_array_object(*array);
		fn.result->set_as_object(newarray);
		return;
	}


	startindex = int(fn.arg(0).to_number());

	// if the index is negative, it means "places from the end"
	// where -1 is the last element
	if (startindex < 0) startindex = startindex + array->size();

	// if we sent at least two arguments, setup endindex
	if (fn.nargs >= 2)
	{
		endindex = int(fn.arg(1).to_number());

		// if the index is negative, it means
		// "places from the end" where -1 is the last element
		if (endindex < 0) endindex = endindex + array->size();
	}
	else
	{
		// They didn't specify where to end,
		// so choose the end of the array
		endindex = array->size();
	}

	if ( startindex < 0 ) startindex = 0;
	else if ( static_cast<size_t>(startindex)  > array->size() ) startindex = array->size();

	if ( endindex < 1 ) endindex = 1;
	else if ( static_cast<size_t>(endindex)  > array->size() ) endindex = array->size();

	std::auto_ptr<as_array_object> newarray(array->slice(
		startindex, endindex));

	fn.result->set_as_object(newarray.release());		

}

void
array_new(const fn_call& fn)
{
	IF_VERBOSE_ACTION (
		log_action("array_new called, nargs = %d", fn.nargs);
	);

	//boost::intrusive_ptr<as_array_object>	ao = new as_array_object;
	as_array_object* ao = new as_array_object;

	if (fn.nargs == 0)
	{
		// Empty array.
	}
	else if (fn.nargs == 1 && fn.arg(0).is_number() )
	{
		// Create an empty array with the given number of undefined elements.
		//
		as_value	index_number;
		as_value null_value;
		null_value.set_null();
		for (int i = 0; i < int(fn.arg(0).to_number()); i++)
		{
			index_number.set_int(i);
			ao->set_member(index_number.to_string(), null_value);
		}
	}
	else
	{
		// Use the arguments as initializers.
		as_value	index_number;
		for (int i = 0; i < fn.nargs; i++)
		{
			ao->push(fn.arg(i));
		}
	}

	IF_VERBOSE_ACTION (
		log_action("array_new setting object %p in result", (void*)ao);
	);

	//fn.result->set_as_object(ao.get());
	fn.result->set_as_object(ao);
}

static void
attachArrayInterface(as_object& proto)
{
	// we don't need an explicit member here,
	// we will be handling 'length' requests
	// within overridden get_member()
	//proto->init_member("length", &array_length);

	proto.init_member("join", new builtin_function(array_join));
	proto.init_member("concat", new builtin_function(array_concat));
	proto.init_member("slice", new builtin_function(array_slice));
	proto.init_member("push", new builtin_function(array_push));
	proto.init_member("unshift", new builtin_function(array_unshift));
	proto.init_member("pop", new builtin_function(array_pop));
	proto.init_member("shift", new builtin_function(array_shift));
	proto.init_member("splice", new builtin_function(array_splice));
	proto.init_member("sort", new builtin_function(array_sort));
	proto.init_member("size", new builtin_function(array_size));
	proto.init_member("sortOn", new builtin_function(array_sortOn));
	proto.init_member("reverse", new builtin_function(array_reverse));
	proto.init_member("toString", new builtin_function(array_to_string));

	proto.init_member("CASEINSENSITIVE", as_array_object::fCaseInsensitive);
	proto.init_member("DESCENDING", as_array_object::fDescending);
	proto.init_member("UNIQUESORT", as_array_object::fUniqueSort);
	proto.init_member("RETURNINDEXEDARRAY", as_array_object::fReturnIndexedArray);
	proto.init_member("NUMERIC", as_array_object::fNumeric);
}

static as_object*
getArrayInterface()
{
	static boost::intrusive_ptr<as_object> proto = NULL;
	if ( proto == NULL )
	{
		proto = new as_object();
		attachArrayInterface(*proto);
	}
	return proto.get();
}

// this registers the "Array" member on a "Global"
// object. "Array" is a constructor, thus an object
// with .prototype full of exported functions + 
// 'constructor'
//
void
array_class_init(as_object& glob)
{
	// This is going to be the global Array "class"/"function"
	static boost::intrusive_ptr<as_function> ar=NULL;

	if ( ar == NULL )
	{
		ar = new builtin_function(&array_new, getArrayInterface());

		// We replicate interface to the Array class itself
		attachArrayInterface(*ar);
	}

	// Register _global.Array
	glob.init_member("Array", ar.get());
}


} // end of gnash namespace


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

