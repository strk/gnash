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

/* $Id: as_value.h,v 1.61 2007/08/02 18:28:42 strk Exp $ */

#ifndef GNASH_AS_VALUE_H
#define GNASH_AS_VALUE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//#include "container.h"
#include "tu_config.h"
#include "smart_ptr.h"

#include <cmath>
#include <limits>
#include <string>
#include <ostream> // for inlined output operator

namespace gnash {

class as_object;
class fn_call;
class as_function;
class sprite_instance;
class as_environment;

#ifndef HAVE_ISFINITE
# ifndef isfinite 
#  define isfinite finite
# endif 
#endif 

#ifndef NAN
#       define NAN (std::numeric_limits<double>::quiet_NaN())
#endif

#ifndef INFINITY
#       define INFINITY (std::numeric_limits<double>::infinity())
#endif


#ifndef isnan
# define isnan(x) \
(sizeof (x) == sizeof (long double) ? isnan_ld (x) \
: sizeof (x) == sizeof (double) ? isnan_d (x) \
: isnan_f (x))
static inline int isnan_f  (float       x) { return x != x; }
static inline int isnan_d  (double      x) { return x != x; }
static inline int isnan_ld (long double x) { return x != x; }
#endif
	  
#ifndef isinf
# define isinf(x) \
(sizeof (x) == sizeof (long double) ? isinf_ld (x) \
: sizeof (x) == sizeof (double) ? isinf_d (x) \
: isinf_f (x))
	static inline int isinf_f  (float       x) { return isnan (x - x); }
	static inline int isinf_d  (double      x) { return isnan (x - x); }
	static inline int isinf_ld (long double x) { return isnan (x - x); }
#endif

/// Use this methods to obtain a properly-formatted property name
/// The methods will convert the name to lowercase if the current VM target
/// is SWF6 or lower
///
#define PROPNAME(x) ( VM::get().getSWFVersion() < 7 ? boost::to_lower_copy(std::string(x)) : (x) )
 

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
		/// Undefined value
		UNDEFINED,

		/// NULL value
		NULLTYPE,

		/// Boolean value
		BOOLEAN,

		/// String value
		STRING,

		/// Number value
		NUMBER, 

		/// Object reference
		OBJECT,

		/// ActionScript function reference
		AS_FUNCTION,

