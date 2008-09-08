// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#include "utility.h" // UNUSED
#include "string_table.h"

// Forward declarations
namespace gnash {
    class VM;
	class as_object;
	class fn_call;
	class as_function;
	class sprite_instance;
	class character;
	class asNamespace;
	class asName;
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
isNaN(const T& num, typename boost::enable_if<boost::is_floating_point<T> >::type* dummy = 0)
{
	UNUSED(dummy);
	return num != num;
}

#define isinf(x) (isNaN(x - x))

/// Use this methods to obtain a properly-formatted property name
/// The methods will convert the name to lowercase if the current VM target
/// is SWF6 or lower
///
//#define PROPNAME(x) ( VM::get().getSWFVersion() < 7 ? boost::to_lower_copy(std::string(x)) : (x) )
#define PROPNAME(x) ( x )

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
class DSOEXPORT as_value
{
public:
	enum type
	{
		// Always make the exception type one greater than the normal type.
		
		/// Undefined value
		UNDEFINED,
		UNDEFINED_EXCEPT,

		/// NULL value
		NULLTYPE,
		NULLTYPE_EXCEPT,

		/// Boolean value
		BOOLEAN,
		BOOLEAN_EXCEPT,

		/// String value
		STRING,
		STRING_EXCEPT,

		/// Number value
		NUMBER, 
		NUMBER_EXCEPT,

		/// Object reference
		OBJECT,
		OBJECT_EXCEPT,

		/// ActionScript function reference
		AS_FUNCTION,
		AS_FUNCTION_EXCEPT,

		/// MovieClip reference
		MOVIECLIP,
		MOVIECLIP_EXCEPT
	};

	/// Construct an UNDEFINED value
	as_value();

	/// Copy-construct a STRING value 
	as_value(const as_value& v);

	/// Construct a STRING value 
	as_value(const char* str);

	/// Construct a STRING value 
	as_value(const std::string& str);

	/// Construct a BOOLEAN value
	as_value(bool val);

	/// Construct a NUMBER value
	as_value(int val);

	/// Construct a NUMBER value
	as_value(unsigned int val);

	/// Construct a NUMBER value
	as_value(float val);

	/// Construct a NUMBER value
	as_value(double val);

	/// Construct a NUMBER value
	as_value(long val);
	
	/// Construct a NUMBER value
	as_value(unsigned long val);

	/// Chad: Document this
	as_value(asNamespace &);

	/// Construct a value from an AMF element
	as_value(const amf::Element& el);
	
	/// Construct a NULL, OBJECT, MOVIECLIP or AS_FUNCTION value
	//
	/// See as_object::to_movie and as_object::to_function
	///
	/// Internally adds a reference to the ref-counted as_object, 
	/// if not-null
	///
	as_value(as_object* obj);

	/// Construct an NULL, MOVIECLIP, AS_FUNCTION or OBJECT value
	as_value(boost::intrusive_ptr<as_object> obj);

	/// Construct a NULL or AS_FUNCTION value
	as_value(as_function* func);

	/// Read AMF0 data from the given buffer
	//
	/// Pass pointer to buffer and pointer to end of buffer. Buffer is raw AMF
	/// encoded data. Must start with a type byte unless third parameter is set.
	///
	/// On success, sets the given as_value and returns true.
	/// On error (premature end of buffer, etc.) returns false and leaves the given
	/// as_value untouched.
	///
	/// IF you pass a fourth parameter, it WILL NOT READ A TYPE BYTE, but use what
	/// you passed instead.
	///
	/// The l-value you pass as the first parameter (buffer start) is updated to
	/// point just past the last byte parsed
	///
	/// TODO restore first parameter on parse errors
	///
	/// @param objRefs
	///     A vector of already-parsed objects to properly interpret references.
	///     Pass an empty vector on first call as it will be used internally.
	///     On return, the vector will be filled with pointers to every complex object
	///     parsed from the stream.
	///
	bool readAMF0(boost::uint8_t *&b, boost::uint8_t *end, int inType, std::vector<as_object*>& objRefs);

    /// Serialize value in AMF0 format.
    //
    /// @param buf
    ///     The buffer to append serialized version of this value to.
    ///
    /// @param offsetTable
    ///     A map of already-parsed objects, pass an empty map on first call as
    ///     it will be used internally.
    ///
    bool writeAMF0(SimpleBuffer& buf, std::map<as_object*, size_t>& offsetTable, VM& vm) const;

