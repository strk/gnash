// as_value.cpp:  ActionScript values, for Gnash.
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
#include "as_object.h"
#include "as_function.h" // for as_function
#include "sprite_instance.h" // for MOVIECLIP values
#include "as_environment.h" // for MOVIECLIP values
#include "VM.h" // for MOVIECLIP values
#include "movie_root.h" // for MOVIECLIP values
#include "gstring.h" // for automatic as_value::STRING => String as object
#include "Number.h" // for automatic as_value::NUMBER => Number as object
#include "Boolean.h" // for automatic as_value::BOOLEAN => Boolean as object
#include "action.h" // for call_method0
#include "utility.h" // for typeName()
#include "namedStrings.h"

#include <boost/algorithm/string/case_conv.hpp>

using namespace std;

#ifdef WIN32
#	define snprintf _snprintf
#endif

// Undefine this to keep MOVIECLIP values by pointer
// rather then by "target" ref.
//#define MOVIECLIP_AS_SOFTREF

// Define the macro below to make abstract equality operator verbose
//#define GNASH_DEBUG_EQUALITY

namespace gnash {

//
// as_value -- ActionScript value type
//

as_value::as_value(as_function* func)
    :
    m_type(AS_FUNCTION),
    m_object_value(func)
{
    if (m_object_value) {
#ifndef GNASH_USE_GC
	m_object_value->add_ref();
#endif // GNASH_USE_GC
    } else {
        m_type = NULLTYPE;
    }
}

// Conversion to const std::string&.
const std::string&
as_value::to_string(as_environment* env) const
{
	switch (m_type)
	{

		case STRING:
			/* don't need to do anything */
			break;

		case MOVIECLIP:
		{
#ifdef MOVIECLIP_AS_SOFTREF
			/* don't need to do anything */
#else
			sprite_instance* sp = m_object_value->to_movie();
			assert(sp); // or return as in to_sprite() ?
			if ( sp ) {
				m_string_value = sp->getTarget();
			}
#endif
			break;
		}

		case NUMBER:
			m_string_value = doubleToString(m_number_value);
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
		case AS_FUNCTION:
		{
			//printf("as_value to string conversion, env=%p\n", env);
			// @@ Moock says, "the value that results from
			// calling toString() on the object".
			//
			// When the toString() method doesn't exist, or
			// doesn't return a valid number, the default
			// text representation for that object is used
			// instead.
			//
			as_object* obj = m_object_value; 
			bool gotValidToStringResult = false;
			if ( env )
			{
				string_table::key methodname = NSV::PROP_TO_STRING;
				as_value method;
				if ( obj->get_member(methodname, &method) )
				{
					as_value ret = call_method0(method, env, obj);
					if ( ret.is_string() )
					{
						gotValidToStringResult=true;
						m_string_value = ret.m_string_value;
					}
					else
					{
						log_msg(_("[object %p].%s() did not return a string: %s"),
								(void*)obj, VM::get().getStringTable().value(methodname).c_str(),
								ret.to_debug_string().c_str());
					}
				}
				else
				{
					log_msg(_("get_member(%s) returned false"), VM::get().getStringTable().value(methodname).c_str());
				}
			}
			if ( ! gotValidToStringResult )
			{
				if ( m_type == OBJECT )
				{
					m_string_value = "[type Object]";
				}
				else
				{
					assert(m_type == AS_FUNCTION);
					m_string_value = "[type Function]";
				}
			}
			break;
		}

		default:
			m_string_value = "[exception]";
			break;
    }
    
    return m_string_value;
}

// Conversion to const std::string&.
const std::string&
as_value::to_string_versioned(int version, as_environment* env) const
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
		
    return to_string(env);
}

// Version-based Conversion to std::string
std::string
as_value::to_std_string_versioned(int version, as_environment* env) const
{
	return to_string_versioned(version, env);
}

