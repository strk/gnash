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
#include <boost/lexical_cast.hpp>


using namespace std;

#ifdef WIN32
#	define snprintf _snprintf
#endif

// Define the macro below to make abstract equality operator verbose
//#define GNASH_DEBUG_EQUALITY 1

// Define the macro below to make to_primitive verbose
//#define GNASH_DEBUG_CONVERSION_TO_PRIMITIVE 1

namespace gnash {

//
// as_value -- ActionScript value type
//

as_value::as_value(as_function* func)
    :
    m_type(AS_FUNCTION)
{
	if ( func )
	{
		_value = boost::intrusive_ptr<as_object>(func);
	}
	else
	{
		m_type = NULLTYPE;
		_value = boost::blank();
	}
}

// Conversion to const std::string&.
std::string
as_value::to_string(as_environment* env) const
{
	switch (m_type)
	{

		case STRING:
			return getStr();

		case MOVIECLIP:
		{
			return getSpriteProxy().getTarget();
		}

		case NUMBER:
		{
			double d = getNum();
			return doubleToString(d);
		}

		case UNDEFINED: 

			// Behavior depends on file version.  In
			// version 7+, it's "undefined", in versions
			// 6-, it's "".
			//
			// We'll go with the v7 behavior by default,
			// and conditionalize via _versioned()
			// functions.
			// 
			return "undefined";

		case NULLTYPE:
			return "null";

		case BOOLEAN:
		{
			bool b = getBool();
			return b ? "true" : "false";
		}

		case OBJECT:
		case AS_FUNCTION:
		{
			as_object* obj = m_type == OBJECT ? getObj().get() : getFun().get();
			if ( env )
			{
				try
				{
					as_value ret = to_primitive(*env, STRING);
					// This additional is_string test is NOT compliant with ECMA-262
					// specification, but seems required for compatibility with the
					// reference player.
#if GNASH_DEBUG_CONVERSION_TO_PRIMITIVE
					log_debug(" %s.to_primitive(STRING) returned %s", to_debug_string().c_str(), ret.to_debug_string().c_str());
#endif
					if ( ret.is_string() ) return ret.to_string();
				}
				catch (ActionTypeError& e)
				{
					log_debug(_("to_primitive(%s, STRING) threw an ActionTypeError %s"),
							to_debug_string().c_str(), e.what());
				}
			}
			else log_debug("%s.to_number() called w/out env", to_debug_string().c_str());

			if ( m_type == OBJECT ) return "[type Object]";
			assert(m_type == AS_FUNCTION);
			return "[type Function]";

		}

		default:
			return "[exception]";
    }
    
}

// Conversion to const std::string&.
std::string
as_value::to_string_versioned(int version, as_environment* env) const
{
	if (m_type == UNDEFINED)
	{
		// Version-dependent behavior.
		if (version <= 6)
		{
		    return "";
		}
		return "undefined";
	}
		
	return to_string(env);
}

primitive_types
as_value::ptype() const
{
	VM& vm = VM::get();
	int swfVersion = vm.getSWFVersion();

	switch (m_type)
	{
	case STRING: return PTYPE_STRING;
	case NUMBER: return PTYPE_NUMBER;
	case AS_FUNCTION:
	case UNDEFINED:
	case NULLTYPE:
	case MOVIECLIP:
		return PTYPE_NUMBER;
	case OBJECT:
	{
		as_object* obj = getObj().get();
		// Date objects should return TYPE_STRING (but only from SWF6 up)
		// See ECMA-262 8.6.2.6
		if ( swfVersion > 5 && obj->isDateObject() ) return PTYPE_STRING;
		return PTYPE_NUMBER;
	}
	case BOOLEAN:
		return PTYPE_BOOLEAN;
	default:
		break; // Should be only exceptions here.
	}
	return PTYPE_NUMBER;
}

// Conversion to primitive value.
as_value
as_value::to_primitive(as_environment& env) const
{
	VM& vm = VM::get();
	int swfVersion = vm.getSWFVersion();

	type hint = NUMBER;

	if ( m_type == OBJECT && swfVersion > 5 && getObj()->isDateObject() )
	{
		hint = STRING;
	}

#if 0
	else if ( m_type == MOVIECLIP && swfVersion > 5 )
	{
		throw ActionTypeError();
	}
#endif

	return to_primitive(env, hint);
}

// Conversion to primitive value.
as_value
as_value::to_primitive(as_environment& env, type hint) const
{
	if ( m_type != OBJECT && m_type != AS_FUNCTION ) return *this; 
	//if ( ! is_object() ) return *this; // include MOVIECLIP !!

#if GNASH_DEBUG_CONVERSION_TO_PRIMITIVE
	log_debug("to_primitive(%s)", hint==NUMBER ? "NUMBER" : "STRING");
#endif 

	// TODO: implement as_object::DefaultValue (ECMA-262 - 8.6.2.6)

	as_value method;
	as_object* obj = NULL;

	if (hint == NUMBER)
	{
#if 1
		if ( m_type == MOVIECLIP )
		{
			return as_value(NAN);
		}
#endif
		if ( m_type == OBJECT ) obj = getObj().get();
		else obj = getFun().get();

		if ( (!obj->get_member(NSV::PROP_VALUE_OF, &method)) || (!method.is_function()) ) // ECMA says ! is_object()
		{
#if GNASH_DEBUG_CONVERSION_TO_PRIMITIVE
			log_debug(" valueOf not found");
#endif
			if ( (!obj->get_member(NSV::PROP_TO_STRING, &method)) || (!method.is_function()) ) // ECMA says ! is_object()
			{
#if GNASH_DEBUG_CONVERSION_TO_PRIMITIVE
				log_debug(" toString not found");
#endif
				throw ActionTypeError();
			}
		}
	}
	else
	{
		assert(hint==STRING);

#if 1
		if ( m_type == MOVIECLIP )
		{
			return as_value(getSpriteProxy().getTarget());
		}
#endif

		if ( m_type == OBJECT ) obj = getObj().get();
		else obj = getFun().get();

		//printf("as_value to string conversion, env=%p\n", env);
		// @@ Moock says, "the value that results from
		// calling toString() on the object".
		//
		// When the toString() method doesn't exist, or
		// doesn't return a valid number, the default
		// text representation for that object is used
		// instead.
		//
		if ( ! obj->useCustomToString() )
		{
#if GNASH_DEBUG_CONVERSION_TO_PRIMITIVE
			log_debug(" not using custom toString");
#endif
			return as_value(obj->get_text_value());
		}

		if ( (!obj->get_member(NSV::PROP_TO_STRING, &method)) || (!method.is_function()) ) // ECMA says ! is_object()
		{
#if GNASH_DEBUG_CONVERSION_TO_PRIMITIVE
			log_debug(" toString not found");
#endif
			if ( (!obj->get_member(NSV::PROP_VALUE_OF, &method)) || (!method.is_function()) ) // ECMA says ! is_object()
			{
#if GNASH_DEBUG_CONVERSION_TO_PRIMITIVE
				log_debug(" valueOf not found");
#endif
				//return as_value(obj->get_text_value());
				throw ActionTypeError();
			}
		}
	}

	assert(obj);

	as_value ret = call_method0(method, &env, obj);
#if GNASH_DEBUG_CONVERSION_TO_PRIMITIVE
	log_debug("to_primitive: method call returned %s", ret.to_debug_string().c_str());
#endif
	if ( ret.m_type == OBJECT || ret.m_type == AS_FUNCTION ) // not a primitive 
	{
		throw ActionTypeError();
	}


	return ret;

}

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

