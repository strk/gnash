// as_value.cpp:  ActionScript values, for Gnash.
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
//

#include "smart_ptr.h" // GNASH_USE_GC
#include "as_value.h"
#include "as_object.h"
#include "as_function.h" // for as_function
#include "MovieClip.h" // for MOVIECLIP values
#include "DisplayObject.h" // for MOVIECLIP values
#include "as_environment.h" // for MOVIECLIP values
#include "VM.h" // for MOVIECLIP values
#include "movie_root.h" // for MOVIECLIP values
#include "action.h" // for call_method0
#include "utility.h" // for typeName() 
#include "GnashNumeric.h"
#include "namedStrings.h"
#include "element.h"
#include "GnashException.h"
#include "Object.h"
#include "amf.h"
#include "Array_as.h"
#include "Date_as.h" // for Date type (readAMF0)
#include "SimpleBuffer.h"
#include "StringPredicates.h"
#include "Global_as.h"

#include <boost/shared_ptr.hpp>
#include <cmath> 
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <locale>
#include <sstream>
#include <iomanip>
#include <string>

// Define the macro below to make abstract equality operator verbose
//#define GNASH_DEBUG_EQUALITY 1

// Define the macro below to make to_primitive verbose
// #define GNASH_DEBUG_CONVERSION_TO_PRIMITIVE 1

// Define this macro to make soft references activity verbose
#define GNASH_DEBUG_SOFT_REFERENCES

// Define this macro to make AMF parsing verbose
#define GNASH_DEBUG_AMF_DESERIALIZE 1

// Define this macto to make AMF writing verbose
// #define GNASH_DEBUG_AMF_SERIALIZE 1

namespace gnash {

namespace {

struct invalidHexDigit {};
boost::uint8_t parseHex(char c)
{
	switch (c)
	{
		case '0': return 0;
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		case 'a': case 'A': return 10;
		case 'b': case 'B': return 11;
		case 'c': case 'C': return 12;
		case 'd': case 'D': return 13;
		case 'e': case 'E': return 14;
		case 'f': case 'F': return 15;
		default: throw invalidHexDigit();
	}
}

/// Truncates a double to a 32-bit unsigned int.
//
/// In fact, it is a 32-bit unsigned int with an additional sign, cast
/// to an unsigned int. Not sure what the sense is, but that's how it works:
//
/// 0xffffffff is interpreted as -1, -0xffffffff as 1.
boost::int32_t
truncateToInt(double d)
{
    if (d < 0)
    {   
	    return - static_cast<boost::uint32_t>(std::fmod(-d, 4294967296.0));
    }
	
    return static_cast<boost::uint32_t>(std::fmod(d, 4294967296.0));
}

enum Base
{
    BASE_OCT,
    BASE_HEX
};


/// Converts a string to a uint32_t cast to an int32_t.
//
/// @param whole    When true, any string that isn't wholly valid is rejected.
/// @param base     The base (8 or 16) to use.
/// @param s        The string to parse.
/// @return         The converted number.
boost::int32_t
parsePositiveInt(const std::string& s, Base base, bool whole = true)
{

    std::istringstream is(s);
    boost::uint32_t target;

    switch (base)
    {
        case BASE_OCT:
            is >> std::oct;
            break;
        case BASE_HEX:
            is >> std::hex;
            break;
    }

    char c;

    // If the cast fails, or if the whole string must be convertible and
    // some DisplayObjects are left, throw an exception.
    if (!(is >> target) || (whole && is.get(c))) {
        throw boost::bad_lexical_cast();
    }

    return target;
}

// This class is used to iterate through all the properties of an AS object,
// so we can change them to children of an AMF0 element.
class PropsSerializer : public AbstractPropertyVisitor
{

public:

    PropsSerializer(amf::Element& el, VM& vm)
        :
        _obj(el),
	    _st(vm.getStringTable())
	{}
    
    void accept(string_table::key key, const as_value& val) 
    {

        // Test conducted with AMFPHP:
        // '__proto__' and 'constructor' members
        // of an object don't get back from an 'echo-service'.
        // Dunno if they are not serialized or just not sent back.
        // A '__constructor__' member gets back, but only if 
        // not a function. Actually no function gets back.
        // 
        if (key == NSV::PROP_uuPROTOuu || key == NSV::PROP_CONSTRUCTOR)
        {
#ifdef GNASH_DEBUG_AMF_SERIALIZE
            log_debug(" skip serialization of specially-named property %s",
                    _st.value(key));
#endif
            return;
        }

        amf::AMF amf;
        boost::shared_ptr<amf::Element> el;
    
        const std::string& name = _st.value(key);

        if (val.is_string()) {
            std::string str;
            if (!val.is_undefined()) {
                str = val.to_string();
            }
            el.reset(new amf::Element(name, str));
        } else if (val.is_bool()) {
            bool flag = val.to_bool();
            el.reset(new amf::Element(name, flag));
        } else if (val.is_object()) {
//            el.reset(new amf::Element(name, flag));
        } else if (val.is_null()) {
	    boost::shared_ptr<amf::Element> tmpel(new amf::Element);
	    tmpel->setName(name);
	    tmpel->makeNull();
            el = tmpel;
        } else if (val.is_undefined()) {
	    boost::shared_ptr<amf::Element> tmpel(new amf::Element);
	    tmpel->setName(name);
	    tmpel->makeUndefined();
            el = tmpel;
        } else if (val.is_number()) { 
            double dub;
            if (val.is_undefined()) {
                dub = 0.0;
            } else {
                dub = val.to_number();
            }
            el.reset(new amf::Element(name, dub));
        }
    
        if (el) {
            _obj.addProperty(el);
        }
    }

private:

    amf::Element& _obj;
    string_table& _st;

};

} // anonymous namespace

/// Class used to serialize properties of an object to a buffer
class PropsBufSerializer : public AbstractPropertyVisitor
{

    typedef std::map<as_object*, size_t> PropertyOffsets;

public:
    PropsBufSerializer(SimpleBuffer& buf, VM& vm,
            PropertyOffsets& offsetTable, bool allowStrict)
        :
        _allowStrict(allowStrict),
        _buf(buf),
        _vm(vm),
        _st(vm.getStringTable()),
        _offsetTable(offsetTable),
        _error(false)
	{}
    
    bool success() const { return !_error; }

    void accept(string_table::key key, const as_value& val) 
    {
        if ( _error ) return;

        // Tested with SharedObject and AMFPHP
        if ( val.is_function() )
        {
            log_debug("AMF0: skip serialization of FUNCTION property");
            return;
        }

        // Test conducted with AMFPHP:
        // '__proto__' and 'constructor' members
        // of an object don't get back from an 'echo-service'.
        // Dunno if they are not serialized or just not sent back.
        // A '__constructor__' member gets back, but only if 
        // not a function. Actually no function gets back.
        // 
        if (key == NSV::PROP_uuPROTOuu || key == NSV::PROP_CONSTRUCTOR)
        {
#ifdef GNASH_DEBUG_AMF_SERIALIZE
            log_debug(" skip serialization of specially-named property %s",
                    _st.value(key));
#endif
            return;
        }

        // write property name
        const std::string& name = _st.value(key);
#ifdef GNASH_DEBUG_AMF_SERIALIZE
        log_debug(" serializing property %s", name);
#endif
        boost::uint16_t namelen = name.size();
        _buf.appendNetworkShort(namelen);
        _buf.append(name.c_str(), namelen);
        if ( ! val.writeAMF0(_buf, _offsetTable, _vm, _allowStrict) )
        {
            log_error("Problems serializing an object's member");
            _error=true;
        }
    }
private:

    bool _allowStrict;
    SimpleBuffer& _buf;
    VM& _vm;
    string_table& _st;
    PropertyOffsets& _offsetTable;
    mutable bool _error;

};
    
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
as_value::to_string() const
{
	switch (m_type)
	{

		case STRING:
			return getStr();

		case MOVIECLIP:
		{
			const CharacterProxy& sp = getCharacterProxy();
			if ( ! sp.get() )
			{
				return "";
			}
			else
			{
				return sp.getTarget();
			}
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

		case AS_FUNCTION:
		case OBJECT:
		{
			try
			{
				as_value ret = to_primitive(STRING);
				// This additional is_string test is NOT compliant with ECMA-262
				// specification, but seems required for compatibility with the
				// reference player.
#if GNASH_DEBUG_CONVERSION_TO_PRIMITIVE
				log_debug(" %s.to_primitive(STRING) returned %s", 
						*this,
						ret);
#endif
				if ( ret.is_string() ) return ret.to_string();
			}
			catch (ActionTypeError& e)
			{
#if GNASH_DEBUG_CONVERSION_TO_PRIMITIVE
				log_debug(_("to_primitive(%s, STRING) threw an ActionTypeError %s"),
						*this, e.what());
#endif
			}

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
as_value::to_string_versioned(int version) const
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
		
	return to_string();
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
as_value::to_primitive() const
{
	VM& vm = VM::get();
	int swfVersion = vm.getSWFVersion();

	AsType hint = NUMBER;

	if ( m_type == OBJECT && swfVersion > 5 && getObj()->isDateObject() )
	{
		hint = STRING;
	}

	return to_primitive(hint);
}

as_value&
as_value::convert_to_primitive()
{
	VM& vm = VM::get();
	int swfVersion = vm.getSWFVersion();

	AsType hint = NUMBER;

	if ( m_type == OBJECT && swfVersion > 5 && getObj()->isDateObject() )
	{
		hint = STRING;
	}

	return convert_to_primitive(hint);
}

// Conversion to primitive value.
as_value
as_value::to_primitive(AsType hint) const
{
	if ( m_type != OBJECT && m_type != AS_FUNCTION ) return *this; 

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
			return as_value(NaN);
		}
#endif
		if ( m_type == OBJECT ) obj = getObj().get();
		else obj = getFun().get();

		if ( (!obj->get_member(NSV::PROP_VALUE_OF, &method)) || (!method.is_object()) ) // ECMA says ! is_object()
		{
#if GNASH_DEBUG_CONVERSION_TO_PRIMITIVE
			log_debug(" valueOf not found");
#endif
            // Returning undefined here instead of throwing
            // a TypeError passes tests in actionscript.all/Object.as
            // and many swfdec tests, with no new failures (though
            // perhaps we aren't testing enough).
            return as_value();
		}
	}
	else
	{
		assert(hint==STRING);

		if ( m_type == MOVIECLIP )
		{
			return as_value(getCharacterProxy().getTarget());
		}

		if ( m_type == OBJECT ) obj = getObj().get();
		else obj = getFun().get();

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

		if ( (!obj->get_member(NSV::PROP_TO_STRING, &method)) ||
                (!method.is_function()) ) // ECMA says ! is_object()
		{
#if GNASH_DEBUG_CONVERSION_TO_PRIMITIVE
			log_debug(" toString not found");
#endif
			if ( (!obj->get_member(NSV::PROP_VALUE_OF, &method)) ||
                    (!method.is_function()) ) // ECMA says ! is_object()
			{
#if GNASH_DEBUG_CONVERSION_TO_PRIMITIVE
				log_debug(" valueOf not found");
#endif
				throw ActionTypeError();
			}
		}
	}

	assert(obj);

	as_environment env(getVM(*obj));
	as_value ret = call_method0(method, env, obj);
#if GNASH_DEBUG_CONVERSION_TO_PRIMITIVE
	log_debug("to_primitive: method call returned %s", ret);
#endif
	if ( ret.m_type == OBJECT || ret.m_type == AS_FUNCTION ) // not a primitive 
	{
		throw ActionTypeError();
	}


	return ret;

}

// Conversion to primitive value.
as_value&
as_value::convert_to_primitive(AsType hint) 
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
		if ( m_type == MOVIECLIP )
		{
			set_double(NaN);
			return *this;
		}
		if ( m_type == OBJECT ) obj = getObj().get();
		else obj = getFun().get();

		if ( (!obj->get_member(NSV::PROP_VALUE_OF, &method)) ||
                (!method.is_object()) ) // ECMA says ! is_object()
		{
#if GNASH_DEBUG_CONVERSION_TO_PRIMITIVE
			log_debug(" valueOf not found");
#endif
			// Returning undefined here instead of throwing
			// a TypeError passes tests in actionscript.all/Object.as
			// and many swfdec tests, with no new failures (though
			// perhaps we aren't testing enough).
			set_undefined();
			return *this;
            
		}
	}
	else
	{
		assert(hint==STRING);

		if ( m_type == MOVIECLIP )
		{
			set_string(getCharacterProxy().getTarget());
			return *this;
		}

		if ( m_type == OBJECT ) obj = getObj().get();
		else obj = getFun().get();

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
			set_string(obj->get_text_value());
			return *this;
		}

		if ( (!obj->get_member(NSV::PROP_TO_STRING, &method)) || 
                (!method.is_function()) ) // ECMA says ! is_object()
		{
#if GNASH_DEBUG_CONVERSION_TO_PRIMITIVE
			log_debug(" toString not found");
#endif
			if ( (!obj->get_member(NSV::PROP_VALUE_OF, &method)) || 
                    (!method.is_function()) ) // ECMA says ! is_object()
			{
#if GNASH_DEBUG_CONVERSION_TO_PRIMITIVE
				log_debug(" valueOf not found");
#endif
				throw ActionTypeError();
			}
		}
	}

	assert(obj);

	as_environment env(getVM(*obj));
	as_value ret = call_method0(method, env, obj);
#if GNASH_DEBUG_CONVERSION_TO_PRIMITIVE
	log_debug("to_primitive: method call returned %s", ret);
#endif
	if ( ret.m_type == OBJECT || ret.m_type == AS_FUNCTION ) // not a primitive 
	{
		throw ActionTypeError();
	}

	*this = ret;

	return *this;
}


