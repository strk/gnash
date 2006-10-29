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
//

/* $Id: as_value.h,v 1.16 2006/10/29 18:34:11 rsavoye Exp $ */

#ifndef GNASH_AS_VALUE_H
#define GNASH_AS_VALUE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cmath>

#include "container.h"
#include "tu_config.h"
//#include "resource.h" // for inheritance of as_object

namespace gnash {

class as_object;
class fn_call;
class as_function;

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
 
typedef void (*as_c_function_ptr)(const fn_call& fn);

// ActionScript value type.

//No private: ???
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

		/// Internal function pointer
		C_FUNCTION,

		/// ActionScript function reference
		AS_FUNCTION
	};

	type	m_type;

	mutable tu_string	m_string_value;

	union
	{
		bool m_boolean_value;
		// @@ hm, what about PS2, where double is bad?  should maybe have int&float types.
		mutable	double	m_number_value;
		as_object*	m_object_value;
		as_c_function_ptr	m_c_function_value;
		as_function*	m_as_function_value;
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

	/// Construct an OBJECT value
	as_value(as_object* obj);

	/// Construct a C_FUNCTION value
	as_value(as_c_function_ptr func)
		:
		m_type(C_FUNCTION),
		m_c_function_value(func)
	{
	}

	/// Construct an AS_FUNCTION value
	as_value(as_function* func);

	~as_value() { drop_refs(); }

	/// Drop any ref counts we have.
	//
	/// This happens prior to changing our value.
	/// Useful when changing types/values.
	///
	void	drop_refs();

	type	get_type() const { return m_type; }

	/// \brief
	/// Return true if this value is callable
	/// (C_FUNCTION or AS_FUNCTION).
	bool is_function() const
	{
		return m_type == C_FUNCTION || m_type == AS_FUNCTION;
	}

	/// \brief
	/// Return true if this value is an object
	/// (OBJECT or AS_FUNCTION).
	bool is_object() const
	{
		return m_type == OBJECT || m_type == AS_FUNCTION;
	}

	/// Get a C string representation of this value.
	const char*	to_string() const;

	/// Get a tu_string representation for this value.
	const tu_string&	to_tu_string() const;

	/// Get a tu_string representation for this value.
	//
	/// This differs from to_tu_string() in that returned
	/// representation will depend on version of the SWF
	/// source. 
	/// @@ shouldn't this be the default ?
	///
	const tu_string&	to_tu_string_versioned(int version) const;

	/// Calls to_tu_string() returning a cast to tu_stringi
	const tu_stringi&	to_tu_stringi() const;

	/// Conversion to double.
	double	to_number() const;

	/// Conversion to boolean.
	bool	to_bool() const;

	/// \brief
	/// Return value as an object
	/// or NULL if this is not possible.
	as_object*	to_object() const;

	/// \brief
	/// Return value as a C function ptr
	/// or NULL if it is not a C function.
	as_c_function_ptr	to_c_function() const;

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
	/// @see to_tu_string_versionioned
	///
	void	convert_to_string_versioned(int version);

	// These set_*()'s are more type-safe; should be used
	// in preference to generic overloaded set().  You are
	// more likely to get a warning/error if misused.

	void	set_tu_string(const tu_string& str) { drop_refs(); m_type = STRING; m_string_value = str; }

	void	set_string(const char* str) { drop_refs(); m_type = STRING; m_string_value = str; }
	void	set_double(double val) { drop_refs(); m_type = NUMBER; m_number_value = val; }
	void	set_bool(bool val) { drop_refs(); m_type = BOOLEAN; m_boolean_value = val; }
	void	set_int(int val) { set_double(val); }
	void	set_nan() { double x = 0.0; set_double(x/x); }

	/// Make this value an as_object.
	/// Internally adds a reference to the ref-counted as_object.
	void	set_as_object(as_object* obj);

	void	set_as_c_function_ptr(as_c_function_ptr func)
	{
		drop_refs(); m_type = C_FUNCTION; m_c_function_value = func;
	}
	void	set_as_function(as_function* func);

	void	set_undefined() { drop_refs(); m_type = UNDEFINED; }

	void	set_null() { drop_refs(); m_type = NULLTYPE; }

	void	operator=(const as_value& v)
	{
		if (v.m_type == UNDEFINED) set_undefined();
		else if (v.m_type == NULLTYPE) set_null();
		else if (v.m_type == BOOLEAN) set_bool(v.m_boolean_value);
		else if (v.m_type == STRING) set_tu_string(v.m_string_value);
		else if (v.m_type == NUMBER) set_double(v.m_number_value);
		else if (v.m_type == OBJECT) set_as_object(v.m_object_value);
		else if (v.m_type == C_FUNCTION) set_as_c_function_ptr(v.m_c_function_value);
		else if (v.m_type == AS_FUNCTION) set_as_function(v.m_as_function_value);
	}

	bool	is_nan() const { return (m_type == NUMBER && isnan(m_number_value)); }
	bool	is_inf() const { return (m_type == NUMBER && isinf(m_number_value)); }

	bool	is_undefined() const { return (m_type == UNDEFINED); }

	bool	is_null() const { return (m_type == NULLTYPE); }

	bool is_finite() const { return (m_type == NUMBER && isfinite(m_number_value)); }

	bool	operator==(const as_value& v) const;
	bool	operator!=(const as_value& v) const;
	bool	operator<(const as_value& v) const { return to_number() < v.to_number(); }
	void	operator+=(const as_value& v) { set_double(this->to_number() + v.to_number()); }
	void	operator-=(const as_value& v) { set_double(this->to_number() - v.to_number()); }
	void	operator*=(const as_value& v) { set_double(this->to_number() * v.to_number()); }
	void	operator/=(const as_value& v) { set_double(this->to_number() / v.to_number()); }  // @@ check for div/0
	void	operator&=(const as_value& v) { set_int(int(this->to_number()) & int(v.to_number())); }
	void	operator|=(const as_value& v) { set_int(int(this->to_number()) | int(v.to_number())); }
	void	operator^=(const as_value& v) { set_int(int(this->to_number()) ^ int(v.to_number())); }
	void	shl(const as_value& v) { set_int(int(this->to_number()) << int(v.to_number())); }
	void	asr(const as_value& v) { set_int(int(this->to_number()) >> int(v.to_number())); }
	void	lsr(const as_value& v) { set_int((uint32_t(this->to_number()) >> int(v.to_number()))); }

	/// Sets this value to this string plus the given string.
	void	string_concat(const tu_string& str);

	tu_string* get_mutable_tu_string() { assert(m_type == STRING); return &m_string_value; }
};

inline std::ostream& operator<< (std::ostream& os, const as_value& v) {
	return os << v.to_string();
}

} // namespace gnash

#endif // GNASH_AS_VALUE_H