			try { 

				double d = boost::lexical_cast<double>(getStr());

				if ( isinf(d) ) {
					return (double)NAN;
				}

				return d;

			} catch (boost::bad_lexical_cast &) {

				return (double)NAN;

			}

		}

		case NULLTYPE:
		case UNDEFINED:
			// Evan: from my tests
			// Martin: FlashPlayer6 gives 0; FP9 gives NaN.
			return ( swfversion >= 7 ? NAN : 0 );

		case BOOLEAN:
			// Evan: from my tests
			// Martin: confirmed
			return getBool() ? 1 : 0;

		case NUMBER:
			return getNum();

		case OBJECT:
		case AS_FUNCTION:
		    {
			// @@ Moock says the result here should be
			// "the return value of the object's valueOf()
			// method".
			//
			// Arrays and Movieclips should return NaN.

			//log_msg(_("OBJECT to number conversion, env is %p"), env);

			as_object* obj = m_type == OBJECT ? getObj().get() : getFun().get();
			if ( env )
			{
				try
				{
					as_value ret = to_primitive(*env, NUMBER);
					// env shouldn't be needed as to_primitive ensure a primitive type is returned
					return ret.to_number();
				}
				catch (ActionTypeError& e)
				{
					log_debug(_("to_primitive(%s, NUMBER) threw an ActionTypeError %s"),
							to_debug_string().c_str(), e.what());
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
				log_debug("%s.to_number() called w/out env", to_debug_string().c_str());
				return obj->get_numeric_value(); 
			}
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
			return getStr() != "";
		case NUMBER:
		{
			double d = getNum();
			return d && ! isnan(d);
		}
		case BOOLEAN:
			return getBool();
		case OBJECT:
		case AS_FUNCTION:
			return true;

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
			if (getStr() == "false") return false;
			else if (getStr() == "true") return true;
			else
			{
				double num = to_number();
				bool ret = num && ! isnan(num);
				return ret;
			}
		}
		case NUMBER:
		{
			double d = getNum();
			return ! isnan(d) && d; 
		}
		case BOOLEAN:
			return getBool();
		case OBJECT:
		case AS_FUNCTION:
			return true;

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
			if (getStr() == "false") return false;
			else if (getStr() == "true") return true;
			else
			{
				double num = to_number();
				bool ret = num && ! isnan(num);
				return ret;
			}
		}
		case NUMBER:
		{
			double d = getNum();
			return isfinite(d) && d;
		}
		case BOOLEAN:
			return getBool();
		case OBJECT:
		case AS_FUNCTION:
			return true;

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
			return getObj();

		case AS_FUNCTION:
			return getFun().get();

		case MOVIECLIP:
			// FIXME: update when to_sprite will return
			//        an intrusive_ptr directly
			return ptr(to_sprite());

		case STRING:
			return init_string_instance(getStr().c_str());

		case NUMBER:
			return init_number_instance(getNum());

		case BOOLEAN:
			return init_boolean_instance(getBool());

		default:
			// Invalid to convert exceptions.
			return NULL;
	}
}