// Conversion to primitive value.
as_value
as_value::to_primitive(as_environment& env) const
{
	if ( m_type == OBJECT || m_type == AS_FUNCTION )
	{
		as_object* obj = m_object_value;
		string_table::key methodname = NSV::PROP_VALUE_OF;
		as_value method;
		if ( obj->get_member(methodname, &method) )
		{
			return call_method0(method, &env, obj);
		}
		else
		{
			log_msg(_("get_member(%s) returned false"), VM::get().getStringTable().value(methodname).c_str());
		}
	}

	return *this;

}

// Conversion to double.
double
as_value::to_number(as_environment* env) const
{
	// TODO:  split in to_number_# (version based)

	int swfversion = VM::get().getSWFVersion();

	switch (m_type)
	{
		case STRING:
		{
			// @@ Moock says the rule here is: if the
			// string is a valid float literal, then it
			// gets converted; otherwise it is set to NaN.
			char* tail=0;
			m_number_value = strtod(m_string_value.c_str(), &tail);
			// Detect failure by "tail" still being at the start of
			// the string or there being extra junk after the
			// converted characters.
			if ( tail == m_string_value.c_str() || *tail != 0 )
			{
				// Failed conversion to Number.
				m_number_value = NAN;
			}

			// "Infinity" and "-Infinity" are recognized by strtod()
			// but Flash Player returns NaN for them.
			if ( isinf(m_number_value) ) {
				m_number_value = NAN;
			}

			return m_number_value;
		}

		case NULLTYPE:
		case UNDEFINED:
			// Evan: from my tests
			// Martin: FlashPlayer6 gives 0; FP9 gives NaN.
			return ( swfversion >= 7 ? NAN : 0 );

		case BOOLEAN:
			// Evan: from my tests
			// Martin: confirmed
			return (this->m_boolean_value) ? 1 : 0;

		case NUMBER:
			return m_number_value;

		case OBJECT:
		case AS_FUNCTION:
		    {
			// @@ Moock says the result here should be
			// "the return value of the object's valueOf()
			// method".
			//
			// Arrays and Movieclips should return NaN.

			//log_msg(_("OBJECT to number conversion, env is %p"), env);

			as_object* obj = m_object_value; 
			if ( env )
			{
				string_table::key methodname = NSV::PROP_VALUE_OF;
				as_value method;
				if (obj->get_member(methodname, &method) )
				{
					as_value ret = call_method0(method, env, obj);
					if ( ret.is_number() )
					{
						return ret.m_number_value;
					}
					else if ( ret.is_string() )
					{
						return ret.to_number();
					}
					else
					{
						log_msg(_("[object %p].%s() did not return a number: %s"),
								(void*)obj, VM::get().getStringTable().value(methodname).c_str(),
								ret.to_debug_string().c_str());
						if ( m_type == AS_FUNCTION && swfversion < 6 )
						{
							return 0;
						}
						else
						{
							return NAN;
						}
					}
				}
				else
				{
					log_msg(_("get_member(%s) returned false"), VM::get().getStringTable().value(methodname).c_str());
				}
			}
			return obj->get_numeric_value(); 
		    }

		case MOVIECLIP:
			// This is tested, no valueOf is going
			// to be invoked for movieclips.
			return NAN; 

		default:
			// Other object types should return NaN, but if we implement that,
			// every GUI's movie canvas shrinks to size 0x0. No idea why.
			return NAN; // 0.0;
	}
	/* NOTREACHED */
}

int32_t
as_value::to_int(as_environment& env) const
{
	double d = to_number(&env);
	int i=0;

	if ( ! isfinite(d) ) return 0;

	if (d < 0)
	{
		i = - (uint32_t) fmod (-d, 4294967296.0);
	}
	else
	{
		i = (uint32_t) fmod (d, 4294967296.0);
	}

	return i;
}

