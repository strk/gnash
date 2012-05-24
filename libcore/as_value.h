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

#ifndef GNASH_AS_VALUE_H
#define GNASH_AS_VALUE_H

#include "dsodefs.h"
#include "CharacterProxy.h"

#include <limits>
#include <string>
#include <boost/variant.hpp>
#include <iosfwd> // for inlined output operator
#include <boost/type_traits/is_floating_point.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/cstdint.hpp>

#include "utility.h" // UNUSED

// Forward declarations
namespace gnash {
    class VM;
	class as_object;
	class Global_as;
	class fn_call;
	class as_function;
	class MovieClip;
	class DisplayObject;
    namespace amf {
        class Writer;
    }
}

namespace gnash {


// NaN constant for use in as_value implementation
static const double NaN = std::numeric_limits<double>::quiet_NaN();

// The following template works just like its C counterpart, with added
// type safety (i.e., they will only compile for floating point arguments).
template <typename T>
inline bool
isNaN(const T& num, typename boost::enable_if<boost::is_floating_point<T> >::
        type* dummy = 0)
{
	UNUSED(dummy);
	return num != num;
}

template <typename T>
inline bool
isInf(const T& num)
{
	return isNaN(num - num);
}


/// These are the primitive types, see the ECMAScript reference.
enum primitive_types
{
	PTYPE_STRING,
	PTYPE_NUMBER,
	PTYPE_BOOLEAN
};

/// ActionScript value type.
//
/// The as_value class can store basic ActionScript types.
//
/// These are the primitive types (Number, Boolean, String, null, and
/// undefined), as well as complex types (Object and DisplayObject).
//
/// Most type handling is hidden within the class. There are two different
/// types of access to the as_value: converting and non-converting.
//
/// Non-converting access
/// Non-converting access is available for the complex types, for instance
/// to_function() and toMovieClip(). In these cases, an object pointer is
/// return only if the as_value is currently of the requested type. There
/// are no ActionScript side-effects in such cases.
//
/// Converting access
/// The primitive types and Objects have converting access. This means that
/// as_values of a different type are converted to the requested type. These
/// functions may have ActionScript side-effects, for instance the calling of
/// toString or valueOf, or construction of an object.
//
/// It is possible to check the current type of an as_value using is_string(),
/// is_number() etc. These functions have no ActionScript side effects.
class as_value
{

public:

    // The exception type should always be one greater than the normal type.
    enum AsType
    {
        UNDEFINED,
        UNDEFINED_EXCEPT,
        NULLTYPE,
        NULLTYPE_EXCEPT,
        BOOLEAN,
        BOOLEAN_EXCEPT,
        STRING,
        STRING_EXCEPT,
        NUMBER, 
        NUMBER_EXCEPT,
        OBJECT,
        OBJECT_EXCEPT,
        DISPLAYOBJECT,
        DISPLAYOBJECT_EXCEPT
    };
    
    /// Construct an undefined value
    DSOEXPORT as_value()
        :
        _type(UNDEFINED),
        _value(boost::blank())
    {
    }
    
    /// Copy constructor.
    DSOEXPORT as_value(const as_value& v)
        :
        _type(v._type),
        _value(v._value)
    {
    }

    ~as_value() {}
    
    /// Construct a primitive String value 
    DSOEXPORT as_value(const char* str)
        :
        _type(STRING),
        _value(std::string(str))
    {}

    /// Construct a primitive String value 
    DSOEXPORT as_value(const std::string& str)
        :
        _type(STRING),
        _value(std::string(str))
    {}
    
    /// Construct a primitive Boolean value
    template <typename T>
    as_value(T val, typename boost::enable_if<boost::is_same<bool, T> >::type*
             dummy = 0)
        :
        _type(BOOLEAN),
        _value(val)
	{
        UNUSED(dummy);
	}
    
    /// Construct a primitive Number value
    as_value(double num)
        :
        _type(NUMBER),
        _value(num)
    {}
    
    /// Construct a null, Object, or DisplayObject value
    as_value(as_object* obj)
        :
        _type(UNDEFINED)
    {
        set_as_object(obj);
    }
    
    /// Assign to an as_value.
    DSOEXPORT as_value& operator=(const as_value& v)
    {
        _type = v._type;
        _value = v._value;
        return *this;
    }

    friend std::ostream& operator<<(std::ostream& o, const as_value&);
    
    /// Return the primitive type of this value as a string.
    const char* typeOf() const;
    
    /// Return true if this value is a function
    bool is_function() const;
    
    /// Return true if this value is a string
    bool is_string() const {
        return _type == STRING;
    }
    
    /// Return true if this value is strictly a number
    bool is_number() const {
        return _type == NUMBER;
    }
    