sprite_instance*
as_value::to_sprite() const
{
	if ( m_type != MOVIECLIP ) return NULL;

	return getSprite();
}

void
as_value::set_sprite(sprite_instance& sprite)
{
	drop_refs();
	m_type = MOVIECLIP;
	_value = SpriteProxy(&sprite);
}

// Return value as an ActionScript function.  Returns NULL if value is
// not an ActionScript function.
as_function*
as_value::to_as_function() const
{
    if (m_type == AS_FUNCTION) {
	// OK.
	return getFun().get();
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
    std::string ns = to_string();
    drop_refs();
    m_type = STRING;	// force type.
    _value = ns;
}


void
as_value::convert_to_string_versioned(int version, as_environment* env)
    // Force type to string.
{
    std::string ns = to_string_versioned(version, env);
    drop_refs();
    m_type = STRING;	// force type.
    _value = ns;
}


void
as_value::set_undefined()
{
	drop_refs();
	m_type = UNDEFINED;
	_value = boost::blank();
}

void
as_value::set_null()
{
	drop_refs();
	m_type = NULLTYPE;
	_value = boost::blank();
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
	if (m_type != OBJECT || getObj() != obj)
	{
		drop_refs();
		m_type = OBJECT;
		_value = boost::intrusive_ptr<as_object>(obj);
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
    if (m_type != AS_FUNCTION || getFun().get() != func)
    {
	drop_refs();
	m_type = AS_FUNCTION;
	if (func)
	{
		_value = boost::intrusive_ptr<as_object>(func);
	}
	else
	{
		m_type = NULLTYPE;
		_value = boost::blank(); // to properly destroy anything else might be stuffed into it
	}
    }
}

bool
as_value::conforms_to(string_table::key name)
{
	// TODO: Implement
	return false;
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
       log_debug(" one of the two things is undefined or null");
#endif
        return this_nulltype == v_nulltype;
    }

    bool obj_or_func = (m_type == OBJECT || m_type == AS_FUNCTION);
    bool v_obj_or_func = (v.m_type == OBJECT || v.m_type == AS_FUNCTION);

    /// Compare to same type
    if ( obj_or_func && v_obj_or_func )
    {
        return boost::get<AsObjPtr>(_value) == boost::get<AsObjPtr>(v._value); 
    }

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
        return as_value(to_number(&env)).equals(v, env); 
    }

    // 19. If Type(y) is Boolean, return the result of the comparison x == ToNumber(y).
    if (v.m_type == BOOLEAN)
    {
        return as_value(v.to_number(&env)).equals(*this, env); 
    }

    // 20. If Type(x) is either String or Number and Type(y) is Object,
    //     return the result of the comparison x == ToPrimitive(y).
    if ( (m_type == STRING || m_type == NUMBER ) && ( v.m_type == OBJECT || v.m_type == AS_FUNCTION ) )
    {
        // convert this value to a primitive and recurse
	try
	{
		as_value v2 = v.to_primitive(env); 
		if ( v.strictly_equals(v2) ) return false;

#ifdef GNASH_DEBUG_EQUALITY
		log_debug(" 20: convertion to primitive : %s -> %s", v.to_debug_string().c_str(), v2.to_debug_string().c_str());
#endif

		return equals(v2, env);
	}
	catch (ActionTypeError& e)
	{
//#ifdef GNASH_DEBUG_EQUALITY
		log_debug(" %s.to_primitive() threw an ActionTypeError %s", v.to_debug_string().c_str(), e.what());
//#endif
		return false; // no valid conversion
	}

    }

    // 21. If Type(x) is Object and Type(y) is either String or Number,
    //    return the result of the comparison ToPrimitive(x) == y.
    if ( (v.m_type == STRING || v.m_type == NUMBER ) && ( m_type == OBJECT || m_type == AS_FUNCTION ) )
    {
        // convert this value to a primitive and recurse
        try
	{
        	as_value v2 = to_primitive(env); 
		if ( strictly_equals(v2) ) return false;

#ifdef GNASH_DEBUG_EQUALITY
		log_debug(" 21: convertion to primitive : %s -> %s", to_debug_string().c_str(), v2.to_debug_string().c_str());
#endif

		return v2.equals(v, env);
	}
	catch (ActionTypeError& e)
	{

//#ifdef GNASH_DEBUG_EQUALITY
		log_debug(" %s.to_primitive() threw an ActionTypeError %s", to_debug_string().c_str(), e.what());
//#endif

		return false; // no valid conversion
	}

    }

	// Both operands are objects (OBJECT,AS_FUNCTION,MOVIECLIP)
	if ( ! is_object() || ! v.is_object() )
	{
		log_error("Equals(%s,%s)", to_debug_string().c_str(), v.to_debug_string().c_str());
	}

    // If any of the two converts to a primitive, we recurse

	as_value p = *this;
	as_value vp = v;

	int converted = 0;
	try
	{
		p = to_primitive(env); 
		if ( ! strictly_equals(p) ) ++converted;
#ifdef GNASH_DEBUG_EQUALITY
		log_debug(" convertion to primitive (this): %s -> %s", to_debug_string().c_str(), p.to_debug_string().c_str());
#endif
	}
	catch (ActionTypeError& e)
	{
#ifdef GNASH_DEBUG_CONVERSION_TO_PRIMITIVE 
		log_debug(" %s.to_primitive() threw an ActionTypeError %s",
			to_debug_string().c_str(), e.what());
#endif
	}

	try
	{
		vp = v.to_primitive(env); 
		if ( ! v.strictly_equals(vp) ) ++converted;
#ifdef GNASH_DEBUG_EQUALITY
		log_debug(" convertion to primitive (that): %s -> %s", v.to_debug_string().c_str(), vp.to_debug_string().c_str());
#endif
	}
	catch (ActionTypeError& e)
	{
#ifdef GNASH_DEBUG_CONVERSION_TO_PRIMITIVE 
		log_debug(" %s.to_primitive() threw an ActionTypeError %s",
			v.to_debug_string().c_str(), e.what());
#endif
	}

	if ( converted )
	{
#ifdef GNASH_DEBUG_EQUALITY
		log_debug(" some conversion took place, recurring");
#endif
		return p.equals(vp, env);
	}
	else
	{
#ifdef GNASH_DEBUG_EQUALITY
		log_debug(" no conversion took place, returning false");
#endif
		return false;
	}


}
	