// Conversion to boolean for SWF7 and up
bool
as_value::to_bool_v7() const
{
	    switch (m_type)
	    {
		case  STRING:
			return m_string_value != "";
		case NUMBER:
			return m_number_value && ! isnan(m_number_value);
		case BOOLEAN:
			return this->m_boolean_value;
		case OBJECT:
		case AS_FUNCTION:
			// it is possible we'll need to convert to number anyway first
			return m_object_value != NULL;
		case MOVIECLIP:
			return true;
		default:
			assert(m_type == UNDEFINED || m_type == NULLTYPE ||
				is_exception());
			return false;
	}
}

// Conversion to boolean up to SWF5
bool
as_value::to_bool_v5() const
{
	    switch (m_type)
	    {
		case  STRING:
		{
			if (m_string_value == "false") return false;
			else if (m_string_value == "true") return true;
			else
			{
				double num = to_number();
				bool ret = num && ! isnan(num);
				//log_msg(_("m_string_value: %s, to_number: %g, to_bool: %d"), m_string_value.c_str(), num, ret);
				return ret;
			}
		}
		case NUMBER:
			return ! isnan(m_number_value) && m_number_value; 
		case BOOLEAN:
			return this->m_boolean_value;
		case OBJECT:
		case AS_FUNCTION:
			// it is possible we'll need to convert to number anyway first
			return m_object_value != NULL;
		case MOVIECLIP:
			return true;
		default:
			assert(m_type == UNDEFINED || m_type == NULLTYPE ||
				is_exception());
			return false;
	}
}

// Conversion to boolean for SWF6
bool
as_value::to_bool_v6() const
{
	    switch (m_type)
	    {
		case  STRING:
		{
			if (m_string_value == "false") return false;
			else if (m_string_value == "true") return true;
			else
			{
				double num = to_number();
				bool ret = num && ! isnan(num);
				//log_msg(_("m_string_value: %s, to_number: %g, to_bool: %d"), m_string_value.c_str(), num, ret);
				return ret;
			}
		}
		case NUMBER:
			return isfinite(m_number_value) && m_number_value; 
		case BOOLEAN:
			return this->m_boolean_value;
		case OBJECT:
		case AS_FUNCTION:
			// it is possible we'll need to convert to number anyway first
			return m_object_value != NULL;
		case MOVIECLIP:
			return true;
		default:
			assert(m_type == UNDEFINED || m_type == NULLTYPE ||
				is_exception());
			return false;
	}
}

// Conversion to boolean.
bool
as_value::to_bool() const
{
    int ver = VM::get().getSWFVersion();
    if ( ver >= 7 ) return to_bool_v7();
    else if ( ver == 6 ) return to_bool_v6();
    else return to_bool_v5();
}
	
// Return value as an object.
boost::intrusive_ptr<as_object>
as_value::to_object() const
{
	typedef boost::intrusive_ptr<as_object> ptr;

	switch (m_type)
	{
		case OBJECT:
		case AS_FUNCTION:
			return ptr(m_object_value);

		case MOVIECLIP:
			// FIXME: update when to_sprite will return
			//        an intrusive_ptr directly
			return ptr(to_sprite());

		case STRING:
			return init_string_instance(m_string_value.c_str());

		case NUMBER:
			return init_number_instance(m_number_value);

		case BOOLEAN:
			return init_boolean_instance(m_boolean_value);

		default:
			// Invalid to convert exceptions.
			return NULL;
	}
}

/* static private */
sprite_instance*
as_value::find_sprite_by_target(const std::string& tgtstr)
{
	// Evaluate target everytime an attempt is made 
	// to fetch a movieclip value.
	sprite_instance* root = VM::get().getRoot().get_root_movie();
	as_environment& env = root->get_environment();
	character* target = env.find_target(tgtstr);
	if ( ! target ) return NULL;
	return target->to_movie();
}

