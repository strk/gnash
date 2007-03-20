// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

/* $Id: as_value.h,v 1.36 2007/03/20 15:01:20 strk Exp $ */

#ifndef GNASH_AS_VALUE_H
#define GNASH_AS_VALUE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "container.h"
#include "tu_config.h"
#include "smart_ptr.h"

#include <cmath>
#include <string>

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

	/// Construct a STRING value
	as_value(const wchar_t* wstr)
		:
		m_type(STRING),
		m_string_value(""),
		m_number_value(0.0)
	{
		// Encode the string value as UTF-8.
		//
		// Is this dumb?  Alternatives:
		//
		// 1. store a tu_wstring instead of tu_string?
		// Bloats typical ASCII strings, needs a
		// tu_wstring type, and conversion back the
		// other way to interface with char[].
		// 
		// 2. store a tu_wstring as a union with
		// tu_string?  Extra complexity.
		//
		// 3. ??
		//
		// Storing UTF-8 seems like a pretty decent
		// way to do it.  Everything else just
		// continues to work.

#if (WCHAR_MAX != MAXINT)
		tu_string::encode_utf8_from_wchar(&m_string_value, (const uint16 *)wstr);
#else
# if (WCHAR_MAX != MAXSHORT)
# error "Can't determine the size of wchar_t"
# else
		tu_string::encode_utf8_from_wchar(&m_string_value, (const uint32 *)wstr);
# endif
#endif
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

	as_value(long val)
		:
		m_type(NUMBER),
		m_number_value(val)
	{
	}
	
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

	as_value(boost::intrusive_ptr<as_object> obj);

	/// Construct a NULL or AS_FUNCTION value
	as_value(as_function* func);

	~as_value() { drop_refs(); }

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

	/// Get a C string representation of this value.
	//
	/// @param env
	///	The environment to use for running the toString() method
	///	for object values. If NULL, toString() won't be run.
	///
	const char*	to_string(as_environment* env=NULL) const;

	/// Get a tu_string representation for this value.
	//
	/// @param env
	///	The environment to use for running the toString() method
	///	for object values. If NULL, toString() won't be run.
	///
	const tu_string&	to_tu_string(as_environment* env=NULL) const;

	/// Get a std::string representation for this value.
	//
	/// @param env
	///	The environment to use for running the toString() method
	///	for object values. If NULL, toString() won't be run.
	///
	std::string to_std_string(as_environment* env=NULL) const;

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

	/// Get a tu_string representation for this value.
	//
	/// This differs from to_tu_string() in that returned
	/// representation will depend on version of the SWF
	/// source. 
	/// @@ shouldn't this be the default ?
	///
	/// @param env
	///	The environment to use for running the toString() method
	///	for object values. If NULL, toString() won't be run.
	///
	const tu_string&	to_tu_string_versioned(int version, as_environment* env=NULL) const;

	/// Calls to_tu_string() returning a cast to tu_stringi
	//
	/// @param env
	///	The environment to use for running the toString() method
	///	for object values. If NULL, toString() won't be run.
	///
	const tu_stringi&	to_tu_stringi(as_environment* env=NULL) const;

	/// Conversion to number 
	//
	/// @param env
	///	The environment to use for running the valueOf() method
	///	for object values. If NULL, valueOf() won't be run.
	///
	double	to_number(as_environment* env=NULL) const;

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

	/// Return value as a primitive type
	//
	/// Primitive types are: undefined, null, boolean, string, number.
	/// See ECMA-2.6.2 (section 4.3.2).
	as_value to_primitive() const;

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
	void	convert_to_number();

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
	/// @see to_tu_string_versionioned
	///
	void	convert_to_string_versioned(int version, as_environment* env=NULL);

	// These set_*()'s are more type-safe; should be used
	// in preference to generic overloaded set().  You are
	// more likely to get a warning/error if misused.

	void	set_tu_string(const tu_string& str) {
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
	void	set_sprite(const std::string& path);
	void	set_sprite(const sprite_instance& sp);
	void	set_int(int val) { set_double(val); }
	void	set_nan() { double x = 0.0; set_double(x/x); }

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

	void	operator=(const as_value& v);

	bool	is_undefined() const { return (m_type == UNDEFINED); }

	bool	is_null() const { return (m_type == NULLTYPE); }

	/// Return true if this value is strictly equal to the given one
	//
	/// Strict equality is defined as the two values being of the
	/// same type and the same value.
	///
	/// TODO: check what makes two MOVIECLIP values strictly equal
	///
	bool strictly_equals(const as_value& v) const;

	/// Return true if this value is abstractly equal to the given one
	//
	/// See ECMA-262 abstract equality comparison (sect 11.9.3)
	///
	/// @param env
	///	The environment to use for running the toString() method
	///	for object values. If NULL, toString() won't be run.
	///
	bool equals(const as_value& v, as_environment* env=NULL) const;

	/// @deprecated use equals() instead
	bool	operator==(const as_value& v) const;

	bool	operator!=(const as_value& v) const;
	bool	operator<(const as_value& v) const { return to_number() < v.to_number(); }

	/// @deprecated, use v.set_double(v.to_number(env) + v.to_number(env)) instead !
	void	operator+=(const as_value& v) { set_double(to_number() + v.to_number()); }

	void	operator-=(const as_value& v) { set_double(to_number() - v.to_number()); }
	void	operator*=(const as_value& v) { set_double(to_number() * v.to_number()); }
	void	operator/=(const as_value& v) { set_double(to_number() / v.to_number()); }  // @@ check for div/0
	void	operator&=(const as_value& v) { set_int(int(to_number()) & int(v.to_number())); }
	void	operator|=(const as_value& v) { set_int(int(to_number()) | int(v.to_number())); }
	void	operator^=(const as_value& v) { set_int(int(to_number()) ^ int(v.to_number())); }
	void	shl(const as_value& v) { set_int(int(to_number()) << int(v.to_number())); }
	void	asr(const as_value& v) { set_int(int(to_number()) >> int(v.to_number())); }
	void	lsr(const as_value& v) { set_int((uint32_t(to_number()) >> int(v.to_number()))); }

	/// Sets this value to this string plus the given string.
	void	string_concat(const tu_string& str);

	//tu_string* get_mutable_tu_string() { assert(m_type == STRING); return &m_string_value; }

private:

	// TODO: make private. The rationale is that callers of this functions
	//       should use is_WHAT() instead, or changes in the available
	//       primitive value types will require modifications in all callers.
	//       This happened when adding MOVIECLIP.
	//
	type	get_type() const { return m_type; }


	type	m_type;

	// TODO: switch to std::string
	mutable tu_string	m_string_value;

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