	/// Convert numeric value to string value, following ECMA-262 specification
	//
	/// TODO: move here some of the good comments found in the function definition.
	///
	static std::string doubleToString(double val, int radix=10);

	/// Return the primitive type of this value, as a string.
	const char* typeOf() const;

	/// Get the primitive type of this value
	primitive_types ptype() const;

	// Chad: Document
	bool conforms_to(string_table::key name);

	/// \brief
	/// Return true if this value is callable
	/// (AS_FUNCTION).
	bool is_function() const
	{
		return m_type == AS_FUNCTION;
	}

	/// Return true if this value is a AS function
	bool is_as_function() const
	{
		return m_type == AS_FUNCTION;
	}

	/// Return true if this value is strictly a string
	//
	/// Note that you usually DON'T need to call this
	/// function, as if you really want a string you
	/// can always call the to_string() or to_std_string()
	/// method to perform a conversion.
	///
	bool is_string() const
	{
		return m_type == STRING;
	}

	/// Return true if this value is strictly a number
	//
	/// Note that you usually DON'T need to call this
	/// function, as if you really want a number you
	/// can always call the to_number()
	/// method to perform a conversion.
	///
	bool is_number() const
	{
		return m_type == NUMBER;
	}

	/// \brief
	/// Return true if this value is an object
	/// (OBJECT, AS_FUNCTION or MOVIECLIP).
	bool is_object() const
	{
		return m_type == OBJECT || m_type == AS_FUNCTION || m_type == MOVIECLIP;
	}

	/// \brief
	/// Return true if this value is a MOVIECLIP 
	/// 
	bool is_sprite() const
	{
		return m_type == MOVIECLIP;
	}

	/// Get a std::string representation for this value.
	//
	/// @param env
	///	The environment to use for running the toString() method
	///	for object values. If NULL, toString() won't be run.
	///
	std::string to_string() const;

    // Used for operator<< to give useful information about an
    // as_value object.
	std::string toDebugString() const;

	/// Get a string representation for this value.
	//
	/// This differs from to_string() in that returned
	/// representation will depend on version of the SWF
	/// source. 
	/// @@ shouldn't this be the default ?
	///
	/// @param env
	///	The environment to use for running the toString() method
	///	for object values. If NULL, toString() won't be run.
	///
	std::string to_string_versioned(int version) const;

	/// Get a number representation for this value
	//
	/// @param env
	///	The environment to use for running the valueOf() method
	///	for object values. If NULL, valueOf() won't be run.
	///
	double	to_number() const;

	/// Get an AMF element representation for this value
	std::auto_ptr<amf::Element> to_element() const;

	/// Conversion to 32bit integer
	//
	/// Use this conversion whenever an int is needed.
	/// This is NOT the same as calling to_number<boost::int32_t>().
	///
	/// @param env
	///	The environment to use for running the valueOf() method
	///	for object values. 
	///
	boost::int32_t	to_int() const;

	/// Shorthand: casts the result of to_number() to the requested number
	/// type.
	//
	/// Parameter identical to that of to_number().
	///
	/// TODO: deprecate this function, it gets confusing as when an integer
	///       is needed the caller should invoke to_int() rather then to_number().
	///       Implementing specializations for *all* integer types might be tedious
	///
	template <typename T>
	T to_number () const
	{
		return static_cast<T>(to_number());
	}

	/// Conversion to boolean.
	//
	/// Will call version-dependent functions
	/// based on current version.
	///
	/// See to_bool_v5(), to_bool_v6(), to_bool_v7() 
	///
	bool	to_bool() const;

	/// Conversion to boolean for SWF7 and up
	//
	/// See to_bool()
	///
	bool	to_bool_v7() const;

	/// Conversion to boolean for SWF6
	//
	/// See to_bool()
	///
	bool	to_bool_v6() const;

	/// Conversion to boolean up to SWF5
	//
	/// See to_bool()
	///
	bool	to_bool_v5() const;

	/// Return value as an object, converting primitive values as needed.
	//
	/// Make sure you don't break the intrusive_ptr chain
	/// as the returned object might be a newly allocated one in case
	/// of a conversion from a primitive string, number or boolean value.
	///
	/// string values will be converted to String objects,
	/// numeric values will be converted to Number objects,
	/// boolean values are currently NOT converted, but should (FIXME!)
	///
	/// If you want to avoid the conversion, check with is_object() before
	/// calling this function.
	///
	boost::intrusive_ptr<as_object> to_object() const;