sprite_instance*
as_value::to_sprite() const
{
	if ( m_type != MOVIECLIP ) return NULL;

#ifndef MOVIECLIP_AS_SOFTREF
	sprite_instance* sp = m_object_value->to_movie();
	if ( ! sp ) return NULL; // shoudl we assert(sp) instead ?

	if ( sp->isUnloaded() )
	// TODO: we should also check if the unload event handlers have been invoked or not, or references to 'this' in unload handlers will be bogus !
	{
		log_debug(_("MovieClip value is a dangling reference: "
				"target %s was unloaded (looking for a substitute on the same target))"),
				sp->getTarget().c_str());
		sp = find_sprite_by_target(sp->getOrigTarget());
		return sp;
		//return NULL;
	}
	return sp;
#else
	// Evaluate target everytime an attempt is made 
	// to fetch a movieclip value.
	sprite_instance* sp = find_sprite_by_target(m_string_value);
	if ( ! sp )
	{
		log_debug(_("MovieClip value is a dangling reference: "
				"target '%s' not found (should set to NULL?)"),
				m_string_value.c_str());
		return NULL;
	}
	else
	{
		return sp;
	}
#endif
}

void
as_value::set_sprite(const sprite_instance& sprite)
{
	drop_refs();
	m_type = MOVIECLIP;
#ifndef MOVIECLIP_AS_SOFTREF
	m_object_value = const_cast<sprite_instance*>(&sprite);
#else
	m_string_value = sprite.get_text_value();
#endif
}

void
as_value::set_sprite(const std::string& path)
{
	drop_refs();
	m_type = MOVIECLIP;
#ifndef MOVIECLIP_AS_SOFTREF
	sprite_instance* sp = find_sprite_by_target(path);
	if ( ! sp ) set_null();
	else set_sprite(*sp);
#else
	// TODO: simplify next statement when m_string_value will become a std::string
	m_string_value = path.c_str();
#endif
}

// Return value as an ActionScript function.  Returns NULL if value is
// not an ActionScript function.
as_function*
as_value::to_as_function() const
{
    if (m_type == AS_FUNCTION) {
	// OK.
	return m_object_value->to_function();
    } else {
	return NULL;
    }
}

// Force type to number.
void
as_value::convert_to_number(as_environment* env)
{
    set_double(to_number(env));
}

// Force type to string.
void
as_value::convert_to_string()
{
    to_string();	// init our string data.
    m_type = STRING;	// force type.
}


void
as_value::convert_to_string_versioned(int version, as_environment* env)
    // Force type to string.
{
    to_string_versioned(version, env); // init our string data.
    m_type = STRING;	// force type.
}


void
as_value::set_as_object(as_object* obj)
{
	if ( ! obj )
	{
		set_null();
		return;
	}
	sprite_instance* sp = obj->to_movie();
	if ( sp )
	{
		set_sprite(*sp);
		return;
	}
	as_function* func = obj->to_function();
	if ( func )
	{
		set_as_function(func);
		return;
	}
	if (m_type != OBJECT || m_object_value != obj)
	{
		drop_refs();
		m_type = OBJECT;
		m_object_value = obj;
#ifndef GNASH_USE_GC
		if (m_object_value)
		{
			m_object_value->add_ref();
		}
#endif // GNASH_USE_GC
	}
}

void
as_value::set_as_object(boost::intrusive_ptr<as_object> obj)
{
	set_as_object(obj.get());
}

void
as_value::set_as_function(as_function* func)
{
    if (m_type != AS_FUNCTION || m_object_value != func) {
	drop_refs();
	m_type = AS_FUNCTION;
	m_object_value = func;
	if (m_object_value) {
#ifndef GNASH_USE_GC
	    m_object_value->add_ref();
#endif // GNASH_USE_GC
	} else {
	    m_type = NULLTYPE;
	}
    }
}