// Sets *this to this string plus the given string.
void
as_value::string_concat(const std::string& str)
{
    std::string currVal = to_string();
    m_type = STRING;
    _value = currVal + str;
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
			abort();
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
		case BOOLEAN:
		case STRING:
			return _value == v._value;

		case MOVIECLIP:
			return to_sprite() == v.to_sprite(); 

		case NUMBER:
		{
			double a = getNum();
			double b = v.getNum();

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
	abort();
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
			sprintf(buf, "[bool:%s]", getBool() ? "true" : "false");
			return buf;
		case OBJECT:
		{
			as_object* obj = getObj().get();
			sprintf(buf, "[object(%s):%p]", typeName(*obj).c_str(), (void *)obj);
			return buf;
		}
		case AS_FUNCTION:
		{
			as_function* obj = getFun().get();
			sprintf(buf, "[function:%p]", (void *)obj);
			return buf;
		}
		case STRING:
			return "[string:" + getStr() + "]";
		case NUMBER:
		{
			std::stringstream stream;
			stream << getNum();
			return "[number:" + stream.str() + "]";
		}
		case MOVIECLIP:
		{
			const SpriteProxy& sp = getSpriteProxy();
			if ( sp.isDangling() )
			{
				sprite_instance* rebound = sp.get();
				if ( rebound )
				{
					snprintf(buf, 511, "[rebound movieclip(%s):%p]", sp.getTarget().c_str(), rebound);
				}
				else
				{
					snprintf(buf, 511, "[dangling movieclip:%s]", sp.getTarget().c_str());
				}
			}
			else
			{
				snprintf(buf, 511, "[movieclip(%s):%p]", sp.getTarget().c_str(), (void *)sp.get());
			}
			buf[511] = '\0';
			return buf;
		}
		default:
			if (is_exception())
				return "[exception]";
			abort();
			return "[invalid type]";
	}
}