bool
as_value::parseNonDecimalInt(const std::string& s, double& d, bool whole)
{
    const std::string::size_type slen = s.length();

    // "0#" would still be octal, but has the same value as a decimal.
    if (slen < 3) return false;

    bool negative = false;

    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
    {
        // The only legitimate place for a '-' is after 0x. If it's a
        // '+' we don't care, as it won't disturb the conversion.
        std::string::size_type start = 2;
        if (s[2] == '-') {
            negative = true;
            ++start;
        }
        d = parsePositiveInt(s.substr(start), BASE_HEX, whole);
        if (negative) d = -d;
        return true;
    }
    else if ((s[0] == '0' || ((s[0] == '-' || s[0] == '+') && s[1] == '0')) &&
            s.find_first_not_of("01234567", 1) == std::string::npos)
    {
        std::string::size_type start = 0;
        if (s[0] == '-') {
            negative = true;
            ++start;
        }
        d = parsePositiveInt(s.substr(start), BASE_OCT, whole);
        if (negative) d = -d;
        return true;
    }

    return false;

}

double
as_value::to_number() const
{

    const int swfversion = VM::get().getSWFVersion();

    switch (m_type)
    {
        case STRING:
        {
            const std::string& s = getStr();
            if ( s.empty() ) {
                return swfversion >= 5 ? NaN : 0.0;
            }
            
            if (swfversion <= 4)
            {
                // For SWF4, any valid number before non-numerical
                // DisplayObjects is returned, including exponent, positive
                // and negative signs and whitespace before.
                double d = 0;
                std::istringstream is(s);
                is >> d;
                return d;
            }

            try {

                if (swfversion > 5)
                {
                    double d;
                    // Will throw if invalid.
                    if (parseNonDecimalInt(s, d)) return d;
                }

                // @@ Moock says the rule here is: if the
                // string is a valid float literal, then it
                // gets converted; otherwise it is set to NaN.
                // Valid for SWF5 and above.
                //
                // boost::lexical_cast is remarkably inflexible and 
                // fails for anything that has non-numerical DisplayObjects.
                // Fortunately, actionscript is equally inflexible.
                std::string::size_type pos;
                if ((pos = s.find_first_not_of(" \r\n\t")) 
                        == std::string::npos) {
                    return NaN;
                }

                return boost::lexical_cast<double>(s.substr(pos));
 
            }
            catch (boost::bad_lexical_cast&) {
                // There is no standard textual representation of infinity
                // in the C++ standard, so our conversion function an
                // exception for 'inf', just like for any other
                // non-numerical text. This is correct behaviour.
                return NaN;
            }
        }

        case NULLTYPE:
        case UNDEFINED: 
        {
            // Evan: from my tests
            // Martin: FlashPlayer6 gives 0; FP9 gives NaN.
            return ( swfversion >= 7 ? NaN : 0 );
        }

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
            try
            {
                as_value ret = to_primitive(NUMBER);
                return ret.to_number();
            }
            catch (ActionTypeError& e)
            {
#if GNASH_DEBUG_CONVERSION_TO_PRIMITIVE
                log_debug(_("to_primitive(%s, NUMBER) threw an "
                            "ActionTypeError %s"), *this, e.what());
#endif
                if (m_type == AS_FUNCTION && swfversion < 6) return 0;
                
                return NaN;
            }
        }

        case MOVIECLIP:
        {
            // This is tested, no valueOf is going
            // to be invoked for movieclips.
            return NaN; 
        }

        default:
            // Other object types should return NaN.
            return NaN;
    }
}

boost::shared_ptr<amf::Element>
as_value::to_element() const
{
    VM& vm = VM::get();
    //int swfVersion = vm.getSWFVersion();
    boost::shared_ptr<amf::Element> el ( new amf::Element );
    boost::intrusive_ptr<as_object> ptr = to_object(*vm.getGlobal());

    switch (m_type) {
      case UNDEFINED:
	  el->makeUndefined();
	  break;
      case NULLTYPE:
	  el->makeNull();
	  break;
      case BOOLEAN:
	  el->makeBoolean(getBool());
	  break;
      case  STRING:
	  el->makeString(getStr());
	  break;
      case NUMBER:
	  el->makeNumber(getNum());
	  break;
      case OBJECT:
      {
	  el->makeObject();
	  PropsSerializer props(*el, vm);
	  ptr->visitPropertyValues(props);
	  break;
      }
      case AS_FUNCTION:
	  log_unimpl("Converting an AS function to an element is not supported");
          // TODO: what kind of Element will be left with ? Should we throw an exception ?
	  break;
      case MOVIECLIP:
	  log_unimpl("Converting a Movie Clip to an element is not supported");
          // TODO: what kind of Element will be left with ? Should we throw an exception ?
	  break;
      default:
	  break;
    }

    return el;
}