bool
as_value::equals(const as_value& v, as_environment& env) const
{
    // Comments starting with numbers refer to the ECMA-262 document

#ifdef GNASH_DEBUG_EQUALITY
    static int count=0;
    log_debug("equals(%s, %s) called [%d]", to_debug_string().c_str(), v.to_debug_string().c_str(), count++);
#endif

    int SWFVersion = env.get_version();

    bool this_nulltype = (m_type == UNDEFINED || m_type == NULLTYPE);
    bool v_nulltype = (v.get_type() == UNDEFINED || v.get_type() == NULLTYPE);

    // It seems like functions are considered the same as a NULL type
    // in SWF5 (and I hope below, didn't check)
    //
    if ( SWFVersion < 6 )
    {
        if ( m_type == AS_FUNCTION ) this_nulltype = true;
        if ( v.m_type == AS_FUNCTION ) v_nulltype = true;
    }

    if (this_nulltype || v_nulltype)
    {
#ifdef GNASH_DEBUG_EQUALITY
       log_debug(" on of the two things is undefined or null");
#endif
        return this_nulltype == v_nulltype;
    }

    bool obj_or_func = (m_type == OBJECT || m_type == AS_FUNCTION);
    bool v_obj_or_func = (v.m_type == OBJECT || v.m_type == AS_FUNCTION);

    /// Compare to same type
    if ( obj_or_func && v_obj_or_func ) return m_object_value == v.m_object_value;
    if ( m_type == v.m_type ) return equalsSameType(v);

    // 16. If Type(x) is Number and Type(y) is String,
    //    return the result of the comparison x == ToNumber(y).
    if (m_type == NUMBER && v.m_type == STRING)
    {
	double n = v.to_number(&env); // no need for the env actually
	if ( ! isfinite(n) ) return false;
        return equalsSameType(n);
    }

    // 17. If Type(x) is String and Type(y) is Number,
    //     return the result of the comparison ToNumber(x) == y.
    if (v.m_type == NUMBER && m_type == STRING)
    {
	double n = to_number(&env); // no need for the env actually
	if ( ! isfinite(n) ) return false;
        return v.equalsSameType(n); 
    }

    // 18. If Type(x) is Boolean, return the result of the comparison ToNumber(x) == y.
    if (m_type == BOOLEAN)
    {
        return as_value(to_number(&env)).equals(v, env); // m_boolean_value == v.to_bool();
    }

    // 19. If Type(y) is Boolean, return the result of the comparison x == ToNumber(y).
    if (v.m_type == BOOLEAN)
    {
        return as_value(v.to_number(&env)).equals(*this, env); 
    }

    // 20. If Type(x) is either String or Number and Type(y) is Object,
    //     return the result of the comparison x == ToPrimitive(y).
    if ( (m_type == STRING || m_type == NUMBER ) && ( v.is_object() ) ) // v.m_type == OBJECT || v.m_type == AS_FUNCTION ) )
    {
        // convert this value to a primitive and recurse
        as_value v2 = v.to_primitive(env); 
#ifdef GNASH_DEBUG_EQUALITY
       log_debug(" convertion to primitive : %s -> %s", v.to_debug_string().c_str(), v2.to_debug_string().c_str());
#endif
        if ( v.strictly_equals(v2) ) // returned self ?
        {
            return false; // no valid conversion  
        }
        else return equals(v2, env);
    }

    // 21. If Type(x) is Object and Type(y) is either String or Number,
    //    return the result of the comparison ToPrimitive(x) == y.
    if ( (v.m_type == STRING || v.m_type == NUMBER ) && ( is_object() ) ) // m_type == OBJECT || m_type == AS_FUNCTION ) )
    {
        // convert this value to a primitive and recurse
        as_value v2 = to_primitive(env); 
#ifdef GNASH_DEBUG_EQUALITY
       log_debug(" convertion to primitive : %s -> %s", to_debug_string().c_str(), v2.to_debug_string().c_str());
#endif
        if ( strictly_equals(v2) ) // returned self ?
        {
            return false; // no valid conversion 
        }
        else return v2.equals(v, env);
    }

    // Both operands are objects (OBJECT,AS_FUNCTION,MOVIECLIP)
    assert(is_object() && v.is_object());

    // If any of the two converts to a primitive, we recurse

    as_value p = to_primitive(env); 
    as_value vp = v.to_primitive(env); 
#ifdef GNASH_DEBUG_EQUALITY
    log_debug(" convertion to primitive (this): %s -> %s", to_debug_string().c_str(), p.to_debug_string().c_str());
    log_debug(" convertion to primitive (that): %s -> %s", v.to_debug_string().c_str(), vp.to_debug_string().c_str());
#endif
    if ( strictly_equals(p) && v.strictly_equals(vp) ) // both returned self ?
    {
            return false; // no valid conversion 
    }

    return p.equals(vp, env);

}
	
