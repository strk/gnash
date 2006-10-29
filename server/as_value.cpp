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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "as_value.h"
#include "as_object.h"
#include "as_function.h" // for as_function

using namespace std;

#ifdef WIN32
#	define snprintf _snprintf
#endif

namespace gnash {

//
// as_value -- ActionScript value type
//


as_value::as_value(as_object* obj)
    :
    m_type(OBJECT),
    m_object_value(obj)
{
    if (m_object_value)	{
	m_object_value->add_ref();
    }
}


as_value::as_value(as_function* func)
    :
    m_type(AS_FUNCTION),
    m_as_function_value(func)
{
    if (m_as_function_value) {
	m_as_function_value->add_ref();
    }
}


// Conversion to string.
const char
*as_value::to_string() const
{
    return to_tu_string().c_str();
}

const tu_stringi
&as_value::to_tu_stringi() const
{
    return reinterpret_cast<const tu_stringi&>(to_tu_string());
}

// Conversion to const tu_string&.
const tu_string&
as_value::to_tu_string() const
{
	char buffer[50];
	const char*	val = NULL;

	switch (m_type)
	{

		case STRING:
			/* don't need to do anything */
			break;

		case NUMBER:
			// @@ Moock says if value is a NAN,
			// then result is "NaN"
			// INF goes to "Infinity"
			// -INF goes to "-Infinity"
			if (isnan(m_number_value))
			{
				m_string_value = "NaN";
			}
			else if (isinf(m_number_value))
			{
				if (m_number_value > 0.0)
				{
					m_string_value = "+Infinity";
				}
				else
				{
					m_string_value = "-Infinity";
				}
			}
			else
			{
				char buffer[50];
				snprintf(buffer, 50, "%.14g", m_number_value);
				m_string_value = buffer;
			}
			break;

		case UNDEFINED: 

			// Behavior depends on file version.  In
			// version 7+, it's "undefined", in versions
			// 6-, it's "".
			//
			// We'll go with the v7 behavior by default,
			// and conditionalize via _versioned()
			// functions.
			m_string_value = "undefined";

			break;

		case NULLTYPE:
			m_string_value = "null";
			break;

		case BOOLEAN:
			m_string_value = this->m_boolean_value ? "true" : "false";
			break;

		case OBJECT:
			// @@ Moock says, "the value that results from
			// calling toString() on the object".
			//
			// The default toString() returns "[object
			// Object]" but may be customized.
			//
			// A Movieclip returns the absolute path of the object.
			//
			if (m_object_value)
			{
				val = m_object_value->get_text_value();
			}
			if (val)
			{
				m_string_value = val;
			}
			else
			{
				// Do we have a "toString" method?
				//
				// TODO: we need an environment in order to
				// call toString()!

				// This is the default.
				//m_string_value = "[object Object]";
				snprintf(buffer, 50, "<as_object %p>",
					(void *) m_object_value);
				m_string_value = buffer;
			}
			break;

		case C_FUNCTION:
			snprintf(buffer, 50, "<c_function %p>",
				(const void *) &m_c_function_value);
			m_string_value = buffer;
			break;

		case AS_FUNCTION:
			snprintf(buffer, 50, "<as_function %p>",
				(void *) m_as_function_value);
			m_string_value = buffer;
			break;

		default:
			m_string_value = "<bad type> "+m_type;
			assert(0);
    }
    
    return m_string_value;
}

// Conversion to const tu_string&.
const tu_string
&as_value::to_tu_string_versioned(int version) const
{
    if (m_type == UNDEFINED) {
	// Version-dependent behavior.
	if (version <= 6) {
	    m_string_value = "";
	} else {
	    m_string_value = "undefined";
	}
	return m_string_value;
    }
		
    return to_tu_string();
}

// Conversion to double.
double
as_value::to_number() const
{
    if (m_type == STRING) {
	// @@ Moock says the rule here is: if the
	// string is a valid float literal, then it
	// gets converted; otherwise it is set to NaN.
	//
	// Also, "Infinity", "-Infinity", and "NaN"
	// are recognized.
	char* tail=0;
	m_number_value = strtod(m_string_value.c_str(), &tail);
	if ( tail == m_string_value.c_str() || *tail != 0 )
	{
		// Failed conversion to Number.

		// avoid divide by zero compiler warning by using a variable
		double temp = 0.0;

		// this division by zero creates a NaN value in the double
		m_number_value = temp / temp;
	}
	return m_number_value;
    } else if (m_type == NULLTYPE) {
	// Evan: from my tests
	return 0;
    } else if (m_type == BOOLEAN) {
	// Evan: from my tests
	return (this->m_boolean_value) ? 1 : 0;
    } else if (m_type == NUMBER) {
	return m_number_value;
    } else if (m_type == OBJECT && m_object_value != NULL) {
	// @@ Moock says the result here should be
	// "the return value of the object's valueOf()
	// method".
	//
	// Arrays and Movieclips should return NaN.
	
	// Text characters with var names could get in
	// here.
	const char* textval = m_object_value->get_text_value();
	if (textval) {
	    return atof(textval);
	}
	
	return 0.0;
    } else {
	return 0.0;
    }
}

// Conversion to boolean.
bool
as_value::to_bool() const
{
    // From Moock
    if (m_type == STRING) {
	if (m_string_value == "false") {
	    return false;
	} else if (m_string_value == "true") {
	    return true;
	} else {
	    // @@ Moock: "true if the string can
	    // be converted to a valid nonzero
	    // number".
	    //
	    // Empty string --> false
	    return to_number() != 0.0;
	}
    } else if (m_type == NUMBER) {
	// If m_number_value is NaN, comparison will automatically be false, as it should
	return m_number_value != 0.0;
    } else if (m_type == BOOLEAN) {
	return this->m_boolean_value;
    } else if (m_type == OBJECT) {
	return m_object_value != NULL;
    } else if (m_type == C_FUNCTION) {
	return m_c_function_value != NULL;
    } else if (m_type == AS_FUNCTION) {
	return m_as_function_value != NULL;
    } else {
	assert(m_type == UNDEFINED || m_type == NULLTYPE);
	return false;
    }
}
	
// Return value as an object.
as_object*
as_value::to_object() const
{
    if (m_type == OBJECT) {
	// OK.
	return m_object_value;
    } else if (m_type == AS_FUNCTION) {
	// An AS_FUNCTION *is* an object
	return m_as_function_value;
    } else {
	return NULL;
    }
}


as_c_function_ptr
as_value::to_c_function() const
    // Return value as a C function ptr.  Returns NULL if value is
    // not a C function.
{
    if (m_type == C_FUNCTION) {
	// OK.
	return m_c_function_value;
    } else {
	return NULL;
    }
}

// Return value as an ActionScript function.  Returns NULL if value is
// not an ActionScript function.
as_function*
as_value::to_as_function() const
{
    if (m_type == AS_FUNCTION) {
	// OK.
	return m_as_function_value;
    } else {
	return NULL;
    }
}

// Force type to number.
void
as_value::convert_to_number()
{
    set_double(to_number());
}

// Force type to string.
void
as_value::convert_to_string()
{
    to_tu_string();	// init our string data.
    m_type = STRING;	// force type.
}


void
as_value::convert_to_string_versioned(int version)
    // Force type to string.
{
    to_tu_string_versioned(version);	// init our string data.
    m_type = STRING;	// force type.
}


void
as_value::set_as_object(as_object* obj) {
    if (m_type != OBJECT || m_object_value != obj) {
	drop_refs();
	m_type = OBJECT;
	m_object_value = obj;
	if (m_object_value) {
	    m_object_value->add_ref();
	}
    }
}

void
as_value::set_as_function(as_function* func)
{
    if (m_type != AS_FUNCTION || m_as_function_value != func) {
	drop_refs();
	m_type = AS_FUNCTION;
	m_as_function_value = func;
	if (m_as_function_value) {
	    m_as_function_value->add_ref();
	}
    }
}

// Return true if operands are equal.
bool
as_value::operator==(const as_value& v) const
{
    bool this_nulltype = (m_type == UNDEFINED || m_type == NULLTYPE);
    bool v_nulltype = (v.get_type() == UNDEFINED || v.get_type() == NULLTYPE);
    if (this_nulltype || v_nulltype)
    {
	return this_nulltype == v_nulltype;
    }
    else if (m_type == C_FUNCTION || v.m_type == C_FUNCTION)
    {
	// a C_FUNCTION is only equal to itself
    	return m_type == v.m_type
		&& m_c_function_value == v.m_c_function_value;
    }
    else if (m_type == STRING)
    {
	return m_string_value == v.to_tu_string();
    }
    else if (m_type == NUMBER)
    {
	return m_number_value == v.to_number();
    }
    else if (m_type == BOOLEAN)
    {
	return m_boolean_value == v.to_bool();

    }
    else if (is_object())
    {
    	if ( v.is_object() )
	{
		// compare by reference
		return to_object() == v.to_object();
	}
	else
	{
		// convert this value to a primitive and recurse
		// TODO: implement ``as_value as_value::to_primitive() const''
		//return to_primitive() == v;

		// to_primitive is not implemented yet, so
		// we force conversion to a number
		// (might as well force conversion to a string though...)
		return as_value(to_number()) == v;
	}
    }
    else
    {
	assert(0);
    }
}
	
// Return true if operands are not equal.
bool
as_value::operator!=(const as_value& v) const
{
    return ! (*this == v);
}
	
// Sets *this to this string plus the given string.
void
as_value::string_concat(const tu_string& str)
{
    to_tu_string();	// make sure our m_string_value is initialized
    m_type = STRING;
    m_string_value += str;
}

// Drop any ref counts we have; this happens prior to changing our value.
void
as_value::drop_refs()
{
    if (m_type == AS_FUNCTION) {
	if (m_as_function_value) {
	    m_as_function_value->drop_ref();
	    m_as_function_value = 0;
	}
    } else if (m_type == OBJECT) {
	if (m_object_value) {
	    m_object_value->drop_ref();
	    m_object_value = 0;
	}
    }
}


}


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