// This returns an as_value as an integer. It is
// probably used for most implicit conversions to 
// int, for instance in the String class.
boost::int32_t
as_value::to_int() const
{
	double d = to_number();

	if (!isFinite(d)) return 0;

    return truncateToInt(d);
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
			return d && ! isNaN(d);
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
			double num = to_number();
			bool ret = num && ! isNaN(num);
			return ret;
		}
		case NUMBER:
		{
			double d = getNum();
			return d && ! isNaN(d);
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
			double num = to_number();
			bool ret = num && ! isNaN(num);
			return ret;
		}
		case NUMBER:
		{
			double d = getNum();
            // see testsuite/swfdec/if-6.swf
			return d && ! isNaN(d);
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
as_value::to_object(Global_as& global) const
{
	typedef boost::intrusive_ptr<as_object> ptr;

	switch (m_type)
	{
		case OBJECT:
			return getObj();

		case AS_FUNCTION:
			return getFun().get();

		case MOVIECLIP:
			return ptr(toDisplayObject());

		case STRING:
			return global.createString(getStr());

		case NUMBER:
			return global.createNumber(getNum());

		case BOOLEAN:
			return global.createBoolean(getBool());

		default:
			// Invalid to convert exceptions.
			return NULL;
	}
}

MovieClip*
as_value::to_sprite(bool allowUnloaded) const
{
	if ( m_type != MOVIECLIP ) return 0;

	DisplayObject *ch = getCharacter(allowUnloaded);
	if ( ! ch ) return 0;
	return ch->to_movie();
}

DisplayObject*
as_value::toDisplayObject(bool allowUnloaded) const
{
	if ( m_type != MOVIECLIP ) return NULL;

	return getCharacter(allowUnloaded);
}

void
as_value::set_sprite(MovieClip& sprite)
{
	setDisplayObject(sprite);
}

void
as_value::setDisplayObject(DisplayObject& sprite)
{
	m_type = MOVIECLIP;
	_value = CharacterProxy(&sprite);
}

// Return value as an ActionScript function.  Returns NULL if value is
// not an ActionScript function.
as_function*
as_value::to_as_function() const
{
    if (m_type == AS_FUNCTION) {
	    // OK.
	    return getFun().get();
    }

    return NULL;
}

// Force type to number.
void
as_value::convert_to_boolean()
{
    set_bool(to_bool());
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
    std::string ns = to_string();
    m_type = STRING;	// force type.
    _value = ns;
}


void
as_value::convert_to_string_versioned(int version)
    // Force type to string.
{
    std::string ns = to_string_versioned(version);
    m_type = STRING;	// force type.
    _value = ns;
}


void
as_value::set_undefined()
{
	m_type = UNDEFINED;
	_value = boost::blank();
}

void
as_value::set_null()
{
	m_type = NULLTYPE;
	_value = boost::blank();
}

void
as_value::set_unsupported()
{
	m_type = UNSUPPORTED;
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
	DisplayObject* sp = obj->toDisplayObject();
	if ( sp )
	{
		setDisplayObject(*sp);
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
as_value::conforms_to(string_table::key /*name*/)
{
	// TODO: Implement
	return false;
}

bool
as_value::equals(const as_value& v) const
{
    // Comments starting with numbers refer to the ECMA-262 document

#ifdef GNASH_DEBUG_EQUALITY
    static int count=0;
    log_debug("equals(%s, %s) called [%d]", *this, v, count++);
#endif

    int SWFVersion = VM::get().getSWFVersion();

    bool this_nulltype = (m_type == UNDEFINED || m_type == NULLTYPE);
    bool v_nulltype = (v.m_type == UNDEFINED || v.m_type == NULLTYPE);

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
        double n = v.to_number();
        if (!isFinite(n)) return false;
        return equalsSameType(n);
    }

    // 17. If Type(x) is String and Type(y) is Number,
    //     return the result of the comparison ToNumber(x) == y.
    if (v.m_type == NUMBER && m_type == STRING)
    {
        double n = to_number();
        if (!isFinite(n)) return false;
        return v.equalsSameType(n); 
    }

    // 18. If Type(x) is Boolean, return the result of the comparison ToNumber(x) == y.
    if (m_type == BOOLEAN)
    {
        return as_value(to_number()).equals(v); 
    }

    // 19. If Type(y) is Boolean, return the result of the comparison x == ToNumber(y).
    if (v.m_type == BOOLEAN)
    {
        return as_value(v.to_number()).equals(*this); 
    }

    // 20. If Type(x) is either String or Number and Type(y) is Object,
    //     return the result of the comparison x == ToPrimitive(y).
    if ( (m_type == STRING || m_type == NUMBER ) && 
            (v.m_type == OBJECT || v.m_type == AS_FUNCTION ))
    {
        // convert this value to a primitive and recurse
	try
	{
		as_value v2 = v.to_primitive(); 
		if ( v.strictly_equals(v2) ) return false;

#ifdef GNASH_DEBUG_EQUALITY
		log_debug(" 20: convertion to primitive : %s -> %s", v, v2);
#endif

		return equals(v2);
	}
	catch (ActionTypeError& e)
	{
#ifdef GNASH_DEBUG_EQUALITY
		log_debug(" %s.to_primitive() threw an ActionTypeError %s", v, e.what());
#endif
		return false; // no valid conversion
	}

    }

    // 21. If Type(x) is Object and Type(y) is either String or Number,
    //    return the result of the comparison ToPrimitive(x) == y.
    if ((v.m_type == STRING || v.m_type == NUMBER) && 
            (m_type == OBJECT || m_type == AS_FUNCTION))
    {
        // convert this value to a primitive and recurse
        try
        {
            // Date objects default to primitive type STRING from SWF6 up,
            // but we always prefer valueOf to toString in this case.
        	as_value v2 = to_primitive(NUMBER); 
            if ( strictly_equals(v2) ) return false;

#ifdef GNASH_DEBUG_EQUALITY
            log_debug(" 21: convertion to primitive : %s -> %s", *this, v2);
#endif

            return v2.equals(v);
        }
        catch (ActionTypeError& e)
        {

#ifdef GNASH_DEBUG_EQUALITY
            log_debug(" %s.to_primitive() threw an ActionTypeError %s",
                    *this, e.what());
#endif

            return false; // no valid conversion
        }

    }


#ifdef GNASH_DEBUG_EQUALITY
	// Both operands are objects (OBJECT,AS_FUNCTION,MOVIECLIP)
	if ( ! is_object() || ! v.is_object() )
	{
		log_debug("Equals(%s,%s)", *this, v);
	}
#endif

    // If any of the two converts to a primitive, we recurse

	as_value p = *this;
	as_value vp = v;

	int converted = 0;
	try
	{
		p = to_primitive(); 
		if ( ! strictly_equals(p) ) ++converted;
#ifdef GNASH_DEBUG_EQUALITY
		log_debug(" convertion to primitive (this): %s -> %s", *this, p);
#endif
	}
	catch (ActionTypeError& e)
	{
#ifdef GNASH_DEBUG_CONVERSION_TO_PRIMITIVE 
		log_debug(" %s.to_primitive() threw an ActionTypeError %s",
			*this, e.what());
#endif
	}

	try
	{
		vp = v.to_primitive(); 
		if ( ! v.strictly_equals(vp) ) ++converted;
#ifdef GNASH_DEBUG_EQUALITY
		log_debug(" convertion to primitive (that): %s -> %s", v, vp);
#endif
	}
	catch (ActionTypeError& e)
	{
#ifdef GNASH_DEBUG_CONVERSION_TO_PRIMITIVE 
		log_debug(" %s.to_primitive() threw an ActionTypeError %s",
			v, e.what());
#endif
	}

	if ( converted )
	{
#ifdef GNASH_DEBUG_EQUALITY
		log_debug(" some conversion took place, recurring");
#endif
		return p.equals(vp);
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
	switch(m_type)
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
		{
			DisplayObject* ch = getCharacter();
			if ( ! ch ) return "movieclip"; // dangling
			if ( ch->to_movie() ) return "movieclip"; // bound to movieclip
			return "object"; // bound to some other DisplayObject
		}

		case as_value::NULLTYPE:
			return "null";

		case as_value::AS_FUNCTION:
			if ( getFun()->isSuper() ) return "object";
			else return "function";

		default:
			if (is_exception())
			{
				return "exception";
		    }
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
    log_debug("equalsSameType(%s, %s) called [%d]", *this, v, count++);
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
			return toDisplayObject() == v.toDisplayObject(); 

		case NUMBER:
		{
			double a = getNum();
			double b = v.getNum();

			// Nan != NaN
			//if ( isNaN(a) || isNaN(b) ) return false;

			if ( isNaN(a) && isNaN(b) ) return true;

			// -0.0 == 0.0
			if ( (a == -0 && b == 0) || (a == 0 && b == -0) ) return true;

			return a == b;
		}
		default:
			if (is_exception())
			{
				return false; // Exceptions equal nothing.
		    }

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
as_value::toDebugString() const
{
    boost::format ret;

	switch (m_type)
	{
		case UNDEFINED:
			return "[undefined]";
		case NULLTYPE:
			return "[null]";
		case BOOLEAN:
		    ret = boost::format("[bool:%s]") % (getBool() ? "true" : "false");
            return ret.str();
		case OBJECT:
		{
			as_object* obj = getObj().get();
			ret = boost::format("[object(%s):%p]") % typeName(*obj) % static_cast<void*>(obj);
			return ret.str();
		}
		case AS_FUNCTION:
		{
			as_function* obj = getFun().get();
			ret = boost::format("[function(%s):%p]") % typeName(*obj) % static_cast<void*>(obj);
			return ret.str();
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
			const CharacterProxy& sp = getCharacterProxy();
			if ( sp.isDangling() )
			{
				DisplayObject* rebound = sp.get();
				if ( rebound )
				{
				    ret = boost::format("[rebound %s(%s):%p]") % 
						typeName(*rebound) % sp.getTarget() %
						static_cast<void*>(rebound);
				}
				else
				{
				    ret = boost::format("[dangling DisplayObject:%s]") % 
					    sp.getTarget();
				}
			}
			else
			{
				DisplayObject* ch = sp.get();
				ret = boost::format("[%s(%s):%p]") % typeName(*ch) %
				                sp.getTarget() % static_cast<void*>(ch);
			}
			return ret.str();
		}
		default:
			if (is_exception())
			{
				return "[exception]";
			}
			abort();
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
	m_type(UNDEFINED)
{
	set_as_object(obj);
}


/// Examples:
//
/// e.g. for 9*.1234567890123456789:
/// 9999.12345678901
/// 99999.123456789
/// 999999.123456789
/// 9999999.12345679
/// [...]
/// 999999999999.123
/// 9999999999999.12
/// 99999999999999.1
/// 999999999999999
/// 1e+16
/// 1e+17
//
/// For 1*.111111111111111111111111111111111111:
/// 1111111111111.11
/// 11111111111111.1
/// 111111111111111
/// 1.11111111111111e+15
/// 1.11111111111111e+16
//
/// For 1.234567890123456789 * 10^-i:
/// 1.23456789012346
/// 0.123456789012346
/// 0.0123456789012346
/// 0.00123456789012346
/// 0.000123456789012346
/// 0.0000123456789012346
/// 0.00000123456789012346
/// 1.23456789012346e-6
/// 1.23456789012346e-7
std::string
as_value::doubleToString(double val, int radix)
{
	// Handle non-numeric values.
	if (isNaN(val)) return "NaN";
	
    if (isInf(val)) return val < 0 ? "-Infinity" : "Infinity";

    if (val == 0.0 || val == -0.0) return "0"; 

    std::ostringstream ostr;

	if (radix == 10)
	{
		// ActionScript always expects dot as decimal point.
		ostr.imbue(std::locale::classic()); 
		
		// force to decimal notation for this range (because the
        // reference player does)
		if (std::abs(val) < 0.0001 && std::abs(val) >= 0.00001)
		{
			// All nineteen digits (4 zeros + up to 15 significant digits)
			ostr << std::fixed << std::setprecision(19) << val;
			
            std::string str = ostr.str();
			
			// Because 'fixed' also adds trailing zeros, remove them.
			std::string::size_type pos = str.find_last_not_of('0');
			if (pos != std::string::npos) {
				str.erase(pos + 1);
			}
            return str;
		}

        ostr << std::setprecision(15) << val;
        
        std::string str = ostr.str();
        
        // Remove a leading zero from 2-digit exponent if any
        std::string::size_type pos = str.find("e", 0);

        if (pos != std::string::npos && str.at(pos + 2) == '0') {
            str.erase(pos + 2, 1);
        }

        return str;
	}

    // Radix isn't 10
	bool negative = (val < 0);
	if (negative) val = -val;

	double left = std::floor(val);
	if (left < 1) return "0";

    std::string str;
    const std::string digits = "0123456789abcdefghijklmnopqrstuvwxyz";

    // Construct the string backwards for speed, then reverse.
    while (left)
	{
		double n = left;
		left = std::floor(left / radix);
		n -= (left * radix);
		str.push_back(digits[static_cast<int>(n)]);
	}
	if (negative) str.push_back('-'); 

    std::reverse(str.begin(), str.end());

	return str;
	
}

void
as_value::setReachable() const
{
#ifdef GNASH_USE_GC
	switch (m_type)
	{
		case OBJECT:
		{
			as_value::AsObjPtr op = getObj();
			if (op)
				op->setReachable();
			break;
		}
		case AS_FUNCTION:
		{
			as_value::AsFunPtr fp = getFun();
			if (fp)
				fp->setReachable();
			break;
		}
		case MOVIECLIP:
		{
			CharacterProxy sp = getCharacterProxy();
			sp.setReachable();
			break;
		}
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

CharacterProxy
as_value::getCharacterProxy() const
{
	assert(m_type == MOVIECLIP);
	return boost::get<CharacterProxy>(_value);
}

as_value::CharacterPtr
as_value::getCharacter(bool allowUnloaded) const
{
	return getCharacterProxy().get(allowUnloaded);
}

as_value::SpritePtr
as_value::getSprite(bool allowUnloaded) const
{
	assert(m_type == MOVIECLIP);
	DisplayObject* ch = getCharacter(allowUnloaded);
	if ( ! ch ) return 0;
	return ch->to_movie();
}

void
as_value::set_string(const std::string& str)
{
	m_type = STRING;
	_value = str;
}

void
as_value::set_double(double val)
{
	m_type = NUMBER;
	_value = val;
}

void
as_value::set_bool(bool val)
{
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

as_value::as_value(double num)
	:
	m_type(NUMBER),
	_value(num)
{
}

as_value::as_value(as_object* obj)
	:
	m_type(UNDEFINED)
{
	set_as_object(obj);
}


/// Chad: Document this
as_value::as_value(asNamespace &)
{
}

/// Instantiate this value from an AMF element 
as_value::as_value(const amf::Element& el)
	:
	m_type(UNDEFINED)
{
//     GNASH_REPORT_FUNCTION;    
//     el.dump();
    
    VM& vm = VM::get();
    string_table& st = vm.getStringTable();

    switch (el.getType()) {
      case amf::Element::NOTYPE:
      {
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
	  log_debug("as_value(Element&) : AMF type NO TYPE!");
#endif
	  break;
      }
      case amf::Element::NULL_AMF0:
      {
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
            log_debug("as_value(Element&) : AMF type NULL");
#endif
            set_null();
            break;
      }
      case amf::Element::UNDEFINED_AMF0:
      {
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
            log_debug("as_value(Element&) : AMF type UNDEFINED");
#endif
            set_undefined();
            break;
      }
      case amf::Element::MOVIECLIP_AMF0:
      {
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
            log_debug("as_value(Element&) : AMF type MOVIECLIP");
#endif
            log_unimpl("MOVIECLIP AMF0 type");
            set_undefined();
            //m_type = MOVIECLIP;
            //_value = el.getData();

            break;
      }
      case amf::Element::NUMBER_AMF0:
      {
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
            log_debug("as_value(Element&) : AMF type NUMBER");
#endif
            double num = el.to_number();
            set_double(num);
            break;
      }
      case amf::Element::BOOLEAN_AMF0:
      {
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
            log_debug("as_value(Element&) : AMF type BOOLEAN");
#endif
            bool flag = el.to_bool();
            set_bool(flag);
            break;
      }

      case amf::Element::STRING_AMF0:
      case amf::Element::LONG_STRING_AMF0:
      {
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
            log_debug("as_value(Element&) : AMF type STRING");
#endif
	    std::string str;
	    // If there is data, convert it to a string for the as_value
	    if (el.getDataSize() != 0) {
		str = el.to_string();
		// Element's store the property name as the name, not as data.
	    } else if (el.getNameSize() != 0) {
		str = el.getName();
	    }
	    
	    set_string(str);
            break;
      }

      case amf::Element::OBJECT_AMF0:
      {

#ifdef GNASH_DEBUG_AMF_DESERIALIZE
          log_debug("as_value(Element&) : AMF type OBJECT");
#endif
	  as_object* obj = new as_object(getObjectInterface());
          if (el.propertySize()) {
              for (size_t i=0; i < el.propertySize(); i++) {
		  const boost::shared_ptr<amf::Element> prop = el.getProperty(i);
		  if (prop == 0) {
		      break;
		  } else {
		      if (prop->getNameSize() == 0) {
			  log_debug("%s:(%d) Property has no name!", __PRETTY_FUNCTION__, __LINE__);
		      } else {
			  obj->set_member(st.find(prop->getName()), as_value(*prop));
		      }
		  }
              }
          }
	  set_as_object(obj);
          break;
      }

      case amf::Element::ECMA_ARRAY_AMF0:
      {
          // TODO: fixme: ECMA_ARRAY has an additional fiedl, dunno
          //              if accessible trought Element class
          //              (the theoretic number of elements in it)

#ifdef GNASH_DEBUG_AMF_DESERIALIZE
          log_debug("as_value(Element&) : AMF type ECMA_ARRAY");
#endif
          Array_as* obj = new Array_as;
          if (el.propertySize()) {
              for (size_t i=0; i < el.propertySize(); i++) {
		  const boost::shared_ptr<amf::Element> prop = el.getProperty(i);
		  if (prop == 0) {
		      break;
		  } else {
		      obj->set_member(st.find(prop->getName()), as_value(*prop));
		  }
              }
          }
          set_as_object(obj);
          break;
      }
    

      case amf::Element::STRICT_ARRAY_AMF0:
      {
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
          log_debug("as_value(Element&) : AMF type STRICT_ARRAY");
#endif
          Array_as* obj = new Array_as;
          size_t len = el.propertySize();
          obj->resize(len);

          for (size_t i=0; i < el.propertySize(); i++) {
              const boost::shared_ptr<amf::Element> prop = el.getProperty(i);
              if (prop == 0) {
                  break;
              } else {
		  if (prop->getNameSize() == 0) {
		      log_debug("%s:(%d) Property has no name!", __PRETTY_FUNCTION__, __LINE__);
		  } else {
		      obj->set_member(st.find(prop->getName()), as_value(*prop));
		  }
              }
          }
          
          set_as_object(obj);
          break;
      }

      case amf::Element::REFERENCE_AMF0:
      {
        log_unimpl("REFERENCE Element to as_value");
        break;
      }

      case amf::Element::DATE_AMF0:
      {
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
	  log_debug("as_value(Element&) : AMF type DATE");
#endif
	  double num = el.to_number();
	  set_double(num);
	  break;
      }
      //if (swfVersion > 5) m_type = STRING;
      
      case amf::Element::UNSUPPORTED_AMF0:
      {
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
	  log_debug("as_value(Element&) : AMF type UNSUPPORTED");
#endif
	  set_unsupported();
	  break;
      }
      case amf::Element::RECORD_SET_AMF0:
          log_unimpl("Record Set data type is not supported yet");
          break;
      case amf::Element::XML_OBJECT_AMF0:
          log_unimpl("XML data type is not supported yet");
          break;
      case amf::Element::TYPED_OBJECT_AMF0:
          log_unimpl("Typed Object data type is not supported yet");
          break;
      case amf::Element::AMF3_DATA:
          log_unimpl("AMF3 data type is not supported yet");
          break;
      default:
          log_unimpl("Element to as_value - unsupported Element type %d", 
                  el.getType());
          break;
    }
}

as_value&
as_value::newAdd(const as_value& op2)
{
	as_value v2=op2;

	try { convert_to_primitive(); }
	catch (ActionTypeError& e)
	{
		log_debug("%s.to_primitive() threw an error during as_value operator+",
			*this);
	}

	try { v2 = v2.to_primitive(); }
	catch (ActionTypeError& e)
	{
		log_debug("%s.to_primitive() threw an error during as_value operator+",
			op2);
	}

#if GNASH_DEBUG
	log_debug(_("(%s + %s) [primitive conversion done]"),
			*this,
			v2);
#endif

	if (is_string() || v2.is_string() )
	{
		// use string semantic

		int version = VM::get().getSWFVersion();
		convert_to_string_versioned(version);
		string_concat(v2.to_string_versioned(version));
	}
	else
	{
		// use numeric semantic
		double v2num = v2.to_number();
		//log_debug(_("v2 num = %g"), v2num);
		double v1num = to_number();
		//log_debug(_("v1 num = %g"), v1num);

		set_double(v2num + v1num); 

	}

	return *this;
}

as_value
as_value::newLessThan(const as_value& op2_in) const
{
    const as_value& op1_in = *this;

    as_value operand1;
    as_value operand2;

    try { operand1 = op1_in.to_primitive(); }
    catch (ActionTypeError& e)
    {
        log_debug("%s.to_primitive() threw an error during ActionNewLessThen",
            op1_in);
    }

    try { operand2 = op2_in.to_primitive(); }
    catch (ActionTypeError& e)
    {
        log_debug("%s.to_primitive() threw an error during ActionNewLessThen",
            op2_in);
    }

    as_value ret;

    if ( operand1.is_string() && operand2.is_string() )
    {
        ret.set_bool(operand1.to_string() < operand2.to_string());
    }
    else
    {
        const double op1 = operand1.to_number();
        const double op2 = operand2.to_number();

        if ( isNaN(op1) || isNaN(op2) )
        {
            ret.set_undefined();
        }
        else
        {
            ret.set_bool(op1 < op2);
        }
    }
    return ret;
}

as_value&
as_value::subtract(const as_value& op2)
{
	double operand1 = to_number();
	double operand2 = op2.to_number();
	set_double(operand1 - operand2);
	return *this;
}


static boost::uint16_t
readNetworkShort(const boost::uint8_t* buf) {
	boost::uint16_t s = buf[0] << 8 | buf[1];
	return s;
}

static boost::uint32_t
readNetworkLong(const boost::uint8_t* buf) {
	boost::uint32_t s = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
	return s;
}

// Pass pointer to buffer and pointer to end of buffer. Buffer is raw AMF
// encoded data. Must start with a type byte unless third parameter is set.
//
// On success, sets the given as_value and returns true.
// On error (premature end of buffer, etc.) returns false and leaves the given
// as_value untouched.
//
// IF you pass a fourth parameter, it WILL NOT READ A TYPE BYTE, but use what
// you passed instead.
//
// The l-value you pass as the first parameter (buffer start) is updated to
// point just past the last byte parsed
//
// TODO restore first parameter on parse errors
//
static bool
amf0_read_value(const boost::uint8_t *&b, const boost::uint8_t *end, 
        as_value& ret, int inType, std::vector<as_object*>& objRefs, VM& vm)
{
	int amf_type;

	if (b > end) {
		return false;
	}
	
	if (inType != -1) {
		amf_type = inType;
	}
	else {
		if (b < end) {
			amf_type = *b; b += 1;
		} else {
			return false;
		}
	}

	switch (amf_type)
    {

		case amf::Element::BOOLEAN_AMF0:
		{
			bool val = *b; b += 1;
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
			log_debug("amf0 read bool: %d", val);
#endif
			ret.set_bool(val);
			return true;
		}

		case amf::Element::NUMBER_AMF0:
        {
			if (b + 8 > end) {
				log_error(_("AMF0 read: premature end of input reading Number type"));
				return false;
			}
			double dub;
            // TODO: may we avoid a copy and swapBytes call
            //       by bitshifting b[0] trough b[7] ?
            std::copy(b, b+8, (char*)&dub); b+=8; 
			amf::swapBytes(&dub, 8);
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
			log_debug("amf0 read double: %e", dub);
#endif
			ret.set_double(dub);
			return true;
        }

		case amf::Element::STRING_AMF0:
        {
			if (b + 2 > end) {
				log_error(_("AMF0 read: premature end of input reading String "
                            " type"));
				return false;
			}
            boost::uint16_t si = readNetworkShort(b); b += 2;
			if (b + si > end) {
				log_error(_("AMF0 read: premature end of input reading String "
                            "type"));
				return false;
			}

			{
				std::string str(reinterpret_cast<const char*>(b), si); b += si;
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
				log_debug("amf0 read string: %s", str);
#endif
				ret.set_string(str);
				return true;

			}
			break;
        }

		case amf::Element::LONG_STRING_AMF0:
        {
			if (b + 4 > end) {
				log_error(_("AMF0 read: premature end of input "
                            "reading Long String type"));
				return false;
			}
            boost::uint32_t si = readNetworkLong(b); b += 4;
			if (b + si > end) {
				log_error(_("AMF0 read: premature end of input "
                            "reading Long String type"));
				return false;
			}

			{
				std::string str(reinterpret_cast<const char*>(b), si); b += si;
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
				log_debug("amf0 read long string: %s", str);
#endif
				ret.set_string(str);
				return true;

			}
			break;
        }

		case amf::Element::STRICT_ARRAY_AMF0:
        {
				boost::intrusive_ptr<Array_as> array(new Array_as());
                objRefs.push_back(array.get());

                boost::uint32_t li = readNetworkLong(b); b += 4;
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
				log_debug("amf0 starting read of STRICT_ARRAY with %i elements", li);
#endif
				as_value arrayElement;
				for(size_t i = 0; i < li; ++i)
				{
					if ( ! amf0_read_value(b, end, arrayElement, -1,
                                objRefs, vm) )
					{
						return false;
					}
					array->push(arrayElement);
				}

				ret.set_as_object(array);
				return true;
        }

		case amf::Element::ECMA_ARRAY_AMF0:
        {
				Array_as* obj = new Array_as(); // GC-managed...
                objRefs.push_back(obj);

                // set the value immediately, so if there's any problem parsing
                // (like premature end of buffer) we still get something.
				ret.set_as_object(obj);

                boost::uint32_t li = readNetworkLong(b); b += 4;
                
                log_debug("array size: %d", li);
                
                // the count specifies array size, so to have that even if none of the members are indexed
                // if short, will be incremented everytime an indexed member is found
                obj->resize(li);

                // TODO: do boundary checking (if b >= end...)

#ifdef GNASH_DEBUG_AMF_DESERIALIZE
				log_debug("amf0 starting read of ECMA_ARRAY with %i elements", li);
#endif
				as_value objectElement;
				string_table& st = vm.getStringTable();
				for (;;)
				{
                    if ( b+2 >= end )
                    {
                        log_error("MALFORMED SOL: premature end of ECMA_ARRAY "
                                "block");
                        break;
                    }
					boost::uint16_t strlen = readNetworkShort(b); b+=2; 

                    // end of ECMA_ARRAY is signalled by an empty string
                    // followed by an OBJECT_END_AMF0 (0x09) byte
                    if ( ! strlen )
                    {
                        // expect an object terminator here
                        if ( *b++ != amf::Element::OBJECT_END_AMF0 )
                        {
                            log_error("MALFORMED SOL: empty member name not "
                                    "followed by OBJECT_END_AMF0 byte");
                        }
                        break;
                    }

					std::string name(reinterpret_cast<const char*>(b), strlen);

#ifdef GNASH_DEBUG_AMF_DESERIALIZE
					log_debug("amf0 ECMA_ARRAY prop name is %s", name);
#endif
					b += strlen;
					if ( ! amf0_read_value(b, end, objectElement, -1, objRefs, vm) )
					{
						return false;
					}
					obj->set_member(st.find(name), objectElement);
				}

				return true;
        }

		case amf::Element::OBJECT_AMF0:
        {
                string_table& st = vm.getStringTable();

				as_object* obj = new as_object(getObjectInterface()); // GC-managed

                // set the value immediately, so if there's any problem parsing
                // (like premature end of buffer) we still get something.
				ret.set_as_object(obj);

#ifdef GNASH_DEBUG_AMF_DESERIALIZE
				log_debug("amf0 starting read of OBJECT");
#endif
                objRefs.push_back(obj);

				as_value tmp;
				std::string keyString;
				for(;;)
				{
					if ( ! amf0_read_value(b, end, tmp, 
                                amf::Element::STRING_AMF0, objRefs, vm) )
					{
						return false;
					}
					keyString = tmp.to_string();

					if ( keyString.empty() )
					{
						if (b < end) {
							b += 1; // AMF0 has a redundant "object end" byte
						} else {
							log_error("AMF buffer terminated just before "
                                    "object end byte. continuing anyway.");
						}
						return true;
					}

					if ( ! amf0_read_value(b, end, tmp, -1, objRefs, vm) )
					{
						return false;
					}
					obj->set_member(st.find(keyString), tmp);
				}
        }

		case amf::Element::UNDEFINED_AMF0:
        {
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
				log_debug("readAMF0: undefined value");
#endif
				ret.set_undefined();
				return true;
        }

		case amf::Element::NULL_AMF0:
        {
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
				log_debug("readAMF0: null value");
#endif
				ret.set_null();
				return true;
        }

		case amf::Element::REFERENCE_AMF0:
        {
            boost::uint16_t si = readNetworkShort(b); b += 2;
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
				log_debug("readAMF0: reference #%d", si);
#endif
                if ( si < 1 || si > objRefs.size() )
                {
                    log_error("readAMF0: invalid reference to object %d (%d known objects)", si, objRefs.size());
                    return false;
                }
                ret.set_as_object(objRefs[si-1]);
                return true;
        }

		case amf::Element::DATE_AMF0:
        {
			if (b + 8 > end) {
				log_error(_("AMF0 read: premature end of input reading Date "
                            "type"));
				return false;
			}
			double dub;
            // TODO: may we avoid a copy and swapBytes call
            //       by bitshifting b[0] trough b[7] ?
            std::copy(b, b+8, (char*)&dub); b+=8; 
			amf::swapBytes(&dub, 8);
#ifdef GNASH_DEBUG_AMF_DESERIALIZE
			log_debug("amf0 read date: %e", dub);
#endif
            as_object* obj = new Date_as(dub);
			ret.set_as_object(obj);

			if (b + 2 > end) {
				log_error(_("AMF0 read: premature end of input reading "
                            "timezone from Date type"));
				return false;
			}
            LOG_ONCE(log_unimpl("Timezone info from AMF0 encoded Date object "
                        "ignored"));
            b+=2;

			return true;
        }

		// TODO define other types (function, sprite, etc)
		default:
        {
			log_unimpl("AMF0 to as_value: unsupported type: %i", amf_type);
			return false;
        }
	}

	// this function was called with a zero-length buffer
	return false;
}

bool
as_value::readAMF0(const boost::uint8_t *&b, const boost::uint8_t *end,
        int inType, std::vector<as_object*>& objRefs, VM& vm)
{
	return amf0_read_value(b, end, *this, inType, objRefs, vm);
}

bool
as_value::writeAMF0(SimpleBuffer& buf, 
        std::map<as_object*, size_t>& offsetTable, VM& vm,
        bool allowStrict) const
{
    typedef std::map<as_object*, size_t> OffsetTable;

    assert (!is_exception());

    switch (m_type)
    {
        default:
            log_unimpl(_("serialization of as_value of type %d"), m_type);
            return false;

        case AS_FUNCTION:
            log_unimpl(_("serialization of as_value of type FUNCTION"), m_type);
            return false;

        case OBJECT:
        {
            as_object* obj = to_object(*vm.getGlobal()).get();
            assert(obj);
            OffsetTable::iterator it = offsetTable.find(obj);
            if ( it == offsetTable.end() )
            {
                size_t idx = offsetTable.size()+1; // 1 for the first, etc...
                offsetTable[obj] = idx;

                Array_as* ary = dynamic_cast<Array_as*>(obj);
                if ( ary )
                {
                    size_t len = ary->size();
                    if ( allowStrict && ary->isStrict() )
                    {
#ifdef GNASH_DEBUG_AMF_SERIALIZE
                        log_debug(_("writeAMF0: serializing array of %d "
                                    "elements as STRICT_ARRAY (index %d)"),
                                    len, idx);
#endif
                        buf.appendByte(amf::Element::STRICT_ARRAY_AMF0);
                        buf.appendNetworkLong(len);

                        as_value elem;
                        for (size_t i=0; i<len; ++i)
                        {
                            elem = ary->at(i);
                            if ( ! elem.writeAMF0(buf, offsetTable, vm, allowStrict) )
                            {
                                log_error("Problems serializing strict array member %d=%s", i, elem);
                                return false;
                            }
                        }
                        return true;
                    }
                    else 
                    {
#ifdef GNASH_DEBUG_AMF_SERIALIZE
                        log_debug(_("writeAMF0: serializing array of %d "
                                    "elements as ECMA_ARRAY (index %d) [allowStrict:%d, isStrict:%d]"),
                                    len, idx, allowStrict, ary->isStrict());
#endif
                        buf.appendByte(amf::Element::ECMA_ARRAY_AMF0);
                        buf.appendNetworkLong(len);
                    }
                }
                else if ( obj->isDateObject() )
                {
                    const Date_as& date = dynamic_cast<const Date_as&>(*obj);
                    double d = date.getTimeValue(); 
#ifdef GNASH_DEBUG_AMF_SERIALIZE
                    log_debug(_("writeAMF0: serializing date object "
                                "with index %d and value %g"), idx, d);
#endif
                    buf.appendByte(amf::Element::DATE_AMF0);

                    // This actually only swaps on little-endian machines
                    amf::swapBytes(&d, 8);
                    buf.append(&d, 8);

                    // This should be timezone
                    boost::uint16_t tz=0; 
                    buf.appendNetworkShort(tz);

                    return true;
                }
                else
                {
#ifdef GNASH_DEBUG_AMF_SERIALIZE
                    log_debug(_("writeAMF0: serializing object (or function) "
                                "with index %d"), idx);
#endif
                    buf.appendByte(amf::Element::OBJECT_AMF0);
                }

                PropsBufSerializer props(buf, vm, offsetTable, allowStrict);
                obj->visitNonHiddenPropertyValues(props);
                if ( ! props.success() ) 
                {
                    log_error("Could not serialize object");
                    return false;
                }
                buf.appendNetworkShort(0);
                buf.appendByte(amf::Element::OBJECT_END_AMF0);
                return true;
            }
            else // object already seen
            {
                size_t idx = it->second;
#ifdef GNASH_DEBUG_AMF_SERIALIZE
                log_debug(_("writeAMF0: serializing object (or function) "
                            "as reference to %d"), idx);
#endif
                buf.appendByte(amf::Element::REFERENCE_AMF0);
                buf.appendNetworkShort(idx);
                return true;
            }
            return true;
        }

        case STRING:
        {
            const std::string& str = getStr();
            size_t strlen = str.size();
            if ( strlen <= 65535 )
            {
#ifdef GNASH_DEBUG_AMF_SERIALIZE
                log_debug(_("writeAMF0: serializing string '%s'"), str);
#endif
                buf.appendByte(amf::Element::STRING_AMF0);
                buf.appendNetworkShort(strlen);
                buf.append(str.c_str(), strlen);
            }
            else
            {
#ifdef GNASH_DEBUG_AMF_SERIALIZE
                log_debug(_("writeAMF0: serializing long string '%s'"), str);
#endif
                buf.appendByte(amf::Element::LONG_STRING_AMF0);
                buf.appendNetworkLong(strlen);
                buf.append(str.c_str(), strlen);
            }
            return true;
        }

        case NUMBER:
        {
            double d = getNum();
#ifdef GNASH_DEBUG_AMF_SERIALIZE
            log_debug(_("writeAMF0: serializing number '%g'"), d);
#endif
            buf.appendByte(amf::Element::NUMBER_AMF0);
            amf::swapBytes(&d, 8); // this actually only swaps on little-endian machines
            buf.append(&d, 8);
            return true;
        }

        case MOVIECLIP:
        {
#ifdef GNASH_DEBUG_AMF_SERIALIZE
            log_debug(_("writeAMF0: serializing MOVIECLIP (as undefined)"));
#endif
            // See misc-ming.all/SharedObjectTest.as
            buf.appendByte(amf::Element::UNDEFINED_AMF0);
            return true;
        }

        case NULLTYPE:
        {
#ifdef GNASH_DEBUG_AMF_SERIALIZE
            log_debug(_("writeAMF0: serializing null"));
#endif
            buf.appendByte(amf::Element::NULL_AMF0);
            return true;
        }

        case UNDEFINED:
        {
#ifdef GNASH_DEBUG_AMF_SERIALIZE
            log_debug(_("writeAMF0: serializing undefined"));
#endif
            buf.appendByte(amf::Element::UNDEFINED_AMF0);
            return true;
        }

        case BOOLEAN:
        {
            bool tf = getBool();
#ifdef GNASH_DEBUG_AMF_SERIALIZE
            log_debug(_("writeAMF0: serializing boolean '%s'"), tf);
#endif

            buf.appendByte(amf::Element::BOOLEAN_AMF0);
            if (tf) buf.appendByte(1);
            else buf.appendByte(0);

            return true;
        }
    }
}

} // namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
