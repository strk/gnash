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
// Linking Gnash statically or dynamically with other modules is making
// a combined work based on Gnash. Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
// 
// In addition, as a special exception, the copyright holders of Gnash give
// you permission to combine Gnash with free software programs or
// libraries that are released under the GNU LGPL and/or with Mozilla, 
// so long as the linking with Mozilla, or any variant of Mozilla, is
// through its standard plug-in interface. You may copy and distribute
// such a system following the terms of the GNU GPL for Gnash and the
// licenses of the other code concerned, provided that you include the
// source code of that other code when and as the GNU GPL requires
// distribution of source code. 
// 
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is
// their choice whether to do so.  The GNU General Public License gives
// permission to release a modified version without this exception; this
// exception also makes it possible to release a modified version which
// carries forward this exception.
//

#ifndef GNASH_OBJECT_H
#define GNASH_OBJECT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "container.h"
#include "smart_ptr.h"

namespace gnash {

struct as_object;
struct fn_call;
struct function_as_object;
struct movie;
struct font;
struct character_def;
struct sound_sample;

#ifndef HAVE_ISFINITE
# ifndef isfinite 
#  define isfinite finite
# endif 
#endif 
 
typedef void (*as_c_function_ptr)(const fn_call& fn);

/// ActionScript value type.
struct as_value
{
	enum type
	{
		UNDEFINED,
		NULLTYPE,
		BOOLEAN,
		STRING,
		NUMBER, 
		OBJECT,
		C_FUNCTION,
		AS_FUNCTION,	// ActionScript function.
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
		function_as_object*	m_as_function_value;
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
		m_c_function_value = func;
	}

	/// Construct an AS_FUNCTION value
	as_value(function_as_object* func);

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
	function_as_object*	to_as_function() const;

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
	void	set_function_as_object(function_as_object* func);
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
		else if (v.m_type == AS_FUNCTION) set_function_as_object(v.m_as_function_value);
	}

	bool	is_nan() const { return (m_type == NUMBER && isnan(m_number_value)); }
	bool	is_inf() const { return (m_type == NUMBER && isinf(m_number_value)); }
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
	void	lsr(const as_value& v) { set_int((Uint32(this->to_number()) >> int(v.to_number()))); }

	/// Sets this value to this string plus the given string.
	void	string_concat(const tu_string& str);

	tu_string* get_mutable_tu_string() { assert(m_type == STRING); return &m_string_value; }
};

/// Flags defining the level of protection of a member
struct as_prop_flags
{
	/// Numeric flags
	int m_flags;

	/// if true, this value is protected (internal to gnash)
	bool m_is_protected;

	/// mask for flags
	const static int as_prop_flags_mask = 0x7;

	/// Default constructor
	as_prop_flags() : m_flags(0), m_is_protected(false)
	{
	}

	/// Constructor
	as_prop_flags(const bool read_only, const bool dont_delete, const bool dont_enum)
		:
		m_flags(((read_only) ? 0x4 : 0) | ((dont_delete) ? 0x2 : 0) | ((dont_enum) ? 0x1 : 0)),
		m_is_protected(false)
	{
	}

	/// Constructor, from numerical value
	as_prop_flags(const int flags)
		: m_flags(flags), m_is_protected(false)
	{
	}

	/// accessor to m_readOnly
	bool get_read_only() const { return (((this->m_flags & 0x4)!=0)?true:false); }

	/// accessor to m_dontDelete
	bool get_dont_delete() const { return (((this->m_flags & 0x2)!=0)?true:false); }

	/// accessor to m_dontEnum
	bool get_dont_enum() const { return (((this->m_flags & 0x1)!=0)?true:false);	}

	/// accesor to the numerical flags value
	int get_flags() const { return this->m_flags; }

	/// accessor to m_is_protected
	bool get_is_protected() const { return this->m_is_protected; }

	/// setter to m_is_protected
	void set_get_is_protected(const bool is_protected) { this->m_is_protected = is_protected; }