void
as_value::operator=(const as_value& v)
{
#if 0
	type the_type = v.m_type;
	if (v.is_exception())
		the_type = (type) ((int) the_type - 1);
#endif

	m_type = v.m_type;
	_value = v._value;

#if 0
	if (v.is_exception())
		flag_exception();
#endif
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
	switch (m_type)
	{
		case OBJECT:
			getObj()->setReachable();
			break;

		case AS_FUNCTION:
			getFun()->setReachable();
			break;

		case MOVIECLIP:
			getSpriteProxy().setReachable();
			break;

		default: break;
	}
#endif // GNASH_USE_GC
}

as_value::AsFunPtr
as_value::getFun() const
{
	assert(m_type == AS_FUNCTION);
	return boost::get<AsObjPtr>(_value)->to_function();
}

as_value::AsObjPtr
as_value::getObj() const
{
	assert(m_type == OBJECT);
	return boost::get<AsObjPtr>(_value);
}

as_value::SpriteProxy
as_value::getSpriteProxy() const
{
	assert(m_type == MOVIECLIP);
	return boost::get<SpriteProxy>(_value);
}

as_value::SpritePtr
as_value::getSprite() const
{
	assert(m_type == MOVIECLIP);
	return boost::get<SpriteProxy>(_value).get();
}

void
as_value::set_string(const std::string& str)
{
	drop_refs();
	m_type = STRING;
	_value = str;
}

void
as_value::set_double(double val)
{
	drop_refs();
	m_type = NUMBER;
	_value = val;
}

void
as_value::set_bool(bool val)
{
	drop_refs();
	m_type = BOOLEAN;
	_value = val;
}

as_value::as_value()
	:
	m_type(UNDEFINED),
	_value(boost::blank())
{
}

as_value::as_value(const as_value& v)
	:
	m_type(v.m_type),
	_value(v._value)
{
}

as_value::as_value(const char* str)
	:
	m_type(STRING),
	_value(std::string(str))
{
}

as_value::as_value(const std::string& str)
	:
	m_type(STRING),
	_value(str)
{
}

as_value::as_value(bool val)
	:
	m_type(BOOLEAN),
	_value(val)
{
}

as_value::as_value(int val)
	:
	m_type(NUMBER),
	_value(double(val))
{
}

as_value::as_value(unsigned int val)
	:
	m_type(NUMBER),
	_value(double(val))
{
}

as_value::as_value(float val)
	:
	m_type(NUMBER),
	_value(double(val))
{
}

as_value::as_value(double val)
	:
	m_type(NUMBER),
	_value(val)
{
}

as_value::as_value(long val)
	:
	m_type(NUMBER),
	_value(double(val))
{
}

as_value::as_value(unsigned long val)
	:
	m_type(NUMBER),
	_value(double(val))
{
}

as_value::as_value(as_object* obj)
	:
	// Initialize to non-object type here,
	// or set_as_object will call
	// drop_ref on undefined memory !!
	m_type(UNDEFINED)
{
	set_as_object(obj);
}

//-------------------------------------
// as_value::SpriteProxy
//-------------------------------------

/* static private */
sprite_instance*
as_value::SpriteProxy::find_sprite_by_target(const std::string& tgtstr)
{
	if ( tgtstr.empty() ) return NULL;

	sprite_instance* root = VM::get().getRoot().get_root_movie();
	as_environment& env = root->get_environment();
	character* target = env.find_target(tgtstr);
	if ( ! target ) return NULL;
	return target->to_movie();
}

void
as_value::SpriteProxy::checkDangling() const
{
	if ( _ptr && _ptr->isDestroyed() ) 
	{
		_tgt = _ptr->getOrigTarget();
		_ptr = 0;
	}
}

std::string
as_value::SpriteProxy::getTarget() const
{
	checkDangling(); // set _ptr to NULL and _tgt to original target if destroyed
	if ( _ptr ) return _ptr->getTarget();
	else return _tgt;
}

void
as_value::SpriteProxy::setReachable() const
{
	checkDangling();
	if ( _ptr ) _ptr->setReachable();
}

} // namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
