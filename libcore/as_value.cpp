// as_value.cpp:  ActionScript values, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#include "as_value.h"

#include <boost/shared_ptr.hpp>
#include <cmath> 
#include <cctype> 
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <locale>
#include <sstream>
#include <iomanip>
#include <string>
#include <algorithm>

#include "as_object.h"
#include "as_function.h" // for as_function
#include "MovieClip.h" // for DISPLAYOBJECT values
#include "DisplayObject.h" // for DISPLAYOBJECT values
#include "as_environment.h" // for DISPLAYOBJECT values
#include "VM.h" // for DISPLAYOBJECT values
#include "movie_root.h" // for DISPLAYOBJECT values
#include "utility.h" // for typeName() 
#include "GnashNumeric.h"
#include "namedStrings.h"
#include "GnashException.h"
#include "Array_as.h"
#include "Date_as.h" // for Date type (readAMF0)
#include "SimpleBuffer.h"
#include "StringPredicates.h"
#include "Global_as.h"
#include "String_as.h"
#include "AMFConverter.h"

// Define the macro below to make abstract equality operator verbose
//#define GNASH_DEBUG_EQUALITY 1

// Define the macro below to make to_primitive verbose
//#define GNASH_DEBUG_CONVERSION_TO_PRIMITIVE 1

// Define this macro to make soft references activity verbose
#define GNASH_DEBUG_SOFT_REFERENCES

namespace gnash {

namespace {
    bool objectEqualsPrimitive(const as_value& obj, const as_value& prim,
            int version);
    bool stringEqualsNumber(const as_value& str, const as_value& num,
            int version);
    bool compareBoolean(const as_value& boolean, const as_value& other,
            int version);
    inline bool findMethod(as_object& obj, const ObjectURI& m, as_value& ret);
    template<typename T> as_object* constructObject(VM& vm, const T& arg,
            const ObjectURI& className);
}

namespace {

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

    // If the conversion fails, or if the whole string must be convertible and
    // some DisplayObjects are left, throw an exception.
    if (!(is >> target) || (whole && is.get(c))) {
        throw boost::bad_lexical_cast();
    }

    return target;
}

struct
NonNumericChar
{
   bool operator()(char c) {
       return (!std::isdigit(c) && c != '.' && c != '-' && c != '+');
   }
};

/// Omit an empty exponent that is valid in ActionScript but not in C++.
//
/// This function throws a boost::bad_lexical_cast if it finds an invalid
/// exponent to avoid attempting an extraction when it will definitely fail.
//
/// A successful return from this function does not mean the exponent is
/// valid, only that the result of stringstream's conversion will mirror
/// AS behaviour.
//
/// @param si       An iterator pointing to the position after an exponent sign.
/// @param last     The end of the string to extract. If we have an exponent
///                 with no following digit, this iterator is moved to
///                 a position before the exponent sign.
void
validateExponent(std::string::const_iterator si,
        std::string::const_iterator& last)
{

    // Check for exponent with no following character. Depending on the
    // version of gcc, extraction may be rejected (probably more correct) or
    // accepted as a valid exponent (what ActionScript wants).
    // In this case we remove the exponent to get the correct behaviour
    // on all compilers.
    if (si == last) {
        --last;
        return;
    }

    // Exponents with a following '-' or '+' are also valid if they end the
    // string. It's unlikely that any version of gcc allowed this.
    if (*si == '-' || *si == '+') {
        ++si;
        if (si == last) {
            last -= 2;
            return;
        }
    }

    // An exponent ("e", "e-", or "e+") followed by a non digit is invalid.
    if (!std::isdigit(*si)) {
        throw boost::bad_lexical_cast();
    }

}

/// Convert a string to a double if the complete string can be converted.
//
/// This follows the conditions of the standard C locale for numbers except
/// that an exponent signifier with no following digit (e.g. "2e") is
/// considered valid. Moreover, "2e-" is also considered valid.
//
/// This function scans the string twice (once for verification, once for
/// extraction) and copies it once (for extraction).
double
parseDecimalNumber(std::string::const_iterator start,
        std::string::const_iterator last)
{
    assert(start != last);
 
    // Find the first position that is not a numeric character ('e' or 'E' not
    // included). Even if no invalid character is found, it does not mean
    // that the number is valid ("++++---" would pass the test).
    std::string::const_iterator si =
        std::find_if(start, last, NonNumericChar());

    if (si != last) {
        // If this character is not an exponent sign, the number is malformed.
        if (*si != 'e' && *si != 'E') throw boost::bad_lexical_cast(); 
        /// Move the last iterator to point before empty exponents.
        else validateExponent(si + 1, last);
    }

    return boost::lexical_cast<double>(std::string(start, last));
}

} // anonymous namespace