// Sets *this to this string plus the given string.
void
as_value::string_concat(const std::string& str)
{
    to_string();	// make sure our m_string_value is initialized
    m_type = STRING;
    m_string_value += str;
}

// Drop any ref counts we have; this happens prior to changing our value.
void
as_value::drop_refs()
{
#ifndef GNASH_USE_GC
    if (m_type == AS_FUNCTION || m_type == OBJECT )
    {
	if (m_object_value) // should assert here ?
	{
	    m_object_value->drop_ref();
	}
    } 
#endif // GNASH_USE_GC
}

const char*
as_value::typeOf() const
{
	switch(get_type())
	{
		case as_value::UNDEFINED:
			return "undefined"; 

		case as_value::STRING:
			return "string";

		case as_value::NUMBER:
			return "number";

		case as_value::BOOLEAN:
			return "boolean";

		case as_value::OBJECT:
			return "object";

		case as_value::MOVIECLIP:
			return "movieclip";

		case as_value::NULLTYPE:
			return "null";

		case as_value::AS_FUNCTION:
			return "function";

		default:
			if (is_exception())
				return "exception";
			assert(0);
			return NULL;
	}
}

/*private*/
bool
as_value::equalsSameType(const as_value& v) const
{
#ifdef GNASH_DEBUG_EQUALITY
    static int count=0;
    log_debug("equalsSameType(%s, %s) called [%d]", to_debug_string().c_str(), v.to_debug_string().c_str(), count++);
#endif
	assert(m_type == v.m_type);
	switch (m_type)
	{
		case UNDEFINED:
		case NULLTYPE:
			return true;

		case OBJECT:
		case AS_FUNCTION:
			return m_object_value == v.m_object_value;

		case BOOLEAN:
			return m_boolean_value == v.m_boolean_value;

		case STRING:
			return m_string_value == v.m_string_value;

		case MOVIECLIP:
#ifdef MOVIECLIP_AS_SOFTREF
			return m_string_value == v.m_string_value;
#else
			return to_sprite() == v.to_sprite(); // m_object_value == v.m_object_value;
#endif

		case NUMBER:
		{
			double a = m_number_value;
			double b = v.m_number_value;

			// Nan != NaN
			//if ( isnan(a) || isnan(b) ) return false;

			if ( isnan(a) && isnan(b) ) return true;

			// -0.0 == 0.0
			if ( (a == -0 && b == 0) || (a == 0 && b == -0) ) return true;

			return a == b;
		}
		default:
			if (is_exception())
				return false; // Exceptions equal nothing.

	}
	assert(0);
	return false;
}

bool
as_value::strictly_equals(const as_value& v) const
{
	if ( m_type != v.m_type ) return false;
	return equalsSameType(v);
}