	/// set the numerical flags value (return the new value )
	/// If unlocked is false, you cannot un-protect from over-write,
	/// you cannot un-protect from deletion and you cannot
	/// un-hide from the for..in loop construct
	int set_flags(const int setTrue, const int set_false = 0)
	{
		if (!this->get_is_protected())
		{
			this->m_flags = this->m_flags & (~set_false);
			this->m_flags |= setTrue;
		}

		return get_flags();
	}
};



/// Member for as_object: value + flags
struct as_member
{
	/// value
	as_value m_value;
	/// Properties flags
	as_prop_flags m_flags;

	/// Default constructor
	as_member() {
	}

	/// Constructor
	as_member(const as_value &value,const as_prop_flags flags=as_prop_flags())
		:
		m_value(value),
		m_flags(flags)
	{
	}

	/// accessor to the value
	as_value get_member_value() const { return m_value; }

	/// accessor to the properties flags
	as_prop_flags get_member_flags() const { return m_flags; }

	/// set the value
	void set_member_value(const as_value &value)  { m_value = value; }
	/// accessor to the properties flags
	void set_member_flags(const as_prop_flags &flags)  { m_flags = flags; }
};

/// For stuff that's tricky to keep track of w/r/t ownership & cleanup.
struct ref_counted
{
	ref_counted();
	virtual ~ref_counted();
	void	add_ref() const;
	void	drop_ref() const;
	int	get_ref_count() const { return m_ref_count; }
	weak_proxy*	get_weak_proxy() const;
private:
	mutable int	m_ref_count;
	mutable weak_proxy*	m_weak_proxy;
};


/// An interface for casting to different types of resources.
struct resource : public ref_counted
{
	virtual ~resource() {}
	
	// Override in derived classes that implement corresponding interfaces.
	virtual font*	cast_to_font() { return 0; }
	virtual character_def*	cast_to_character_def() { return 0; }
	virtual sound_sample*	cast_to_sound_sample() { return 0; }
};



/// \brief
/// A generic bag of attributes. Base class for all ActionScript-able objects.
//
/// Base-class for ActionScript script-defined objects.
/// This would likely be ActionScript's 'Object' class.
///
struct as_object : public resource
{
	/// Members of this objects in an hash
	stringi_hash<as_member>	m_members;

	/// Reference to this object's '__proto__'
	as_object*	m_prototype;

	/// Construct an ActionScript object with no prototype associated.
	as_object() : m_prototype(NULL) { }

	/// \brief
	/// Construct an ActionScript object based on the given prototype.
	/// Adds a reference to the prototype, if any.
	as_object(as_object* proto) : m_prototype(proto)
	{
		if (m_prototype) m_prototype->add_ref();
	}

	/// \brief
	/// Default destructor for ActionScript objects.
	/// Drops reference on prototype member, if any.
	virtual ~as_object()
	{
		if (m_prototype) m_prototype->drop_ref();
	}
	
	/// Return a text representation for this object
	virtual const char* get_text_value() const { return NULL; }

	/// Set a member value
	virtual void set_member(const tu_stringi& name,
			const as_value& val );

	/// Get a member as_value by name
	virtual bool get_member(const tu_stringi& name, as_value* val);

	/// Get an member pointer by name
	virtual bool get_member(const tu_stringi& name,
			as_member* member) const;

	/// Set member flags (probably used by ASSetPropFlags)
	virtual bool set_member_flags(const tu_stringi& name,
			const int flags);

	/// This object is not a movie; no conversion.
	virtual movie*	to_movie() { return NULL; }

	void	clear();

	/// Check whether this object is an instance of the given
	/// function_as_object constructor
	//
	/// NOTE: built-in classes should NOT be C_FUNCTIONS for this to
	/// work
	///
	bool instanceOf(function_as_object* ctor);
};

} // namespace gnash

#endif // GNASH_OBJECT_H