	/// Return value as a sprite or NULL if this is not possible.
	//
	/// This is just a wrapper around to_character() performing 
	/// an additional final cast.
	///
	sprite_instance* to_sprite(bool skipRebinding=false) const;

	/// Return value as a character or NULL if this is not possible.
	//
	/// If the value is a MOVIECLIP value, the stored character target
	/// is evaluated using the root movie's environment.
	/// If the target points to something that doesn't cast to a character,
	/// NULL is returned.
	///
	/// Note that if the value is NOT a MOVIECLIP type, NULL is always
	/// returned.
	///
	/// @param skipRebinding
	/// 	If true a reference to a destroyed character is still returned
	///	as such, rather then attempted to be resolved as a soft-reference.
	///	Main use for this is during paths resolution, to avoid
	///	infinite loops. See bug #21647.
	///
	character* to_character(bool skipRebinding=false) const;

	/// \brief
	/// Return value as an ActionScript function ptr
	/// or NULL if it is not an ActionScript function.
	as_function*	to_as_function() const;

	/// Convert this value to a primitive type
	//
	/// Primitive types are: undefined, null, boolean, string, number.
	/// See ECMA-2.6.2 (sections 4.3.2 and 8.6.2.6).
	///
	/// @throw ActionTypeError if an object can't be converted to a primitive
	///
	as_value& convert_to_primitive();

	/// Return value as a primitive type
	//
	/// Primitive types are: undefined, null, boolean, string, number.
	/// See ECMA-2.6.2 (sections 4.3.2 and 8.6.2.6).
	///
	/// @throw ActionTypeError if an object can't be converted to a primitive
	///
	as_value to_primitive() const;

	/// Return value as a primitive type, with a preference
	//
	/// Primitive types are: undefined, null, boolean, string, number.
	/// See ECMA-2.6.2 (sections 4.3.2 and 8.6.2.6).
	///
	/// @param env
	/// 	The environment to use for calling the valueOf method.
	///
	/// @param hint
	/// 	NUMBER or STRING, the preferred representation we're asking for.
	///
	/// @throw ActionTypeError if an object can't be converted to a primitive
	///
	as_value to_primitive(type hint) const;

	/// Convert this value to a primitive type, with a preference
	//
	/// Primitive types are: undefined, null, boolean, string, number.
	/// See ECMA-2.6.2 (sections 4.3.2 and 8.6.2.6).
	///
	/// @param hint
	/// 	NUMBER or STRING, the preferred representation we're asking for.
	///
	/// @throw ActionTypeError if an object can't be converted to a primitive
	///
	as_value& convert_to_primitive(type hint);

	/// Force type to number.
	//
	/// @param env
	///	The environment to use for running the valueOf() method
	///	for object values. If NULL, valueOf() won't be run.
	///
	void	convert_to_number();

	/// Force type to string.
	void	convert_to_string();
    
	/// Force type to bool.
	void	convert_to_boolean();
    
	/// Force type to string.
	//
	/// uses swf-version-aware converter
	///
	/// @param env
	///	The environment to use for running the toString() method
	///	for object values. If NULL, toString() won't be run.
	///
	/// @see to_string_versionioned
	///
	void	convert_to_string_versioned(int version);

	// These set_*()'s are more type-safe; should be used
	// in preference to generic overloaded set().  You are
	// more likely to get a warning/error if misused.

	void	set_string(const std::string& str);

	void	set_std_string(const std::string& str)
	{
		set_string(str);
	}

	void	set_string(const char* str)
	{
		set_string(std::string(str));
	}

	void	set_double(double val);

	void	set_bool(bool val);

	void	set_sprite(sprite_instance& sp);

	void	set_character(character& sp);

	void	set_int(int val) { set_double(val); }

	void	set_nan() { set_double(NaN); }

	/// Make this value a NULL, OBJECT, MOVIECLIP or AS_FUNCTION value
	//
	/// See as_object::to_movie and as_object::to_function
	///
	/// Internally adds a reference to the ref-counted as_object, 
	/// if not-null
	///
	void	set_as_object(as_object* obj);

	void	set_as_object(boost::intrusive_ptr<as_object> obj);

	/// Make this a NULL or AS_FUNCTION value
	void	set_as_function(as_function* func);

	void	set_undefined();

	/// Set this value to the NULL value
	void set_null();

