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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// This needs to be included first for NetBSD systems or we get a weird
// problem with pthread_t being defined too many times if we use any
// STL containers.
#ifdef HAVE_PTHREADS
#include <pthread.h>
#endif

#include <string>
#include <algorithm>
#include <memory> // for auto_ptr

#include "array.h"
#include "action.h"
#include "log.h"
#include "Function.h" // for Array class

namespace gnash {

static as_object* getArrayInterface();

// @@ TODO : implement as_array_object's unimplemented functions

	as_array_object::as_array_object()
		:
		as_object(getArrayInterface()), // pass Array inheritance
		elements(0)
	{
    		IF_VERBOSE_ACTION(
			log_msg("%s : %p\n", __FUNCTION__, this)
		);
	}

	as_array_object::as_array_object(const as_array_object& other)
		:
		as_object(other),
		elements(other.elements)
	{
    		IF_VERBOSE_ACTION(
    			log_msg("%s : %p\n", __FUNCTION__, this)
		);
	}

	int as_array_object::index_requested(const tu_stringi& name)
	{
		double value;
		as_value temp;
		temp.set_string(name.c_str());
		value = temp.to_number();

		// if we were sent a string that can't convert like "asdf", it returns as NaN. -1 means invalid index
		if (isnan(value)) return -1;

		// TODO / WARNING: because to_number returns a double and we're converting to an int,
		// I want to make sure we're above any "grey area" when we we round down
		// by adding a little to the number before we round it. We don't want to accidentally look to index-1!
		return int(value + 0.01);
	}

	void as_array_object::push(as_value& val)
	{
		elements.push_back(val);
	}

	void as_array_object::unshift(as_value& val)
	{
		elements.push_front(val);
	}

	as_value as_array_object::pop()
	{
		// If the array is empty, report an error and return undefined!
		if (elements.size() <= 0)
		{
			IF_VERBOSE_ACTION(log_error("tried to pop element from back of empty array!\n"));
			return as_value(); // undefined
		}

		as_value ret = elements.back();
		elements.pop_back();

		return ret;
	}

	as_value as_array_object::shift()
	{
		// If the array is empty, report an error and return undefined!
		if (elements.size() <= 0)
		{
			IF_VERBOSE_ACTION(log_error("tried to shift element from front of empty array!\n"));
			return as_value(); // undefined
		}

		as_value ret = elements.front();
		elements.pop_front();

		return ret;
	}

	void as_array_object::reverse()
	{
		// Reverse the deque elements
		std::reverse(elements.begin(), elements.end());
	}

	std::string as_array_object::join(const std::string& separator)
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

	void as_array_object::concat(const as_array_object& other)
	{
		elements.insert(elements.end(), other.elements.begin(),
			other.elements.end());
	}

	std::string as_array_object::toString()
	{
		return join(",");
	}

	unsigned int as_array_object::size() const
	{
		return elements.size();
	}

#if 0
	void as_array_object::resize(unsigned int newsize)
	{
		elements.resize(newsize);
	}
#endif

	as_value as_array_object::at(unsigned int index)
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
		std::auto_ptr<as_array_object> newarray(new as_array_object);
		newarray->elements.resize(one_past_end - start - 1);

		// maybe there's a standard algorithm for this ?
		for (unsigned int i=start; i<one_past_end; ++i)
		{
			newarray->elements[i-start] = elements[i];
		}