// Conversion to const std::string&.
std::string
as_value::to_string(int version) const
{
    switch (_type)
    {
        case STRING:
            return getStr();
        case DISPLAYOBJECT:
        {
            const CharacterProxy& sp = getCharacterProxy();
            if (!sp.get()) return "";
            return sp.getTarget();
        }
        case NUMBER:
            return doubleToString(getNum());
        case UNDEFINED: 
            if (version <= 6) return "";
            return "undefined";
        case NULLTYPE:
            return "null";
        case BOOLEAN:
            return getBool() ? "true" : "false";
        case OBJECT:
        {
            as_object* obj = getObj();
            String_as* s;
            if (isNativeType(obj, s)) return s->value();

            try {
                as_value ret = to_primitive(STRING);
                // This additional is_string test is NOT compliant with ECMA-262
                // specification, but seems required for compatibility with the
                // reference player.
                if (ret.is_string()) return ret.getStr();
            }
            catch (const ActionTypeError& e) {}
           
            return is_function() ? "[type Function]" : "[type Object]";

        }

        default:
            return "[exception]";
    }
    
}

as_value::AsType
as_value::defaultPrimitive(int version) const
{
    if (_type == OBJECT && version > 5) {
        Date_as* d;
        if (isNativeType(getObj(), d)) return STRING;
    }
    return NUMBER;
}

// Conversion to primitive value.
as_value
as_value::to_primitive(AsType hint) const
{
    if (_type != OBJECT) return *this; 

#if GNASH_DEBUG_CONVERSION_TO_PRIMITIVE
    log_debug("to_primitive(%s)", hint==NUMBER ? "NUMBER" : "STRING");
#endif 

    // TODO: implement as_object::DefaultValue (ECMA-262 - 8.6.2.6)

    as_value method;
    as_object* obj(0);

    if (hint == NUMBER) {
        assert(_type == OBJECT);
        obj = getObj();

        if (!findMethod(*obj, NSV::PROP_VALUE_OF, method)) {
            // Returning undefined here instead of throwing
            // a TypeError passes tests in actionscript.all/Object.as
            // and many swfdec tests, with no new failures (though
            // perhaps we aren't testing enough).
            return as_value();
        }
    }
    else {
        assert(hint == STRING);
        assert(_type == OBJECT);
        obj = getObj();

        // @@ Moock says, "the value that results from
        // calling toString() on the object".
        if (!findMethod(*obj, NSV::PROP_TO_STRING, method) &&
                !findMethod(*obj, NSV::PROP_VALUE_OF, method)) {
                throw ActionTypeError();
        }
    }

    assert(obj);

    as_environment env(getVM(*obj));
    fn_call::Args args;
    as_value ret = invoke(method, env, obj, args);

#if GNASH_DEBUG_CONVERSION_TO_PRIMITIVE
    log_debug("to_primitive: method call returned %s", ret);
#endif

    if (ret._type == OBJECT) {
        throw ActionTypeError();
    }
    return ret;
}