		/// MovieClip reference
		MOVIECLIP
	};

	/// Construct an UNDEFINED value
	as_value()
		:
		m_type(UNDEFINED),
		m_number_value(0.0)
	{
	}

	as_value(const as_value& v)
		:
		m_type(UNDEFINED),
		m_number_value(0.0)
	{
		*this = v;
	}

	/// Construct a STRING value 
	as_value(const char* str)
		:
		m_type(STRING),
		m_string_value(str),
		m_number_value(0.0)
	{
	}

	/// Construct a STRING value 
	as_value(const std::string& str)
		:
		m_type(STRING),
		m_string_value(str.c_str()),
		m_number_value(0.0)
	{
	}

	/// Construct a BOOLEAN value
	as_value(bool val)
		:
		m_type(BOOLEAN),
		m_boolean_value(val)
	{
	}

	/// Construct a NUMBER value
	as_value(int val)
		:
		m_type(NUMBER),
		m_number_value(double(val))
	{
	}

	/// Construct a NUMBER value
	as_value(unsigned int val)
		:
		m_type(NUMBER),
		m_number_value(double(val))
	{
	}

	/// Construct a NUMBER value
	as_value(float val)
		:
		m_type(NUMBER),
		m_number_value(double(val))
	{
	}

	/// Construct a NUMBER value
	as_value(double val)
		:
		m_type(NUMBER),
		m_number_value(val)
	{
	}

	/// Construct a NUMBER value
	as_value(long val)
		:
		m_type(NUMBER),
		m_number_value(val)
	{
	}
	
	/// Construct a NUMBER value
	as_value(unsigned long val)
		:
		m_type(NUMBER),
		m_number_value(val)
	{
	}

	/// Construct a NULL, OBJECT, MOVIECLIP or AS_FUNCTION value
	//
	/// See as_object::to_movie and as_object::to_function
	///
	/// Internally adds a reference to the ref-counted as_object, 
	/// if not-null
	///
	as_value(as_object* obj)
		:
		// Initialize to non-object type here,
		// or set_as_object will call
		// drop_ref on undefined memory !!
		m_type(UNDEFINED)
	{
		set_as_object(obj);
	}

	/// Construct an NULL, MOVIECLIP, AS_FUNCTION or OBJECT value
	as_value(boost::intrusive_ptr<as_object> obj);

	/// Construct a NULL or AS_FUNCTION value
	as_value(as_function* func);

	~as_value() { drop_refs(); }

	/// Convert numeric value to string value, following ECMA-262 specification
	//
	/// TODO: move here some of the good comments found in the function definition.
	///
	static std::string doubleToString(double val);

	/// Drop any ref counts we have.
	//
	/// This happens prior to changing our value.
	/// Useful when changing types/values.
	///
	void	drop_refs();

	/// Return the primitive type of this value, as a string.
	const char* typeOf() const;

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

	/// Get a std::string representation for this value.
	//
	/// @param env
	///	The environment to use for running the toString() method
	///	for object values. If NULL, toString() won't be run.
	///
	const std::string& to_string(as_environment* env=NULL) const;

	std::string to_debug_string() const;

	/// Get a version-dependent std::string representation for this value.
	//
	/// @param version
	///	The SWF version to be compatible with.
	///
	/// @param env
	///	The environment to use for running the toString() method
	///	for object values. If NULL, toString() won't be run.
	///
	std::string to_std_string_versioned(int version, as_environment* env=NULL) const;

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
	const std::string&	to_string_versioned(int version, as_environment* env=NULL) const;

	/// Conversion to number 
	//
	/// @param env
	///	The environment to use for running the valueOf() method
	///	for object values. If NULL, valueOf() won't be run.
	///
	double	to_number(as_environment* env=NULL) const;

	/// Shorthand: casts the result of to_number() to the requested number
	/// type.
	//
	/// Parameter identical to that of to_number().
	template <typename T>
	T to_number (as_environment* env=NULL) const
	{
		return static_cast<T>(to_number(env));
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
	/// If the value is a MOVIECLIP value, the stored sprite target
	/// is evaluated using the root movie's environment
	/// (see gnash::as_environment::find_target). If the target
	/// points to something that doesn't cast to a sprite,
	/// NULL is returned.
	///
	/// Note that if the value is NOT a MOVIECLIP, NULL is always
	/// returned.
	///
	sprite_instance* to_sprite() const;

	/// \brief
	/// Return value as an ActionScript function ptr
	/// or NULL if it is not an ActionScript function.
	as_function*	to_as_function() const;

	/// Force type to number.
	//
	/// @param env
	///	The environment to use for running the valueOf() method
	///	for object values. If NULL, valueOf() won't be run.
	///
	void	convert_to_number(as_environment* env);

	/// Force type to string.
	void	convert_to_string();
    
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
	void	convert_to_string_versioned(int version, as_environment* env=NULL);

	// These set_*()'s are more type-safe; should be used
	// in preference to generic overloaded set().  You are
	// more likely to get a warning/error if misused.

	void	set_string(const std::string& str) {
          drop_refs();
          m_type = STRING;
          m_string_value = str;
        }

	void	set_std_string(const std::string& str) {
          drop_refs();
          m_type = STRING;
          m_string_value = str.c_str();
        }

	void	set_string(const char* str) {
          drop_refs();
          m_type = STRING;
          m_string_value = str;
        }

	void	set_double(double val) {
          drop_refs();
          m_type = NUMBER;
          m_number_value = val;
        }

	void	set_bool(bool val) {
          drop_refs();
          m_type = BOOLEAN;
          m_boolean_value = val;
        }

	void	set_sprite(const sprite_instance& sp);

	void	set_int(int val) { set_double(val); }

	void	set_nan() { set_double(NAN); }

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

	void	set_undefined() { drop_refs(); m_type = UNDEFINED; }

	/// Set this value to the NULL value
	//
	/// @return a reference to this instance
	///
	as_value& set_null() { drop_refs(); m_type = NULLTYPE; return *this; }

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
	bool equals(const as_value& v, as_environment& env) const;

	/// Sets this value to this string plus the given string.
	void	string_concat(const std::string& str);

	/// Set any object value as reachable (for the GC)
	//
	/// Object values are values stored by pointer (objects and functions)
	///
	void setReachable() const;

private:

	static sprite_instance* find_sprite_by_target(const std::string& target);

	void	set_sprite(const std::string& path);


	/// Return value as a primitive type
	//
	/// Primitive types are: undefined, null, boolean, string, number.
	/// See ECMA-2.6.2 (section 4.3.2).
	///
	/// @param env
	/// 	The environment to use for calling the valueOf method.
	///
	as_value to_primitive(as_environment& env) const;

	/// Compare values of the same type
	//
	/// NOTE: will abort if values are not of the same type!
	///
	bool equalsSameType(const as_value& v) const;

	// TODO: make private. The rationale is that callers of this functions
	//       should use is_WHAT() instead, or changes in the available
	//       primitive value types will require modifications in all callers.
	//       This happened when adding MOVIECLIP.
	//
	type	get_type() const { return m_type; }


	type	m_type;

	mutable std::string	m_string_value;

	union
	{
		bool m_boolean_value;
		// @@ hm, what about PS2, where double is bad?  should maybe have int&float types.
		mutable	double	m_number_value;
		as_object*	m_object_value;
	};

};

typedef as_value (*as_c_function_ptr)(const fn_call& fn);

inline std::ostream& operator<< (std::ostream& os, const as_value& v) {
	return os << v.to_debug_string();
}

} // namespace gnash

#endif // GNASH_AS_VALUE_H

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