		return newarray;

	}


	/* virtual public, overriding as_object::set_member */
	bool as_array_object::get_member(const tu_stringi& name, as_value *val)
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

	/* virtual public, overriding as_object::set_member */
	void as_array_object::set_member(const tu_stringi& name,
			const as_value& val )
	{
		if ( name == "length" ) 
		{
			IF_VERBOSE_ACTION(log_msg("assigning to Array.length unsupported"));
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


	// Callback for unimplemented functions
	void	array_not_impl(const fn_call& fn)
	{
		assert(dynamic_cast<as_array_object*>(fn.this_ptr));
		//as_array_object* array = static_cast<as_array_object*>(fn.this_ptr);

		IF_VERBOSE_ACTION(log_error("array method not implemented yet!\n"));
	}

	// Callback to report array length
	void array_length(const fn_call& fn)
	{
		assert(dynamic_cast<as_array_object*>(fn.this_ptr));
		as_array_object* array = \
			static_cast<as_array_object*>(fn.this_ptr);

		IF_VERBOSE_ACTION(log_msg("calling array length, result:%d\n",array->size()));

		fn.result->set_int(array->size());
	}

	// Callback to push values to the back of an array
	void array_push(const fn_call& fn)
	{
		assert(dynamic_cast<as_array_object*>(fn.this_ptr));
		as_array_object* array = \
			static_cast<as_array_object*>(fn.this_ptr);

		IF_VERBOSE_ACTION(log_msg("calling array push, pushing %d values onto back of array\n",fn.nargs));

		for (int i=0;i<fn.nargs;i++)
			array->push(fn.arg(i));

		fn.result->set_int(array->size());
	}

	// Callback to push values to the front of an array
	void array_unshift(const fn_call& fn)
	{
		assert(dynamic_cast<as_array_object*>(fn.this_ptr));
		as_array_object* array = \
			static_cast<as_array_object*>(fn.this_ptr);

		IF_VERBOSE_ACTION(log_msg("calling array unshift, pushing %d values onto front of array\n",fn.nargs));

		for (int i=fn.nargs-1;i>=0;i--)
			array->unshift(fn.arg(i));

		fn.result->set_int(array->size());
	}

	// Callback to pop a value from the back of an array
	void array_pop(const fn_call& fn)
	{
		assert(dynamic_cast<as_array_object*>(fn.this_ptr));
		as_array_object* array = \
			static_cast<as_array_object*>(fn.this_ptr);

		// Get our index, log, then return result
		(*fn.result) = array->pop();
		IF_VERBOSE_ACTION(log_msg("calling array pop, result:%s, new array size:%zd\n",fn.result->to_string(),array->size()));
	}

	// Callback to pop a value from the front of an array
	void array_shift(const fn_call& fn)
	{
		assert(dynamic_cast<as_array_object*>(fn.this_ptr));
		as_array_object* array = \
			static_cast<as_array_object*>(fn.this_ptr);

		// Get our index, log, then return result
		(*fn.result) = array->shift();
		IF_VERBOSE_ACTION(log_msg("calling array shift, result:%s, new array size:%zd\n",fn.result->to_string(),array->size()));
	}

	// Callback to reverse the position of the elements in an array
	void array_reverse(const fn_call& fn)
	{
		assert(dynamic_cast<as_array_object*>(fn.this_ptr));
		as_array_object* array = \
			static_cast<as_array_object*>(fn.this_ptr);

		array->reverse();

		fn.result->set_as_object(array);

		IF_VERBOSE_ACTION(log_msg("called array reverse, result:%s, new array size:%zd\n",fn.result->to_string(),array->size()));
		
	}

	// Callback to convert array to a string with optional custom separator (default ',')
	void array_join(const fn_call& fn)
	{
		assert(dynamic_cast<as_array_object*>(fn.this_ptr));
		as_array_object* array = \
			static_cast<as_array_object*>(fn.this_ptr);

		std::string separator = ",";

		if (fn.nargs > 0)
			separator = fn.arg(0).to_string();

		std::string ret = array->join(separator);

		fn.result->set_string(ret.c_str());
	}

	// Callback to convert array to a string
	void array_to_string(const fn_call& fn)
	{
		IF_VERBOSE_ACTION(
			log_msg("array_to_string called, nargs = %d, "
				"this_ptr = %p",
				fn.nargs, fn.this_ptr)
		);

		assert(dynamic_cast<as_array_object*>(fn.this_ptr));
		as_array_object* array = \
			static_cast<as_array_object*>(fn.this_ptr);

		std::string ret = array->toString();

		IF_VERBOSE_ACTION(
			log_msg("to_string result is: %s", ret.c_str())
		);

		fn.result->set_string(ret.c_str());
	}

	/// concatenates the elements specified in the parameters with
	/// the elements in my_array, and creates a new array. If the
	/// value parameters specify an array, the elements of that
	/// array are concatenated, rather than the array itself. The
	/// array my_array is left unchanged.
	void array_concat(const fn_call& fn)
	{
		assert(dynamic_cast<as_array_object*>(fn.this_ptr));
		as_array_object* array = \
			static_cast<as_array_object*>(fn.this_ptr);

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
	void array_slice(const fn_call& fn)
	{
		assert(dynamic_cast<as_array_object*>(fn.this_ptr));
		as_array_object* array = \
			static_cast<as_array_object*>(fn.this_ptr);

		// start and end index of the part we're slicing
		int startindex, endindex;

		if (fn.nargs > 2)
		{
			IF_VERBOSE_ACTION(log_error("More than 2 arguments sent to slice, and I don't know what to do with them!\n"));
			IF_VERBOSE_ACTION(log_error("Ignoring them as we continue...\n"));
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
		// if it's still negative, this is a problem
		if (startindex < 0 || (unsigned int)startindex > array->size())
		{
			IF_VERBOSE_ACTION(log_error("bad startindex sent to array_slice! startindex: %s, Length: %zd",
				fn.arg(0).to_string(),array->size()));
			return;				
		}
		// if we sent at least two arguments, setup endindex
		if (fn.nargs >= 2)
		{
			endindex = int(fn.arg(1).to_number());
			// if the index is negative, it means
			// "places from the end" where -1 is the last element
			if (endindex < 0) endindex = endindex + array->size();
			// the endindex is non-inclusive, so add 1
			endindex++;
			if (endindex < 0)
			{
				IF_VERBOSE_ACTION(log_error("bad endindex sent to array_slice! endindex: %s, length: %zd",
					fn.arg(1).to_string(),array->size()));
				return;				
			}
			// If they overshoot the end of the array,
			// just copy to the end
			if ((unsigned int)endindex > array->size() + 1)
				endindex = array->size() + 1;
		}
		else
		{
			// They didn't specify where to end, so choose the end of the array
			endindex = array->size() + 1;
		}

		std::auto_ptr<as_array_object> newarray(array->slice(
			startindex, endindex));

		fn.result->set_as_object(newarray.release());		

	}

	void	array_new(const fn_call& fn)
	{
		IF_VERBOSE_ACTION(log_msg("array_new called, nargs = %d", fn.nargs));

		//smart_ptr<as_array_object>	ao = new as_array_object;
		as_array_object* ao = new as_array_object;

		if (fn.nargs == 0)
		{
			// Empty array.
		}
		else if (fn.nargs == 1
			 && fn.arg(0).get_type() == as_value::NUMBER)
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

		IF_VERBOSE_ACTION(
			log_msg("array_new setting object %p in result", ao)
		);

		//fn.result->set_as_object(ao.get_ptr());
		fn.result->set_as_object(ao);
	}

static void
attachArrayInterface(as_object* proto)
{
	// we don't need an explicit member here,
	// we will be handling 'length' requests
	// within overridden get_member()
	//proto->set_member("length", &array_length);

	proto->set_member("join", &array_join);
	proto->set_member("concat", &array_concat);
	proto->set_member("slice", &array_slice);
	proto->set_member("push", &array_push);
	proto->set_member("unshift", &array_unshift);
	proto->set_member("pop", &array_pop);
	proto->set_member("shift", &array_shift);
	proto->set_member("splice", &array_not_impl);
	proto->set_member("sort", &array_not_impl);
	proto->set_member("sortOn", &array_not_impl);
	proto->set_member("reverse", &array_reverse);
	proto->set_member("toString", &array_to_string);
	proto->set_member("CASEINSENSITIVE", 1);
	proto->set_member("DESCENDING", 2);
	proto->set_member("UNIQUESORT", 4);
	proto->set_member("RETURNINDEXEDARRAY", 8);
	proto->set_member("NUMERIC", 16);
}

static as_object*
getArrayInterface()
{
	static as_object* proto = NULL;
	if ( proto == NULL )
	{
		proto = new as_object();
		attachArrayInterface(proto);
		proto->set_member("constructor", &array_new); 
		proto->set_member_flags("constructor", 1);
	}
	return proto;
}

// this registers the "Array" member on a "Global"
// object. "Array" is a constructor, thus an object
// with .prototype full of exported functions + 
// 'constructor'
//
void
array_init(as_object* glob)
{
	// This is going to be the global Array "class"/"function"
	static function_as_object* ar=NULL;

	if ( ar == NULL )
	{
		ar = new function_as_object(getArrayInterface());

		// We replicate interface to the Array class itself
		attachArrayInterface(ar);

	}

	// Register _global.Array
	glob->set_member("Array", ar);
}


};
