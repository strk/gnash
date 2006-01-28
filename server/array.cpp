// 
//   Copyright (C) 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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

#include "array.h"
#include "action.h"
#include "log.h"

namespace gnash {

// @@ TODO : implement as_array_object's unimplemented functions

	as_array_object::as_array_object()
		: elements(0)
	{
		array_init(this);
	}

	const int as_array_object::size() const
	{
		return elements.size();
	}
	
	int as_array_object::index_requested(const tu_stringi& name)
	{
		double value;
		as_value temp;
		temp.set_string((const char *)name);
		value = temp.to_number();

		// if we were sent a string that can't convert like "asdf", it returns as NaN. -1 means invalid index
		if (isnan(value)) return -1;

		// TODO / WARNING: because to_number returns a double and we're converting to an int,
		// I want to make sure we're above any "grey area" when we we round down
		// by adding a little to the number before we round it. We don't want to accidentally look to index-1!
		return int(value + 0.01);
	}

	void as_array_object::set_member(const tu_stringi& name, const as_value& val )
	{
		int index = index_requested(name);

		if (index >= 0) // if we were sent a valid array index and not a normal member
		{
			if (index >= int(elements.size()))
				elements.resize(index+1); // if we're setting index (x), the vector must be size (x+1)

			// set the appropriate index and return
			elements[index] = val;
			return;
		}

		as_object::set_member(name,val);
	}

	bool as_array_object::get_member(const tu_stringi& name, as_value *val)
	{
		int index = index_requested(name);

		// if we found an array index
		if (index >= 0)
		{
			// let's return false if it exceeds the bounds of num_elements
			if (index >= int(elements.size()))
				return false;
			// otherwise return the appropriate array index from the vector
			else
			{
				*val = elements[index];
				return true;
			}
		}

		// We're past the array-specific stuff - let's proceed to the regular as_object::get_member() for default behavior
		return as_object::get_member(name,val);
	}

	// Callback for unimplemented functions
	void	array_not_impl(const fn_call& fn)
	{
		IF_VERBOSE_ACTION(log_error("array method not implemented yet!\n"));
	}

	// Callback to report array length
	void array_length(const fn_call& fn)
	{
		as_array_object* array = (as_array_object*) (as_object*) fn.this_ptr;
		IF_VERBOSE_ACTION(log_msg("calling array length, result:%d\n",array->size()));

		fn.result->set_int(array->size());
	}

	// Callback to push values to the back of an array
	void array_push(const fn_call& fn)
	{
		as_array_object* array = (as_array_object*) (as_object*) fn.this_ptr;
		IF_VERBOSE_ACTION(log_msg("calling array push, pushing %d values onto back of array\n",fn.nargs));

		for (int i=0;i<fn.nargs;i++)
			array->elements.push_back(fn.arg(i));

		fn.result->set_int(array->size());
	}

	// Callback to push values to the front of an array
	void array_unshift(const fn_call& fn)
	{
		as_array_object* array = (as_array_object*) (as_object*) fn.this_ptr;
		IF_VERBOSE_ACTION(log_msg("calling array unshift, pushing %d values onto front of array\n",fn.nargs));

		for (int i=fn.nargs-1;i>=0;i--)
			array->elements.push_front(fn.arg(i));

		fn.result->set_int(array->size());
	}

	// Callback to pop a value from the back of an array
	void array_pop(const fn_call& fn)
	{
		as_array_object* array = (as_array_object*) (as_object*) fn.this_ptr;

		// If the array is empty, report an error and return undefined!
		if (array->elements.size() <= 0)
		{
			IF_VERBOSE_ACTION(log_error("tried to pop element from back of empty array!\n"));
			fn.result->set_undefined();
			return;
		}

		// Get our index, log, then return result
		(*fn.result) = array->elements[array->elements.size()-1];
		array->elements.pop_back();
		IF_VERBOSE_ACTION(log_msg("calling array pop, result:%s, new array size:%d\n",fn.result->to_string(),array->elements.size()));
	}

	// Callback to pop a value from the front of an array
	void array_shift(const fn_call& fn)
	{
		as_array_object* array = (as_array_object*) (as_object*) fn.this_ptr;

		// If the array is empty, report an error and return undefined!
		if (array->elements.size() <= 0)
		{
			IF_VERBOSE_ACTION(log_error("tried to shift element from front of empty array!\n"));
			fn.result->set_undefined();
			return;
		}

		// Get our index, log, then return result
		(*fn.result) = array->elements[0];
		array->elements.pop_front();
		IF_VERBOSE_ACTION(log_msg("calling array shift, result:%s, new array size:%d\n",fn.result->to_string(),array->elements.size()));
	}

	// Callback to reverse the position of the elements in an array
	void array_reverse(const fn_call& fn)
	{
		as_array_object* array = (as_array_object*) (as_object*) fn.this_ptr;

		int i,j;
		as_value temp;

		// Reverse the deque elements
		for (i=0,j=int(array->elements.size())-1;i<j;i++,j--)
		{
			temp = array->elements[i];
			array->elements[i] = array->elements[j];
			array->elements[j] = temp;
		}
		
		IF_VERBOSE_ACTION(log_msg("calling array reverse on array with size:%d\n",array->elements.size()));

		// result is undefined
		fn.result->set_undefined();
	}

