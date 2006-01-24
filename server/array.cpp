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

		fn.result->set_undefined();
	}

	// Callback to push values to the front of an array
	void array_unshift(const fn_call& fn)
	{
		as_array_object* array = (as_array_object*) (as_object*) fn.this_ptr;
		IF_VERBOSE_ACTION(log_msg("calling array unshift, pushing %d values onto front of array\n",fn.nargs));

		for (int i=fn.nargs-1;i>=0;i--)
			array->elements.push_front(fn.arg(i));

		fn.result->set_undefined();
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

	// Unimplemented callback to convert array to a string
	void array_to_string(const fn_call& fn)
	{
		IF_VERBOSE_ACTION(log_error("array method to_string not implemented yet\n"));
	}

	// this sets all the callback members for an array function - it's called from as_array_object's constructor
	void array_init(as_array_object *array)
	{
		array->set_member("length", &array_length);
		array->set_member("join", &array_not_impl);
		array->set_member("concat", &array_not_impl);
		array->set_member("slice", &array_not_impl);
		array->set_member("push", &array_push);
		array->set_member("unshift", &array_unshift);
		array->set_member("pop", &array_pop);
		array->set_member("shift", &array_shift);
		array->set_member("splice", &array_not_impl);
		array->set_member("sort", &array_not_impl);
		array->set_member("sortOn", &array_not_impl);
		array->set_member("reverse", &array_not_impl);
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