double
as_value::to_number(const int version) const
{

    switch (_type) {
        case STRING:
        {
            const std::string& s = getStr();
            if ( s.empty() ) {
                return version >= 5 ? NaN : 0.0;
            }
            
            if (version <= 4)
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

                if (version > 5) {
                    double d;
                    // Will throw if invalid.
                    if (parseNonDecimalInt(s, d)) return d;
                }

                // @@ Moock says the rule here is: if the
                // string is a valid float literal, then it
                // gets converted; otherwise it is set to NaN.
                // Valid for SWF5 and above.
                const std::string::size_type pos =
                    s.find_first_not_of(" \r\n\t");

                if (pos == std::string::npos) return NaN;
                
                // Will throw a boost::bad_lexical_cast if it fails.
                return parseDecimalNumber(s.begin() + pos, s.end());
 
            }
            catch (const boost::bad_lexical_cast&) {
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
            return (version >= 7 ? NaN : 0);
        }

        case BOOLEAN: 
            return getBool() ? 1 : 0;

        case NUMBER:
            return getNum();

        case OBJECT:
        {
            // @@ Moock says the result here should be
            // "the return value of the object's valueOf()
            // method".
            //
            // Arrays and Movieclips should return NaN.
            try {
                as_value ret = to_primitive(NUMBER);
                return ret.to_number(version);
            }
            catch (const ActionTypeError& e) {
#if GNASH_DEBUG_CONVERSION_TO_PRIMITIVE
                log_debug("to_primitive(%s, NUMBER) threw an "
                            "ActionTypeError %s", *this, e.what());
#endif
                if (is_function() && version < 6) {
                    return 0;
                }
                
                return NaN;
            }
        }

        case DISPLAYOBJECT:
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

// Conversion to boolean 
bool
as_value::to_bool(const int version) const
{
    switch (_type)
    {
        case STRING:
        {
            if (version >= 7) return !getStr().empty();
            const double num = to_number(version);
            return num && !isNaN(num);
        }
        case NUMBER:
        {
            const double d = getNum();
            // see testsuite/swfdec/if-6.swf
            return d && ! isNaN(d);
        }
        case BOOLEAN:
            return getBool();
        case OBJECT:
            return true;
        case DISPLAYOBJECT:
            return true;
        default:
            assert(_type == UNDEFINED || _type == NULLTYPE || is_exception());
            return false;
    }
}

// Return value as an object.
as_object*
as_value::to_object(VM& vm) const
{

    switch (_type)
    {
        case OBJECT:
            return getObj();

        case DISPLAYOBJECT:
            return getObject(toDisplayObject());

        case STRING:
            return constructObject(vm, getStr(), NSV::CLASS_STRING);

        case NUMBER:
            return constructObject(vm, getNum(), NSV::CLASS_NUMBER);

        case BOOLEAN:
            return constructObject(vm, getBool(), NSV::CLASS_BOOLEAN);

        default:
            // Invalid to convert exceptions.
            return NULL;
    }
}

MovieClip*
as_value::toMovieClip(bool allowUnloaded) const
{
    if (_type != DISPLAYOBJECT) return 0;

    DisplayObject *ch = getCharacter(allowUnloaded);
    if (!ch) return 0;
    return ch->to_movie();
}

DisplayObject*
as_value::toDisplayObject(bool allowUnloaded) const
{
    if (_type != DISPLAYOBJECT) return 0;
    return getCharacter(allowUnloaded);
}

// Return value as an ActionScript function.  Returns NULL if value is
// not an ActionScript function.
as_function*
as_value::to_function() const
{
    if (_type == OBJECT) {
        return getObj()->to_function();
    }

    return 0;
}

as_object*
as_value::get_object() const
{
    if (_type == OBJECT) {
        return getObj();
    }

    return 0;
}

void
as_value::set_undefined()
{
    _type = UNDEFINED;
    _value = boost::blank();
}

void
as_value::set_null()
{
    _type = NULLTYPE;
    _value = boost::blank();
}

void
as_value::set_as_object(as_object* obj)
{
    if (!obj)
    {
        set_null();
        return;
    }
    if (obj->displayObject()) {
        // The static cast is fine as long as the as_object is genuinely
        // a DisplayObject.
        _type = DISPLAYOBJECT;
        _value = CharacterProxy(obj->displayObject(), getRoot(*obj));
        return;
    }

    if (_type != OBJECT || getObj() != obj) {
        _type = OBJECT;
        _value = obj;
    }
}

bool
as_value::equals(const as_value& v, int version) const
{

    // First compare values of the same type.
    if (_type == v._type) return equalsSameType(v);
    
    // Then compare booleans.
    if (is_bool()) return compareBoolean(*this, v, version);
    if (v.is_bool()) return compareBoolean(v, *this, version);

    // Then compare any other primitive, including null and undefined, with
    // an object.
    if (!is_object() && v.is_object()) {
        return objectEqualsPrimitive(v, *this, version);
    }

    if (is_object() && !v.is_object()) {
        return objectEqualsPrimitive(*this, v, version);
    }

    // Remaining null or undefined values only equate to other null or
    // undefined values.
    const bool null = (is_undefined() || is_null());
    const bool v_null = (v.is_undefined() || v.is_null());
    if (null || v_null) return null == v_null;

    // Now compare a number with a string.
    if (is_number() && v.is_string()) {
        return stringEqualsNumber(v, *this, version);
    }
    if (is_string() && v.is_number()) {
        return stringEqualsNumber(*this, v, version);
    }
    
    // Finally compare non-identical objects.
    as_value p = *this;
    as_value vp = v;

    try {
        p = to_primitive(NUMBER); 
    }
    catch (const ActionTypeError& e) {}

    try {
        vp = v.to_primitive(NUMBER); 
    }
    catch (const ActionTypeError& e) {}

    // No conversion took place; the result is false
    if (strictly_equals(p) && v.strictly_equals(vp)) {
        return false;
    }
    
    return p.equals(vp, version);
}
    
const char*
as_value::typeOf() const
{
    switch (_type)
    {
        case UNDEFINED:
            return "undefined"; 

        case STRING:
            return "string";

        case NUMBER:
            return "number";

        case BOOLEAN:
            return "boolean";

        case OBJECT:
            return is_function() ? "function" : "object";

        case DISPLAYOBJECT:
        {
            DisplayObject* ch = getCharacter();
            if ( ! ch ) return "movieclip"; // dangling
            if ( ch->to_movie() ) return "movieclip"; // bound to movieclip
            return "object"; // bound to some other DisplayObject
        }

        case NULLTYPE:
            return "null";

        default:
            if (is_exception()) return "exception";
            std::abort();
            return 0;
    }
}

bool
as_value::equalsSameType(const as_value& v) const
{
    assert(_type == v._type);

    switch (_type)
    {
        case UNDEFINED:
        case NULLTYPE:
            return true;

        case OBJECT:
        case BOOLEAN:
        case STRING:
            return _value == v._value;

        case DISPLAYOBJECT:
            return toDisplayObject() == v.toDisplayObject(); 

        case NUMBER:
        {
            const double a = getNum();
            const double b = v.getNum();
            if (isNaN(a) && isNaN(b)) return true;
            return a == b;
        }
        default:
            if (is_exception()) return false; 

    }
    std::abort();
    return false;
}

bool
as_value::strictly_equals(const as_value& v) const
{
    if ( _type != v._type ) return false;
    return equalsSameType(v);
}

void
as_value::setReachable() const
{
    switch (_type)
    {
        case OBJECT:
        {
            as_object* op = getObj();
            if (op) op->setReachable();
            break;
        }
        case DISPLAYOBJECT:
        {
            CharacterProxy sp = getCharacterProxy();
            sp.setReachable();
            break;
        }
        default: break;
    }
}

as_object*
as_value::getObj() const
{
    assert(_type == OBJECT);
    return boost::get<as_object*>(_value);
}

CharacterProxy
as_value::getCharacterProxy() const
{
    assert(_type == DISPLAYOBJECT);
    return boost::get<CharacterProxy>(_value);
}

DisplayObject*
as_value::getCharacter(bool allowUnloaded) const
{
    return getCharacterProxy().get(allowUnloaded);
}

void
as_value::set_string(const std::string& str)
{
    _type = STRING;
    _value = str;
}

void
as_value::set_double(double val)
{
    _type = NUMBER;
    _value = val;
}

void
as_value::set_bool(bool val)
{
    _type = BOOLEAN;
    _value = val;
}

bool
as_value::is_function() const
{
    return _type == OBJECT && getObj()->to_function();
}

bool
as_value::writeAMF0(amf::Writer& w) const
{

    assert (!is_exception());

    switch (_type)
    {
        default:
            log_unimpl(_("serialization of as_value of type %d"), _type);
            return false;

        case OBJECT:
            if (is_function()) return false;
            return w.writeObject(getObj());

        case STRING:
            return w.writeString(getStr());

        case NUMBER:
            return w.writeNumber(getNum());

        case DISPLAYOBJECT:
        case UNDEFINED:
            return w.writeUndefined();

        case NULLTYPE:
            return w.writeNull();

        case BOOLEAN:
            return w.writeBoolean(getBool());
    }
}

bool
parseNonDecimalInt(const std::string& s, double& d, bool whole)
{
    const std::string::size_type slen = s.length();

    // "0#" would still be octal, but has the same value as a decimal.
    if (slen < 3) return false;

    bool negative = false;

    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
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
            s.find_first_not_of("01234567", 1) == std::string::npos) {

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

std::string
doubleToString(double val, int radix)
{
    // Examples:
    //
    // e.g. for 9*.1234567890123456789:
    // 9999.12345678901
    // 99999.123456789
    // 999999.123456789
    // 9999999.12345679
    // [...]
    // 999999999999.123
    // 9999999999999.12
    // 99999999999999.1
    // 999999999999999
    // 1e+16
    // 1e+17
    //
    // For 1*.111111111111111111111111111111111111:
    // 1111111111111.11
    // 11111111111111.1
    // 111111111111111
    // 1.11111111111111e+15
    // 1.11111111111111e+16
    //
    // For 1.234567890123456789 * 10^-i:
    // 1.23456789012346
    // 0.123456789012346
    // 0.0123456789012346
    // 0.00123456789012346
    // 0.000123456789012346
    // 0.0000123456789012346
    // 0.00000123456789012346
    // 1.23456789012346e-6
    // 1.23456789012346e-7

    // Handle non-numeric values.
    if (isNaN(val)) return "NaN";
    
    if (isInf(val)) return val < 0 ? "-Infinity" : "Infinity";

    if (val == 0.0 || val == -0.0) return "0"; 

    std::ostringstream ostr;

    if (radix == 10) {

        // ActionScript always expects dot as decimal point.
        ostr.imbue(std::locale::classic()); 
        
        // force to decimal notation for this range (because the
        // reference player does)
        if (std::abs(val) < 0.0001 && std::abs(val) >= 0.00001) {

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
    while (left) {
        double n = left;
        left = std::floor(left / radix);
        n -= (left * radix);
        str.push_back(digits[static_cast<int>(n)]);
    }
    if (negative) str.push_back('-'); 

    std::reverse(str.begin(), str.end());

    return str;
}

namespace {

/// Checks for equality between an object value and a primitive value
//
/// @param obj      An as_value of object type. Callers must ensure this
///                 condition is met.
/// @param prim     An as_value of primitive type. Callers must ensure this
///                 condition is met.
//
/// This is a function try-block.
bool
objectEqualsPrimitive(const as_value& obj, const as_value& prim, int version)
try {

    assert(obj.is_object());
    assert(!prim.is_object());

    as_value tmp = obj.to_primitive(as_value::NUMBER);
    if (obj.strictly_equals(tmp)) return false;
    return tmp.equals(prim, version);
}
catch (const ActionTypeError&) {
    return false;
}

/// @param boolean      A boolean as_value
/// @param other        An as_value of any type.
bool
compareBoolean(const as_value& boolean, const as_value& other, int version)
{
    assert(boolean.is_bool());
    return as_value(boolean.to_number(version)).equals(other, version); 
}

bool
stringEqualsNumber(const as_value& str, const as_value& num, int version)
{
    assert(num.is_number());
    assert(str.is_string());
    const double n = str.to_number(version);
    if (!isFinite(n)) return false;
    return num.strictly_equals(n);
}


/// Returns a member only if it is an object.
inline bool
findMethod(as_object& obj, const ObjectURI& m, as_value& ret)
{
    return obj.get_member(m, &ret) && ret.is_object();
}

/// Construct an instance of the specified global class.
//
/// If the class is not present or is not a constructor function, this
/// function throws an ActionTypeError.
//
/// TODO: consider whether ActionTypeError is an appropriate exception.
/// TODO: test the other failure cases.
template<typename T>
as_object*
constructObject(VM& vm, const T& arg, const ObjectURI& className)
{

    as_object& gl = *vm.getGlobal();

    as_value clval;

    // This is tested in actionscript.all/Object.as to return an 
    // undefined value. We throw the exception back to the VM, which pushes
    // an undefined value onto the stack.
    if (!gl.get_member(className, &clval) ) {
        throw ActionTypeError();
    }
    
    // This is not properly tested.
    if (!clval.is_function()) {
        throw ActionTypeError();
    }
    
    as_function* ctor = clval.to_function();

    // This is also not properly tested.
    if (!ctor) throw ActionTypeError();

    fn_call::Args args;
    args += arg;

    as_environment env(vm);
    as_object* ret = constructInstance(*ctor, env, args);

    return ret;

}

} // unnamed namespace

std::ostream&
operator<<(std::ostream& o, const as_value& v)
{

    switch (v._type)
    {
        case as_value::UNDEFINED:
            return o << "[undefined]";
        case as_value::NULLTYPE:
            return o << "[null]";
        case as_value::BOOLEAN:
        {
            return o << "[bool:" << std::boolalpha << v.getBool() << "]";
        }
        case as_value::OBJECT:
        {
            as_object* obj = v.getObj();
            assert(obj);
            const std::string desc = obj->array() ? "array" :
                obj->relay() ? typeName(*obj->relay()) : typeName(*obj);
            return o << "[object(" << desc << "):" << static_cast<void*>(obj)
                                                       << "]";
        }
        case as_value::STRING:
            return o << "[string:" + v.getStr() + "]";
        case as_value::NUMBER:
            return o << "[number:" << v.getNum() << "]";
        case as_value::DISPLAYOBJECT:
        {
            boost::format ret;
            const CharacterProxy& sp = v.getCharacterProxy();
            if (sp.isDangling()) {
                DisplayObject* rebound = sp.get();
                if (rebound) {
                    ret = boost::format("[rebound %s(%s):%p]") % 
                        typeName(*rebound) % sp.getTarget() %
                        static_cast<void*>(rebound);
                }
                else {
                    ret = boost::format("[dangling DisplayObject:%s]") % 
                        sp.getTarget();
                }
            }
            else {
                DisplayObject* ch = sp.get();
                ret = boost::format("[%s(%s):%p]") % typeName(*ch) %
                                sp.getTarget() % static_cast<void*>(ch);
            }
            return o << ret.str();
        }
        default:
            assert(v.is_exception());
            return o << "[exception]";
    }
}


} // namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
