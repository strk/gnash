// array.cpp:  ActionScript array class, for Gnash.
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
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "as_value.h"
#include "array.h"
#include "log.h"
#include "builtin_function.h" // for Array class
#include "as_function.h" // for sort user-defined comparator
#include "fn_call.h"
#include "GnashException.h"
#include "action.h" // for call_method

#include <string>
#include <algorithm>
#include <memory> // for auto_ptr
#include <boost/algorithm/string/case_conv.hpp>

//#define GNASH_DEBUG 


namespace gnash {

static as_object* getArrayInterface();
static void attachArrayProperties(as_object& proto);
static void attachArrayInterface(as_object& proto);

// Default as_value strict weak comparator (string based)
class AsValueLessThen
{
public:
	bool operator() (const as_value& a, const as_value& b)
	{
		return a.to_string().compare(b.to_string()) < 0;
	}
};

// Default descending as_value strict weak comparator (string based)
class AsValueLessThenDesc
{
public:
	bool operator() (const as_value& a, const as_value& b)
	{
		return a.to_string().compare(b.to_string()) > 0;
	}
};

// Case-insensitive as_value strict weak comparator (string)
class AsValueLessThenNoCase
{
public:
	bool operator() (const as_value& a, const as_value& b)
	{
		using namespace boost::algorithm;

		std::string strA = to_upper_copy(a.to_string());
		std::string strB = to_upper_copy(b.to_string());

		return strA.compare(strB) < 0;
	}
};

// Descending Case-insensitive as_value strict weak comparator (string)
class AsValueLessThenDescNoCase
{
public:
	bool operator() (const as_value& a, const as_value& b)
	{
		using namespace boost::algorithm;

		std::string strA = to_upper_copy(a.to_string());
		std::string strB = to_upper_copy(b.to_string());

		return strA.compare(strB) > 0;
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
	as_object* _object;

	AsValueFuncComparator(as_function& comparator, boost::intrusive_ptr<as_object> this_ptr)
		:
		_comp(comparator)
	{
		_object = this_ptr.get();
	}

	bool operator() (const as_value& a, const as_value& b)
	{
		as_value cmp_method(&_comp);
		as_environment env;
		as_value ret(0);
		int retval;

		env.push(a);
		env.push(b);
		ret = call_method(cmp_method, &env, _object, 2, 1);
		retval = (int)ret.to_number();
		if (retval > 0) return true;
		return false;
	}
};

// @@ TODO : implement as_array_object's unimplemented functions

as_array_object::as_array_object()
	:
	as_object(getArrayInterface()), // pass Array inheritance
	elements(0)
{
	//IF_VERBOSE_ACTION (
	//log_action("%s: %p", __FUNCTION__, (void*)this);
	//)
	attachArrayProperties(*this);
}

as_array_object::as_array_object(const as_array_object& other)
	:
	as_object(other),
	elements(other.elements)
{
    //IF_VERBOSE_ACTION (
    //log_action("%s: %p", __FUNCTION__, (void*)this);
    //)
}

as_array_object::~as_array_object() 
{
}

int
as_array_object::index_requested(const std::string& name)
{
	as_value temp;
	temp.set_string(name);
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
as_array_object::push(const as_value& val)
{
	elements.push_back(val);
}

void
as_array_object::unshift(const as_value& val)
{
	elements.push_front(val);
}

as_value
as_array_object::pop()
{
	// If the array is empty, report an error and return undefined!
	if (elements.size() <= 0)
	{
	    log_error(_("tried to pop element from back of empty array, returning undef"));
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
		log_error(_("tried to shift element from front of empty array, returning undef"));
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
as_array_object::join(const std::string& separator, as_environment* env) const
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
		temp += (*it++).to_string(env);

		// print subsequent elements with separator prefix
		while ( it != itEnd )
		{
			temp += separator + (*it++).to_string(env);
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
as_array_object::toString(as_environment* env) const
{
	return join(",", env);
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

#ifdef GNASH_DEBUG
	log_msg(_("Array.slice(%u, %u) called"), start, one_past_end);
#endif

	size_t newsize = one_past_end - start;
	newarray->elements.resize(newsize);

	// maybe there's a standard algorithm for this ?
	for (unsigned int i=start; i<one_past_end; ++i)
	{
		newarray->elements[i-start] = elements[i];
	}

	return newarray;

}

std::auto_ptr<as_array_object>
as_array_object::splice(unsigned start, unsigned len,
		const std::vector<as_value>& replace)
{
	assert(len <= size()-start);
	assert(start <= size());

#ifdef GNASH_DEBUG
	std::stringstream ss;
	ss << "Array.splice(" << start << ", " << len << ", ";
	std::ostream_iterator<as_value> ostrIter(ss, "," ) ;
	std::copy(replace.begin(), replace.end(), ostrIter);
        ss << ") called";
	log_msg("%s", ss.str().c_str());
	log_msg(_("Current array is %s"), toString().c_str());
#endif

	container::iterator itStart = elements.begin()+start;
	container::iterator itEnd = itStart+len;

	// This will be returned...
	std::auto_ptr<as_array_object> ret(new as_array_object);
	
	// If something has to be removed do it and assign
	// to the returned object
	if ( itStart != itEnd )
	{
		ret->elements.assign(itStart, itEnd);

		elements.erase(itStart, itEnd);
	}

	// Now insert the new stuff, if needed
	if ( replace.size() )
	{
		container::iterator itStart = elements.begin()+start;
		elements.insert(itStart, replace.begin(), replace.end());
	}

	return ret;
}

/* virtual public, overriding as_object::get_member */
bool
as_array_object::get_member(const std::string& name, as_value *val)
{
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
	log_unimpl("Array.sorted_index");
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
			//log_msg(_("Default sorting"));
			std::sort(elements.begin(), elements.end(),
				AsValueLessThen());
			break;

		case as_array_object::fDescending:
			//log_msg(_("Default descending"));
			std::sort(elements.begin(), elements.end(),
				AsValueLessThenDesc());
			break;

		case as_array_object::fCaseInsensitive: 
			//log_msg(_("case insensitive"));
			std::sort(elements.begin(), elements.end(),
				AsValueLessThenNoCase());
			break;

		case as_array_object::fCaseInsensitive | as_array_object::fDescending:
			//log_msg(_("case insensitive descending"));
			std::sort(elements.begin(), elements.end(),
				AsValueLessThenDescNoCase());
			break;

		case as_array_object::fNumeric: 
			//log_msg(_("numeric"));
			std::sort(elements.begin(), elements.end(),
				AsValueLessThenNumeric());
			break;

		case as_array_object::fNumeric | as_array_object::fDescending:
			//log_msg(_("numeric descending"));
			std::sort(elements.begin(), elements.end(),
				AsValueLessThenDescNumeric());
			break;

		default:
			log_error(_("Unhandled sort flags: %d (0x%X)"), flags, flags);
			break;
	}

	// do the unique step afterwards to simplify code
	// (altought it's slower, but we can take care of this later)
	// TODO: use the do_unique variable inside the switch cases
	// to either use std::sort or std::uniq or similar
	if ( do_unique )
	{
		log_unimpl(_("array.sort with unique flag"));
	}
}

void
as_array_object::sort(as_function& comparator, boost::intrusive_ptr<as_object> this_ptr, uint8_t flags)
{

	// use sorted_index to use this flag
	assert( ! (flags & as_array_object::fReturnIndexedArray) );

	// Other flags are simply NOT used
	// (or are them ? the descending one could be!)
	std::sort(elements.begin(), elements.end(),
		AsValueFuncComparator(comparator, this_ptr));

}

static as_value
array_splice(const fn_call& fn)
{
	boost::intrusive_ptr<as_array_object> array = ensureType<as_array_object>(fn.this_ptr);

#ifdef GNASH_DEBUG
	std::stringstream ss;
	fn.dump_args(ss);
	log_msg(_("Array(%s).splice(%s) called"), array->toString().c_str(), ss.str().c_str());
#endif

	if (fn.nargs < 1)
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Array.splice() needs at least 1 argument, call ignored"));
		);
		return as_value();
	}

	unsigned origlen = array->size();

	//----------------
	// Get start offset
	//----------------
	unsigned startoffset;
	int start = fn.arg(0).to_number<int>(&(fn.env()));
	if ( start < 0 ) start = array->size()+start; // start is negative, so + means -abs()
	startoffset = iclamp(start, 0, origlen);
#ifdef GNASH_DEBUG
	if ( startoffset != start )
		log_msg(_("Array.splice: start:%d became %u"), start, startoffset);
#endif

	//----------------
	// Get length
	//----------------
	unsigned len = 0;
	if (fn.nargs > 1)
	{
		int lenval = fn.arg(1).to_number<int>(&(fn.env()));
		if ( lenval < 0 )
		{
			IF_VERBOSE_ASCODING_ERRORS(
			log_aserror(_("Array.splice(%d,%d): negative length given, call ignored"),
				start, lenval);
			);
			return as_value();
		}
		len = iclamp(lenval, 0, origlen-startoffset);
	}

	//----------------
	// Get replacement
	//----------------
	std::vector<as_value> replace;
	for (unsigned i=2; i<fn.nargs; ++i)
	{
		replace.push_back(fn.arg(i));
	}

	std::auto_ptr<as_array_object> spliced ( array->splice(startoffset, len, replace) );

	boost::intrusive_ptr<as_object> ret = spliced.release();

	return as_value(ret);
}

static as_value
array_sort(const fn_call& fn)
{
	boost::intrusive_ptr<as_array_object> array = ensureType<as_array_object>(fn.this_ptr);

	uint8_t flags = 0;

	if ( fn.nargs == 0 )
	{
		array->sort(flags);
	}
	else if ( fn.nargs == 1 && fn.arg(0).is_number() )
	{
		flags=static_cast<uint8_t>(fn.arg(0).to_number());
		array->sort(flags);
	}
	else if ( fn.arg(0).is_as_function() )
	{
		// Get comparison function
		as_function* as_func = fn.arg(0).to_as_function();
	
		if ( fn.nargs == 2 && fn.arg(1).is_number() )
		{
			flags=static_cast<uint8_t>(fn.arg(1).to_number());
		}
		array->sort(*as_func, fn.this_ptr, flags);
	}
	else
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Sort called with invalid arguments."));
		)
	}