	/// Equality operator, follows strict equality semantic
	//
	/// See strictly_equals
	///
	bool operator==(const as_value& v) const
	{
		return strictly_equals(v);
	}

	/// Inequality operator, follows strict inequality semantic
	//
	/// See strictly_equals
	///
	bool operator!=(const as_value& v) const {
		return ! ( *this  == v );
	}

	void	operator=(const as_value& v);

	bool	is_undefined() const { return (m_type == UNDEFINED); }

	bool	is_null() const { return (m_type == NULLTYPE); }

	bool	is_bool() const { return (m_type == BOOLEAN); }

	bool	is_exception() const
	{ return (m_type == UNDEFINED_EXCEPT || m_type == NULLTYPE_EXCEPT
		|| m_type == BOOLEAN_EXCEPT || m_type == NUMBER_EXCEPT
		|| m_type == OBJECT_EXCEPT || m_type == AS_FUNCTION_EXCEPT
		|| m_type == MOVIECLIP_EXCEPT || m_type == STRING_EXCEPT);
	}

	// Flag or unflag an as_value as an exception -- this gets flagged
	// when an as_value is 'thrown'.
	void	flag_exception() 
	{ if (!is_exception()) m_type = static_cast<type>(static_cast<int>(m_type) + 1); }
	void	unflag_exception()
	{ if (is_exception()) m_type = static_cast<type>(static_cast<int>(m_type) - 1); }

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
	/// @param env
	///	The environment to use for running the toString() and valueOf()
	///	methods for object values. 
	///
	bool equals(const as_value& v) const;

	/// Sets this value to this string plus the given string.
	void	string_concat(const std::string& str);

	/// Equivalent of ActionNewAdd
	as_value& newAdd(const as_value& v1);

	/// Equivalent of ActionNewLessThan
	as_value newLessThan(const as_value& op2_in) const;

	// Equivalent of ActionSubtract
	as_value& subtract(const as_value& o);

	/// Set any object value as reachable (for the GC)
	//
	/// Object values are values stored by pointer (objects and functions)
	///
	void setReachable() const;

private:

	/// Compare values of the same type
	//
	/// NOTE: will abort if values are not of the same type!
	///
	bool equalsSameType(const as_value& v) const;

	type m_type;

	typedef sprite_instance* SpritePtr;
	typedef character* CharacterPtr;
	typedef boost::intrusive_ptr<as_function> AsFunPtr;
	typedef boost::intrusive_ptr<as_object> AsObjPtr;

        boost::variant<  
			 boost::blank,  // UNDEFINED or NULL
                         double,	// NUMBER
                         bool,		// BOOLEAN
                         AsObjPtr,	// OBJECT,
//                        AsFuncPtr,	// AS_FUNCTION
                         CharacterProxy,	// MOVIECLIP
			 std::string	// STRING 
                      > _value;


	/// Get the function pointer variant member (we assume m_type == FUNCTION)
	AsFunPtr getFun() const;

	/// Get the object pointer variant member (we assume m_type == OBJECT)
	AsObjPtr getObj() const;

	/// Get the sprite pointer variant member (we assume m_type == MOVIECLIP)
	//
	/// NOTE: this is possibly NULL !
	///
	SpritePtr getSprite(bool skipRebinding=false) const;

	/// Get the character pointer variant member (we assume m_type == MOVIECLIP)
	//
	/// NOTE: this is possibly NULL !
	///
	CharacterPtr getCharacter(bool skipRebinding=false) const;

	/// Get the sprite proxy variant member (we assume m_type == MOVIECLIP)
	//
	CharacterProxy getCharacterProxy() const;

	/// Get the number variant member (we assume m_type == NUMBER)
	double getNum() const
	{
		assert(m_type == NUMBER);
		return boost::get<double>(_value);
	}

	/// Get the boolean variant member (we assume m_type == BOOLEAN)
	bool getBool() const
	{
		assert(m_type == BOOLEAN);
		return boost::get<bool>(_value);
	}

	/// Get the boolean variant member (we assume m_type == STRING)
	const std::string& getStr() const
	{
		assert(m_type == STRING);
		return boost::get<std::string>(_value);
	}

};

typedef as_value (*as_c_function_ptr)(const fn_call& fn);

inline std::ostream& operator<< (std::ostream& os, const as_value& v) {
	return os << v.toDebugString();
}

} // namespace gnash

#endif // GNASH_AS_VALUE_H

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