    /// Return true if this value is an object
    //
    /// Both DisplayObjects and Objects count as Objects
    bool is_object() const {
        return _type == OBJECT || _type == DISPLAYOBJECT;
    }
    
    /// Return true if this value is a DISPLAYOBJECT 
    bool is_sprite() const {
        return _type == DISPLAYOBJECT;
    }
    
    /// Get a std::string representation for this value.
    //
    /// @param version      The SWF version to use to transform the string.
    ///                     This only affects undefined values, which trace
    ///                     "undefined" for version 7 and above, nothing
    ///                     for lower versions.
    //
    /// TODO: drop the default argument.
    std::string to_string(int version = 7) const;
    
    /// Get a number representation for this value
    //
    /// This function performs conversion if necessary.
    double to_number(int version) const;
    
    /// Conversion to boolean.
    //
    /// This function performs conversion if necessary.
    bool to_bool(int version) const;
    
    /// Return value as an object, converting primitive values as needed.
    //
    /// This function performs conversion where necessary.
    //
    /// string values are converted to String objects
    /// numeric values are converted to Number objects
    /// boolean values are converted to Boolean objects
    ///
    /// If you want to avoid the conversion, check with is_object() before
    /// calling this function.
    //
    /// @param global   The global object object for the conversion. This
    ///                 contains the prototypes or constructors necessary for
    ///                 conversion.
    as_object* to_object(VM& vm) const;

    /// Return the value as an as_object only if it is an as_object.
    //
    /// Note that this performs no conversion, so returns 0 if the as_value
    /// is not an object.
    as_object* get_object() const;
    
    /// Returns value as a MovieClip if it is a MovieClip.
    //
    /// This function performs no conversion, so returns 0 if the as_value is
    /// not a MovieClip.
    //
    /// This is just a wrapper around toDisplayObject() performing 
    /// an additional final cast.
    MovieClip* toMovieClip(bool skipRebinding = false) const;
    
    /// Return value as a DisplayObject or NULL if this is not possible.
    //
    /// Note that this function performs no conversion, so returns 0 if the
    /// as_value is not a DisplayObject.
    //
    /// If the value is a DisplayObject value, the stored DisplayObject target
    /// is evaluated using the root movie's environment.
    /// If the target points to something that doesn't cast to a DisplayObject,
    /// 0 is returned.
    ///
    /// @param skipRebinding    If true a reference to a destroyed
    ///                         DisplayObject is still returned, rather than
    ///                         attempting to resolve it as a soft-reference.
    ///	                        Main use for this is during paths resolution,
    ///                         to avoid infinite loops. See bug #21647.
    DisplayObject* toDisplayObject(bool skipRebinding = false) const;

    /// Return the value as a function only if it is a function.
    //
    /// Note that this performs no conversion, so returns 0 if the as_value
    /// is not a function.
    as_function* to_function() const;
    
    AsType defaultPrimitive(int version) const;
    
    /// Return value as a primitive type, with a preference
    //
    /// This function performs no conversion.
    //
    /// Primitive types are: undefined, null, boolean, string, number.
    /// See ECMA-2.6.2 (sections 4.3.2 and 8.6.2.6).
    ///
    /// @param hint
    /// 	NUMBER or STRING, the preferred representation we're asking for.
    ///
    /// @throw ActionTypeError if an object can't be converted to a primitive
    ///
    as_value to_primitive(AsType hint) const;
    
    /// Set to a primitive string.
    void set_string(const std::string& str);
    
    /// Set to a primitive number.
    void set_double(double val);
    
    /// Set to a primitive boolean.
    void set_bool(bool val);

    /// Make this value a NULL, OBJECT, DISPLAYOBJECT value
    void set_as_object(as_object* obj);
    
    /// Set to undefined.
    void set_undefined();
    
    /// Set this value to the NULL value
    void set_null();
    
    bool is_undefined() const {
        return (_type == UNDEFINED);
    }
    
    bool is_null() const {
        return (_type == NULLTYPE);
    }
    
    bool is_bool() const {
        return (_type == BOOLEAN);
    }
    
    bool is_exception() const {
        return (_type == UNDEFINED_EXCEPT || _type == NULLTYPE_EXCEPT
                || _type == BOOLEAN_EXCEPT || _type == NUMBER_EXCEPT
                || _type == OBJECT_EXCEPT || _type == DISPLAYOBJECT_EXCEPT
                || _type == STRING_EXCEPT);
    }
    
    // Flag or unflag an as_value as an exception -- this gets flagged
    // when an as_value is 'thrown'.
    void flag_exception() {
        if (!is_exception()) {
            _type = static_cast<AsType>(static_cast<int>(_type) + 1);
        }
    }
    
    void unflag_exception() {
        if (is_exception()) {
            _type = static_cast<AsType>(static_cast<int>(_type) - 1);
        }
    }
    