	return as_value(); // returns void
}

static as_value
array_sortOn(const fn_call& fn)
{
	boost::intrusive_ptr<as_array_object> array = ensureType<as_array_object>(fn.this_ptr);
	UNUSED(array);

	log_unimpl("Array.sortOn()");
	return as_value();
}

// Callback to push values to the back of an array
static as_value
array_push(const fn_call& fn)
{
	boost::intrusive_ptr<as_array_object> array = ensureType<as_array_object>(fn.this_ptr);

		IF_VERBOSE_ACTION (
	log_action(_("calling array push, pushing %d values onto back of array"),fn.nargs);
		);

	for (unsigned int i=0;i<fn.nargs;i++)
		array->push(fn.arg(i));

	return as_value(array->size());
}

// Callback to push values to the front of an array
static as_value
array_unshift(const fn_call& fn)
{
	boost::intrusive_ptr<as_array_object> array = ensureType<as_array_object>(fn.this_ptr);

		IF_VERBOSE_ACTION (
	log_action(_("calling array unshift, pushing %d values onto front of array"), fn.nargs);
		);

	for (int i=fn.nargs-1; i>=0; i--)
		array->unshift(fn.arg(i));

	return as_value(array->size());
}

// Callback to pop a value from the back of an array
static as_value
array_pop(const fn_call& fn)
{
	boost::intrusive_ptr<as_array_object> array = ensureType<as_array_object>(fn.this_ptr);

	// Get our index, log, then return result
	as_value rv = array->pop();

	IF_VERBOSE_ACTION (
	log_action(_("calling array pop, result:%s, new array size:%d"),
		rv.to_string().c_str(), array->size());
	);
        return rv;
}

// Callback to pop a value from the front of an array
static as_value
array_shift(const fn_call& fn)
{
	boost::intrusive_ptr<as_array_object> array = ensureType<as_array_object>(fn.this_ptr);

	// Get our index, log, then return result
	as_value rv = array->shift();

	IF_VERBOSE_ACTION (
	log_action(_("calling array shift, result:%s, new array size:%d"),
		rv.to_string().c_str(), array->size());
	);
	return rv;
}

// Callback to reverse the position of the elements in an array
static as_value
array_reverse(const fn_call& fn)
{
	boost::intrusive_ptr<as_array_object> array = ensureType<as_array_object>(fn.this_ptr);

	array->reverse();

	as_value rv(array.get()); 

	IF_VERBOSE_ACTION (
	log_action(_("called array reverse, result:%s, new array size:%d"),
		rv.to_string().c_str(), array->size());
	);
	return rv;
}

// Callback to convert array to a string with optional custom separator (default ',')
static as_value
array_join(const fn_call& fn)
{
	boost::intrusive_ptr<as_array_object> array = ensureType<as_array_object>(fn.this_ptr);

	std::string separator = ",";

	if (fn.nargs > 0)
		separator = fn.arg(0).to_string();

	std::string ret = array->join(separator, &(fn.env()));

	return as_value(ret.c_str());
}

static as_value
array_size(const fn_call& fn)
{
	boost::intrusive_ptr<as_array_object> array = ensureType<as_array_object>(fn.this_ptr);

	return as_value(array->size());
}

// Callback to convert array to a string
// TODO CHECKME: rely on Object.toString  ? (
static as_value
array_to_string(const fn_call& fn)
{
	boost::intrusive_ptr<as_array_object> array = ensureType<as_array_object>(fn.this_ptr);

	std::string ret = array->toString(&(fn.env()));

		IF_VERBOSE_ACTION
		(
	log_action(_("array_to_string called, nargs = %d, "
			"this_ptr = %p"),
			fn.nargs, (void*)fn.this_ptr.get());
	log_action(_("to_string result is: %s"), ret.c_str());
		);

	return as_value(ret.c_str());
}

/// concatenates the elements specified in the parameters with
/// the elements in my_array, and creates a new array. If the
/// value parameters specify an array, the elements of that
/// array are concatenated, rather than the array itself. The
/// array my_array is left unchanged.
static as_value
array_concat(const fn_call& fn)
{
	boost::intrusive_ptr<as_array_object> array = ensureType<as_array_object>(fn.this_ptr);

	// use copy ctor
	as_array_object* newarray = new as_array_object(*array);

	for (unsigned int i=0; i<fn.nargs; i++)
	{
		// Array args get concatenated by elements
		boost::intrusive_ptr<as_array_object> other = boost::dynamic_pointer_cast<as_array_object>(fn.arg(i).to_object());
		if ( other )
		{
			newarray->concat(*other);
		}
		else
		{
			newarray->push(fn.arg(i));
		}
	}

	return as_value(newarray);		
}

// Callback to slice part of an array to a new array
// without changing the original
static as_value
array_slice(const fn_call& fn)
{
	boost::intrusive_ptr<as_array_object> array = ensureType<as_array_object>(fn.this_ptr);

	// start and end index of the part we're slicing
	int startindex, endindex;

	if (fn.nargs > 2)
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("More than 2 arguments to Array.slice, "
			"and I don't know what to do with them.  "
			"Ignoring them"));
		);
	}

	// They passed no arguments: simply duplicate the array
	// and return the new one
	if (fn.nargs < 1)
	{
		as_array_object* newarray = new as_array_object(*array);
		return as_value(newarray);
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

	return as_value(newarray.release());		

}