std::string
as_value::to_debug_string() const
{
	char buf[512];

	switch (m_type)
	{
		case UNDEFINED:
			return "[undefined]";
		case NULLTYPE:
			return "[null]";
		case BOOLEAN:
			sprintf(buf, "[bool:%s]", m_boolean_value ? "true" : "false");
			return buf;
		case OBJECT:
			sprintf(buf, "[object(%s):%p]", typeName(*m_object_value).c_str(), (void *)m_object_value);
			return buf;
		case AS_FUNCTION:
			sprintf(buf, "[function:%p]", (void *)m_object_value);
			return buf;
		case STRING:
			return "[string:" + m_string_value + "]";
		case NUMBER:
		{
			std::stringstream stream;
			stream << m_number_value;
			return "[number:" + stream.str() + "]";
		}
		case MOVIECLIP:
		{
			sprite_instance* sp = m_object_value->to_movie();
			assert(sp); // will change in case we'll have better management :)
			snprintf(buf, 511, "[%smovieclip(%s):%p]", sp->isUnloaded() ? "dangling " : "", sp->getOrigTarget().c_str(), (void *)sp);
			buf[511] = '\0';
			return buf;
		}
		default:
			if (is_exception())
				return "[exception]";
			assert(0);
			return NULL;
	}
}

void
as_value::operator=(const as_value& v)
{
	type the_type = v.m_type;
	if (v.is_exception())
		the_type = (type) ((int) the_type - 1);

	if (the_type == UNDEFINED) set_undefined();
	else if (the_type == NULLTYPE) set_null();
	else if (the_type == BOOLEAN) set_bool(v.m_boolean_value);
	else if (the_type == STRING) set_string(v.m_string_value);
	else if (the_type == NUMBER) set_double(v.m_number_value);
	else if (the_type == OBJECT) set_as_object(v.m_object_value);

	else if (the_type == MOVIECLIP)
	{
#ifndef MOVIECLIP_AS_SOFTREF
		sprite_instance* sp = dynamic_cast<sprite_instance*>(v.m_object_value);
		assert(sp);
		set_sprite(*sp);
#else
		set_sprite(v.m_string_value);
#endif
	}

	else if (the_type == AS_FUNCTION) set_as_function(v.m_object_value->to_function());
	else 
		assert(0);

	if (v.is_exception())
		flag_exception();
}

as_value::as_value(boost::intrusive_ptr<as_object> obj)
	:
	// Initialize to non-object type here,
	// or set_as_object will call
	// drop_ref on undefined memory !!
	m_type(UNDEFINED)
{
	set_as_object(obj);
}