	// Callback to convert array to a string with optional custom separator (default ',')
	void array_join(const fn_call& fn)
	{
		as_array_object* array = (as_array_object*) (as_object*) fn.this_ptr;

		unsigned int i;
		std::string temp = "(",separator = ",";

		if (fn.nargs > 0)
			separator = fn.arg(0).to_string();

		// TODO - confirm this is the right format!
		// Reportedly, flash version 7 on linux, and Flash 8 on IE look like
		// "(1,2,3)" and "1,2,3" respectively - which should we mimic?
		// I've chosen the former for now, because the parentheses help clarity
		for (i=0;i<array->elements.size() - 1;i++)
			temp = temp + array->elements[i].to_string() + separator;

		// Add the last element without a trailing separator
		if (array->elements.size() > 0)
			temp = temp + array->elements[i].to_string();

		temp = temp + ")";

		fn.result->set_string(temp.c_str());
	}

	// Callback to convert array to a string
	void array_to_string(const fn_call& fn)
	{
		as_array_object* array = (as_array_object*) (as_object*) fn.this_ptr;

		unsigned int i;
		std::string temp = "(";

		// TODO - confirm this is the right format!
		// Reportedly, flash version 7 on linux, and Flash 8 on IE look like
		// "(1,2,3)" and "1,2,3" respectively - which should we mimic?
		// I've chosen the former for now, because the parentheses help clarity
		for (i=0;i<array->elements.size() - 1;i++)
			temp = temp + array->elements[i].to_string() + ',';

		// Add the last element without a trailing comma
		if (array->elements.size() > 0)
			temp = temp + array->elements[i].to_string();

		temp = temp + ")";

		fn.result->set_string(temp.c_str());
	}

	// Callback to convert array to a string
	void array_concat(const fn_call& fn)
	{
		as_array_object* array = (as_array_object*) (as_object*) fn.this_ptr;
		as_array_object* newarray = new as_array_object;

		newarray->elements = array->elements;

		for (int i=0;i<fn.nargs;i++)
			newarray->elements.push_back(fn.arg(i));

		fn.result->set_as_object_interface(newarray);		
	}

	// Callback to slice part of an array to a new array without changing the original
	void array_slice(const fn_call& fn)
	{
		as_array_object* array = (as_array_object*) (as_object*) fn.this_ptr;

		int startindex,endindex; // start and end index of the part we're slicing

		if (fn.nargs > 2)
		{
			IF_VERBOSE_ACTION(log_error("More than 2 arguments sent to slice, and I don't know what to do with them!\n"));
			IF_VERBOSE_ACTION(log_error("Ignoring them as we continue...\n"));
		}

		// if we sent at least one argument, let's setup startindex
		if (fn.nargs >= 1)
		{
			startindex = int(fn.arg(0).to_number());
			// if the index is negative, it means "places from the end" where -1 is the last element
			if (startindex < 0) startindex = startindex + array->elements.size();
			// if it's still negative, this is a problem
			if (startindex < 0 || startindex > int(array->elements.size()))
			{
				IF_VERBOSE_ACTION(log_error("bad startindex sent to array_slice! startindex: %s, Length: %d",
					fn.arg(0).to_string(),array->elements.size()));
				return;				
			}
			// if we sent at least two arguments, setup endindex
			if (fn.nargs >= 2)
			{
				endindex = int(fn.arg(1).to_number());
				// if the index is negative, it means "places from the end" where -1 is the last element
				if (endindex < 0) endindex = endindex + array->elements.size();
				// the endindex is non-inclusive, so add 1
				endindex++;
				if (endindex < 0)
				{
					IF_VERBOSE_ACTION(log_error("bad endindex sent to array_slice! endindex: %s, length: %d",
						fn.arg(1).to_string(),array->elements.size()));
					return;				
				}
				// If they overshoot the end of the array, just copy to the end
				if (endindex > int(array->elements.size()) + 1) endindex = array->elements.size() + 1;
			}
			else
				// They didn't specify where to end, so choose the end of the array
				endindex = array->elements.size() + 1;
		}
		else
		{
			// They passed no arguments: simply duplicate the array and return the new one
			as_array_object* newarray = new as_array_object;
			newarray->elements = array->elements;
			fn.result->set_as_object_interface(newarray);
			return;
		}

		as_array_object* newarray = new as_array_object;

		newarray->elements.resize(endindex - startindex - 1);

		for (int i=startindex;i<endindex;i++)
			newarray->elements[i-startindex] = array->elements[i];

		fn.result->set_as_object_interface(newarray);		
	}

	// this sets all the callback members for an array function - it's called from as_array_object's constructor
	void array_init(as_array_object *array)
	{
		array->set_member("length", &array_length);
		array->set_member("join", &array_join);
		array->set_member("concat", &array_concat);
		array->set_member("slice", &array_slice);
		array->set_member("push", &array_push);
		array->set_member("unshift", &array_unshift);
		array->set_member("pop", &array_pop);
		array->set_member("shift", &array_shift);
		array->set_member("splice", &array_not_impl);
		array->set_member("sort", &array_not_impl);
		array->set_member("sortOn", &array_not_impl);
		array->set_member("reverse", &array_reverse);
		array->set_member("toString", &array_to_string);
	}

	void	as_global_array_ctor(const fn_call& fn)
	// Constructor for ActionScript class Array.
	{
		smart_ptr<as_array_object>	ao = new as_array_object;

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
				index_number.set_int(i);
				ao->set_member(index_number.to_string(), fn.arg(i));
			}
		}

		fn.result->set_as_object_interface(ao.get_ptr());
	}
};