static as_value
array_length(const fn_call& fn)
{
	boost::intrusive_ptr<as_array_object> array = ensureType<as_array_object>(fn.this_ptr);

	if ( fn.nargs ) // setter
	{
		array->resize(unsigned(fn.arg(0).to_number(&(fn.env()))));
		return as_value();
	}
	else // getter
	{
		return as_value(array->size());
	}
}

as_value
array_new(const fn_call& fn)
{
	IF_VERBOSE_ACTION (
		log_action(_("array_new called, nargs = %d"), fn.nargs);
	);

	boost::intrusive_ptr<as_array_object>	ao = new as_array_object;

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
			ao->set_member(index_number.to_string().c_str(), null_value);
		}
	}
	else
	{
		// Use the arguments as initializers.
		as_value	index_number;
		for (unsigned int i = 0; i < fn.nargs; i++)
		{
			ao->push(fn.arg(i));
		}
	}

	IF_VERBOSE_ACTION (
		log_action(_("array_new setting object %p in result"), (void*)ao.get());
	);

	return as_value(ao.get());
	//return as_value(ao);
}

static void
attachArrayProperties(as_object& proto)
{
	boost::intrusive_ptr<builtin_function> gettersetter;

	gettersetter = new builtin_function(&array_length, NULL);
	proto.init_property("length", *gettersetter, *gettersetter);
}

static void
attachArrayInterface(as_object& proto)
{

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

#ifdef GNASH_USE_GC
void
as_array_object::markReachableResources() const
{
	for (container::const_iterator i=elements.begin(), e=elements.end(); i!=e; ++i)
	{
		i->setReachable();
	}
	markAsObjectReachable();
}
#endif // GNASH_USE_GC

} // end of gnash namespace


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

