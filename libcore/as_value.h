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


#ifndef GNASH_AS_VALUE_H
#define GNASH_AS_VALUE_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "dsodefs.h"
#include "smart_ptr.h"
#include "CharacterProxy.h"

#include <cmath>
#include <limits>
#include <string>
#include <vector>
#include <boost/variant.hpp>
#include <ostream> // for inlined output operator
#include <boost/type_traits/is_floating_point.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/static_assert.hpp>
#include <boost/shared_ptr.hpp>

#include "utility.h" // UNUSED
#include "string_table.h"

// Forward declarations
namespace gnash {
    class VM;
	class as_object;
	class Global_as;
	class fn_call;
	class as_function;
	class MovieClip;
	class DisplayObject;
    class SimpleBuffer;
}
namespace amf {
	class Element;
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
/// Any ActionScript value is stored into an instance of this
/// class. The instance keeps primitive types by value and
/// composite types by reference (smart pointer).
///
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

	/// Construct an UNDEFINED value
	DSOEXPORT as_value();

	/// Construct a STRING value 
	as_value(const char* str);
	as_value(const std::string& str);

	/// Construct a BOOLEAN value
	template <typename T>
	as_value(T val, typename boost::enable_if<boost::is_same<bool, T> >::type*
            dummy = 0)
		:
        _type(BOOLEAN),
		_value(val)
	{
		UNUSED(dummy);
	}

	/// Construct a NUMBER value
	as_value(double val);
	
	/// Construct a NULL, OBJECT, or DISPLAYOBJECT value
	as_value(as_object* obj);

	/// Copy constructor.
	as_value(const as_value& value);

	/// Construct a value from an AMF element
	as_value(const amf::Element& el);

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
	static std::string doubleToString(double val, int radix=10);

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
    static bool parseNonDecimalInt(const std::string& s, double& d,
            bool whole = true);

	/// Return the primitive type of this value as a string.
	const char* typeOf() const;

	/// Get the primitive type of this value
    //
    /// Only used in AVM2
	primitive_types ptype() const;

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
	std::string to_string() const;

    // Used for operator<< to give useful information about an
    // as_value object.
	DSOEXPORT std::string toDebugString() const;

	/// Get a string representation for this value.
	//
	/// This differs from to_string() in that returned
	/// representation will depend on version of the SWF
	/// source. 
	/// @@ shouldn't this be the default ?
	///
	/// @param version
    ///     SWF version for which the operation is desired.
	///
	std::string to_string_versioned(int version) const;

	/// Get a number representation for this value
	double to_number() const;

	/// Get an AMF element representation for this value
    boost::shared_ptr<amf::Element> to_element() const;

	/// AS-compatible conversion to 32bit integer
    //
    /// This truncates large numbers to fit in the 32-bit space.
	boost::int32_t to_int() const;

	/// Conversion to boolean.
	//
	/// Will call version-dependent functions
	/// based on current version.
	bool to_bool() const;

	/// Conversion to boolean for SWF7 and up
	bool to_bool_v7() const;

	/// Conversion to boolean for SWF6
	bool to_bool_v6() const;

	/// Conversion to boolean up to SWF5
	bool to_bool_v5() const;

	/// Return value as an object, converting primitive values as needed.
	//
    /// This function does perform a conversion.
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
	as_object* to_object(Global_as& global) const;

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

	as_value& convert_to_primitive();

	void set_string(const std::string& str);

	void set_double(double val);

	void set_bool(bool val);

	void set_nan() { set_double(NaN); }

	/// Make this value a NULL, OBJECT, DISPLAYOBJECT value
	//
	/// See as_object::to_movie and as_object::to_function
	///
	/// Internally adds a reference to the ref-counted as_object, 
	/// if not-null
	///
	void set_as_object(as_object* obj);

	void set_undefined();

	/// Set this value to the NULL value
	void set_null();

	void operator=(const as_value& v);

	bool is_undefined() const { return (_type == UNDEFINED); }

	bool is_null() const { return (_type == NULLTYPE); }

	bool is_bool() const { return (_type == BOOLEAN); }

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
	///
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
	/// @param v
    ///     The as_value to compare to
	///
	bool equals(const as_value& v) const;

	/// Sets this value to this string plus the given string.
	void string_concat(const std::string& str);

	/// Set any object value as reachable (for the GC)
	//
	/// Object values are values stored by pointer (objects and functions)
	///
	void setReachable() const;

	/// Read AMF0 data from the given buffer
	//
	/// Pass pointer to buffer and pointer to end of buffer. Buffer is raw AMF
	/// encoded data. Must start with a type byte unless third parameter is set.
	///
	/// On success, sets the given as_value and returns true.
	/// On error (premature end of buffer, etc.) returns false and
    /// leaves the given as_value untouched.
	///
	/// IF you pass a fourth parameter, it WILL NOT READ A TYPE BYTE, but
    /// use what you passed instead.
	///
	/// The l-value you pass as the first parameter (buffer start) is updated to
	/// point just past the last byte parsed
	///
	/// TODO restore first parameter on parse errors
	///
	/// @param b
    ///     Pointer to buffer where to start reading.
    ///     Will be moved as data is read.
    ///
	/// @param end
    ///     Pointer to end of buffer. Reading from this would
    ///     be invalid.
    ///
	/// @param inType
    ///     Type of the AMF object to read. If -1, type will be
    ///     read from a type byte.
    ///
	/// @param objRefs
	///     A vector of already-parsed objects to properly interpret references.
	///     Pass an empty vector on first call as it will be used internally.
	///     On return, the vector will be filled with pointers to every
    ///     complex object parsed from the stream.
    ///
	/// @param vm
    ///     Virtual machine to use for initialization of the values
    ///     (string_table)
	///
	DSOEXPORT bool readAMF0(const boost::uint8_t*& b,
            const boost::uint8_t* const end, int inType,
            std::vector<as_object*>& objRefs, VM& vm);

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
    bool writeAMF0(SimpleBuffer& buf, std::map<as_object*, size_t>& offsetTable,
                   VM& vm, bool allowStrictArray) const;

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


/// Force type to number.
as_value& convertToNumber(as_value& v, VM& vm);

/// Force type to string.
as_value& convertToString(as_value& v, VM& vm);

/// Force type to bool.
as_value& convertToBoolean(as_value& v, VM& vm);

/// Convert to primitive type
as_value& convertToPrimitive(as_value& v, VM& vm);

inline std::ostream& operator<< (std::ostream& os, const as_value& v) {
	return os << v.toDebugString();
}

} // namespace gnash

#endif // GNASH_AS_VALUE_H

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