// Convert numeric value to string value, following ECMA-262 specification
std::string
as_value::doubleToString(double _val)
{
	// Printing formats:
	//
	// If _val > 1, Print up to 15 significant digits, then switch
	// to scientific notation, rounding at the last place and
	// omitting trailing zeroes.
	// e.g. for 9*.1234567890123456789
	// ...
	// 9999.12345678901
	// 99999.123456789
	// 999999.123456789
	// 9999999.12345679
	// 99999999.1234568
	// 999999999.123457
	// 9999999999.12346
	// 99999999999.1235
	// 999999999999.123
	// 9999999999999.12
	// 99999999999999.1
	// 999999999999999
	// 1e+16
	// 1e+17
	// ...
	// e.g. for 1*.111111111111111111111111111111111111
	// ...
	// 1111111111111.11
	// 11111111111111.1
	// 111111111111111
	// 1.11111111111111e+15
	// 1.11111111111111e+16
	// ...
	// For values < 1, print up to 4 leading zeroes after the
	// deciman point, then switch to scientific notation with up
	// to 15 significant digits, rounding with no trailing zeroes
	// e.g. for 1.234567890123456789 * 10^-i:
	// 1.23456789012346
	// 0.123456789012346
	// 0.0123456789012346
	// 0.00123456789012346
	// 0.000123456789012346
	// 0.0000123456789012346
	// 0.00000123456789012346
	// 1.23456789012346e-6
	// 1.23456789012346e-7
	// ...
	//
	// If the value is negative, just add a '-' to the start; this
	// does not affect the precision of the printed value.
	//
	// This almost corresponds to printf("%.15g") format, except
	// that %.15g switches to scientific notation at e-05 not e-06,
	// and %g always prints at least two digits for the exponent.

	// The following code gives the same results as Adobe player
	// except for
	// 9.99999999999999[39-61] e{-2,-3}. Adobe prints these as
	// 0.0999999999999999 and 0.00999999999999 while we print them
	// as 0.1 and 0.01
	// These values are at the limit of a double's precision,
	// for example, in C,
	// .99999999999999938 printfs as
	// .99999999999999933387 and
	// .99999999999999939 printfs as
	// .99999999999999944489
	// so this behaviour is probably too compiler-dependent to
	// reproduce exactly.
	//
	// There may be some milage in comparing against
	// 0.00009999999999999995 and
	// 0.000009999999999999995 instead.

	// Handle non-numeric values.
	// "printf" gives "nan", "inf", "-inf", so we check explicitly
	if(isnan(_val))
	{
		//strcpy(_str, "NaN");
		return "NaN";
	}
	else if(isinf(_val))
	{
		return _val < 0 ? "-Infinity" : "Infinity";
		//strcpy(_str, _val < 0 ? "-Infinity" : "Infinity");
	}
	else if(_val == 0.0 || _val == -0.0)
	{
		return "0";
		//strcpy(_str, _val < 0 ? "-Infinity" : "Infinity");
	}

	char _str[256];

	// FP_ZERO, FP_NORMAL and FP_SUBNORMAL
	if (fabs(_val) < 0.0001 && fabs(_val) >= 0.00001)
	{
		// This is the range for which %.15g gives scientific
		// notation but for which we must give decimal.
		// We can't easily use %f bcos it prints a fixed number
		// of digits after the point, not the maximum number of
		// significant digits with trailing zeroes removed that
		// we require. So we just get %g to do its non-e stuff
		// by multiplying the value by ten and then stuffing
		// an extra zero into the result after the decimal
		// point. Yuk!
		char *cp;
		
		sprintf(_str, "%.15g", _val * 10.0);
		if ((cp = strchr(_str, '.')) == NULL || cp[1] != '0') {
			log_error(_("Internal error: Cannot find \".0\" in %s for %.15g"), _str, _val);
			// Just give it to them raw instead
			sprintf(_str, "%.15g", _val);
		} else {
#if HAVE_MEMMOVE
			// Shunt the digits right one place after the
			// decimal point.
			memmove(cp+2, cp+1, strlen(cp+1)+1);
#else
			// We can't use strcpy() cos the args overlap.

			char c;	// character being moved forward
			
			// At this point, cp points at the '.'
			//
			// In the loop body it points at where we pick
			// up the next char to move forward and where
			// we drop the one we picked up on its left.
			// We stop when we have just picked up the \0.
			for (c = '0', cp++; c != '\0'; cp++) {
				char tmp = *cp; *cp = c; c = tmp;
			}
			// Store the '\0' we just picked up
			*cp = c;
#endif
		}
	}
	else
	{
		// Regular case
		char *cp;

		sprintf(_str, "%.15g", _val);
		// Remove a leading zero from 2-digit exponent if any
		if ((cp = strchr(_str, 'e')) != NULL &&
		    cp[2] == '0') {
			// We can't use strcpy() cos its src&dest can't
			// overlap. However, this can only be "...e+0n"
			// or ...e-0n;  3+digit exponents never have
			// leading 0s.
			cp[2] = cp[3]; cp[3] = '\0';
		}
	}

	return std::string(_str);
}

void
as_value::setReachable() const
{
#ifdef GNASH_USE_GC
#ifdef MOVIECLIP_AS_SOFTREF
	if ( m_type == OBJECT || m_type == AS_FUNCTION ||
		m_type == OBJECT_EXCEPT)
#else
	if ( m_type == OBJECT || m_type == AS_FUNCTION || m_type == MOVIECLIP
		|| m_type == OBJECT_EXCEPT)
#endif
	{
		m_object_value->setReachable();
	}
#endif // GNASH_USE_GC
}

} // namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