    /// Return true if this value is strictly equal to the given one
    //
    /// Strict equality is defined as the two values being of the
    /// same type and the same value.
    bool strictly_equals(const as_value& v) const;
    
    /// Return true if this value is abstractly equal to the given one
    //
    /// See ECMA-262 abstract equality comparison (sect 11.9.3)
    ///
    /// NOTE: these invariants should hold 
    ///
    ///	- A != B is equivalent to ! ( A == B )
    ///	- A == B is equivalent to B == A, except for order of
    ///	  evaluation of A and B.
    ///
    /// @param v     The as_value to compare to
    DSOEXPORT bool equals(const as_value& v, int version) const;
    
    /// Set any object value as reachable (for the GC)
    //
    /// Object values are values stored by pointer (objects and functions)
    void setReachable() const;
    
    /// Serialize value in AMF0 format.
    //
    /// @param buf
    ///     The buffer to append serialized version of this value to.
    ///
    /// @param offsetTable
    ///     A map of already-parsed objects, pass an empty map on first call as
    ///     it will be used internally.
    ///
    /// @param vm
    ///     Virtual machine to use for serialization of property names
    ///     (string_table)
    ///
    /// @param allowStrictArray
    ///     If true strict arrays will be encoded a STRICT_ARRAY types.
    ///
    bool writeAMF0(amf::Writer& w) const;

private:

    /// AsValueType handles the following AS types:
    //
    /// 1. undefined / null
    /// 2. Number
    /// 3. Boolean
    /// 4. Object
    /// 5. MovieClip
    /// 6. String
    typedef boost::variant<boost::blank,
                           double,
                           bool,
                           as_object*,
                           CharacterProxy,
                           std::string>
    AsValueType;
    
    /// Use the relevant equality function, not operator==
    bool operator==(const as_value& v) const;
    
    /// Use the relevant inequality function, not operator!=
    bool operator!=(const as_value& v) const;
    
    /// Compare values of the same type
    //
    /// NOTE: will abort if values are not of the same type!
    ///
    bool equalsSameType(const as_value& v) const;
    
    AsType _type;
    
    AsValueType _value;
    
    /// Get the object pointer variant member.
    //
    /// Callers must check that this is an Object (including DisplayObjects).
    as_object* getObj() const;
    
    /// Get the DisplayObject variant member.
    //
    /// The caller must check that this is a DisplayObject.
    DisplayObject* getCharacter(bool skipRebinding = false) const;

    /// Get the DisplayObject proxy variant member.
    //
    /// The caller must check that this value is a DisplayObject
    CharacterProxy getCharacterProxy() const;

    /// Get the number variant member.
    //
    /// The caller must check that this value is a Number.
    double getNum() const {
        assert(_type == NUMBER);
        return boost::get<double>(_value);
    }
    
    /// Get the boolean variant member.
    //
    /// The caller must check that this value is a Boolean.
    bool getBool() const {
        assert(_type == BOOLEAN);
        return boost::get<bool>(_value);
    }

    /// Get the boolean variant member.
    //
    /// The caller must check that this value is a String.
    const std::string& getStr() const {
        assert(_type == STRING);
        return boost::get<std::string>(_value);
    }
    
};

/// Stream operator.
std::ostream& operator<<(std::ostream& os, const as_value& v);

/// Convert numeric value to string value, following ECMA-262 specification
//
// Printing formats:
//
// If _val > 1, Print up to 15 significant digits, then switch
// to scientific notation, rounding at the last place and
// omitting trailing zeroes.
// For values < 1, print up to 4 leading zeroes after the
// decimal point, then switch to scientific notation with up
// to 15 significant digits, rounding with no trailing zeroes
// If the value is negative, just add a '-' to the start; this
// does not affect the precision of the printed value.
//
// This almost corresponds to iomanip's std::setprecision(15)
// format, except that iomanip switches to scientific notation
// at e-05 not e-06, and always prints at least two digits for the exponent.
std::string doubleToString(double val, int radix = 10);

/// Try to parse a string into a 32-bit signed int using base 8 or 16.  //
/// This function will throw a boost::bad_lexical_cast (or a derived
/// exception) if the passed string cannot be converted.
//
/// @param s      The string to parse
/// @param d      The 32-bit int represented as a double. This is only a
///               valid number if the return value is true.
/// @param whole  If true, expect the whole string to be valid, i.e.
///               throw if there are any invalid DisplayObjects. If false,
///               returns any valid number up to the first invalid
///               DisplayObject.
/// @return       True if the string was non-decimal and successfully
///               parsed.
bool parseNonDecimalInt(const std::string& s, double& d, bool whole = true);

/// Set a value to NaN
inline void
setNaN(as_value& v) {
    v.set_double(NaN);
}

} // namespace gnash

#endif // GNASH_AS_VALUE_H

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:

