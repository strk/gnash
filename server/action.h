// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

// Implementation and helpers for SWF actions.


#ifndef GNASH_ACTION_H
#define GNASH_ACTION_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "gnash.h"
#include "types.h"
#include <wchar.h>

#include "container.h"
#include "smart_ptr.h"
//#include "Function.h"
#include "log.h"

namespace gnash {
	struct movie;
	struct as_environment;
	struct as_object;
	struct as_object_interface;
	struct as_value;
	struct function_as_object;


	extern smart_ptr<as_object> s_global;

	//
	// event_id
	//

	/// For keyDown and stuff like that.
	struct event_id
	{

		/// These must match the function names in event_id::get_function_name()
		enum id_code
		{
			INVALID,

			// These are for buttons & sprites.
			PRESS,
			RELEASE,
			RELEASE_OUTSIDE,
			ROLL_OVER,
			ROLL_OUT,
			DRAG_OVER,
			DRAG_OUT,
			KEY_PRESS,

			// These are for sprites only.
			INITIALIZE,
			LOAD,
			UNLOAD,
			ENTER_FRAME,
			MOUSE_DOWN,
			MOUSE_UP,
			MOUSE_MOVE,
			KEY_DOWN,
			KEY_UP,
			DATA,
			
                        // These are for the MoveClipLoader ActionScript only
                        LOAD_START,
                        LOAD_ERROR,
                        LOAD_PROGRESS,
                        LOAD_INIT,
			
                        // These are for the XMLSocket ActionScript only
                        SOCK_CLOSE,
                        SOCK_CONNECT,
                        SOCK_DATA,
                        SOCK_XML,
			
                        // These are for the XML ActionScript only
                        XML_LOAD,
                        XML_DATA,
			
                        // This is for setInterval
                        TIMER,
			
			EVENT_COUNT
		};

		unsigned char	m_id;
		unsigned char	m_key_code;

		event_id() : m_id(INVALID), m_key_code(key::INVALID) {}

		event_id(id_code id, key::code c = key::INVALID)
			:
			m_id((unsigned char) id),
			m_key_code((unsigned char) c)
		{
			// For the button key events, you must supply a keycode.
			// Otherwise, don't.
			assert((m_key_code == key::INVALID && (m_id != KEY_PRESS))
				|| (m_key_code != key::INVALID && (m_id == KEY_PRESS)));
		}

		bool	operator==(const event_id& id) const { return m_id == id.m_id && m_key_code == id.m_key_code; }

		/// Return the name of a method-handler function
		/// corresponding to this event.
		const tu_string&	get_function_name() const;
	};

	//
	// with_stack_entry
	//

	/// The "with" stack is for Pascal-like with-scoping.
	struct with_stack_entry
	{
		smart_ptr<as_object_interface>	m_object;
		int	m_block_end_pc;
		
		with_stack_entry()
			:
			m_object(NULL),
			m_block_end_pc(0)
		{
		}

		with_stack_entry(as_object_interface* obj, int end)
			:
			m_object(obj),
			m_block_end_pc(end)
		{
		}
	};


	/// Base class for actions.
	struct action_buffer
	{
		action_buffer();

		/// Read action bytes from input stream
		void	read(stream* in);

		/// \brief
		/// Interpret the actions in this action buffer, and evaluate
		/// them in the given environment. 
		//
		/// Execute our whole buffer,
		/// without any arguments passed in.
		///
		void	execute(as_environment* env);

		/// Interpret the specified subset of the actions in our buffer.
		//
		/// Caller is responsible for cleaning up our local
		/// stack frame (it may have passed its arguments in via the
		/// local stack frame).
		void	execute(
			as_environment* env,
			int start_pc,
			int exec_bytes,
			as_value* retval,
			const array<with_stack_entry>& initial_with_stack,
			bool is_function2);

		bool	is_null()
		{
			return m_buffer.size() < 1 || m_buffer[0] == 0;
		}

		int	get_length() const { return m_buffer.size(); }

	private:
		// Don't put these as values in array<>!  They contain
		// internal pointers and cannot be moved or copied.
		// If you need to keep an array of them, keep pointers
		// to new'd instances.
		action_buffer(const action_buffer& a) { assert(0); }
		void operator=(const action_buffer& a) { assert(0); }

		void	process_decl_dict(int start_pc, int stop_pc);

		// data:
		array<unsigned char>	m_buffer;
		array<const char*>	m_dictionary;
		int	m_decl_dict_processed_at;

		void doActionNew(as_environment* env, 
			array<with_stack_entry>& with_stack);

		void doActionInstanceOf(as_environment* env);

		void doActionCallMethod(as_environment* env);

		void doActionCallFunction(as_environment* env,
			array<with_stack_entry>& with_stack);

		void doActionDefineFunction(as_environment* env,
			array<with_stack_entry>& with_stack, int pc, int* next_pc);

		void doActionDefineFunction2(as_environment* env,
			array<with_stack_entry>& with_stack, int pc, int* next_pc);

		void doActionGetMember(as_environment* env);
	};


	struct fn_call;
	typedef void (*as_c_function_ptr)(const fn_call& fn);


	struct as_property_interface
	{
		virtual bool	set_property(int index, const as_value& val) = 0;
	};


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
			as_object_interface*	m_object_value;
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
		as_value(as_object_interface* obj);

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
		as_object_interface*	to_object() const;


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

		/// Make this value an as_object_interface.
		/// Internally adds a reference to the ref-counted as_object_interface.
		void	set_as_object_interface(as_object_interface* obj);

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
			else if (v.m_type == OBJECT) set_as_object_interface(v.m_object_value);
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


// tulrich: I'm not too sure this is useful.  For things like
// xml_as_object, is it sufficient to always store the event handlers
// as ordinary members using their canonical names, instead of this
// special table?  I have a feeling that's what Macromedia does
// (though I'm not sure).
#if 0
	// This class is just as_object_interface, with an event
	// handler table added.
	struct as_object_with_handlers : public as_object_interface
	{
                // ActionScript event handler table.
                hash<event_id, gnash::as_value>        m_event_handlers;

                // ActionScript event handler.
                void    set_event_handler(event_id id, const as_value& method)
                {
                        // m_event_handlers.push_back(as);
                        //m_event_handlers.set(id, method);
                }

                bool    get_event_handler(event_id id, gnash::as_value* result)
                {
                        //return m_event_handlers.get(id, result);
			return false;
                }
	};
#endif // 0


	//
	// as_prop_flags
	//

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

	//
	// as_member
	//

	/// Member for as_object: value + flags
	struct as_member {
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


	/// \brief
	/// A generic bag of attributes. 
	//
	/// Base-class for ActionScript script-defined objects.
	/// This would likely be ActionScript's 'Object' class.
	///
	struct as_object : public as_object_interface
	{
		/// Members of this objects in an hash
		stringi_hash<as_member>	m_members;

		/// Reference to this object's '__proto__'
		as_object_interface*	m_prototype;

		/// Construct an ActionScript object with no prototype associated.
		as_object() : m_prototype(NULL) { }

		/// \brief
		/// Construct an ActionScript object based on the given prototype.
		/// Adds a reference to the prototype, if any.
		as_object(as_object_interface* proto) : m_prototype(proto)
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
	};


	/// ActionScript "environment", essentially VM state?
	struct as_environment
	{
		/// Stack of as_values in this environment
		array<as_value>	m_stack;

		as_value	m_global_register[4];

		/// function2 uses this
		array<as_value>	m_local_register;

		movie*	m_target;

		/// Variables available in this environment
		stringi_hash<as_value>	m_variables;

		/// For local vars.  Use empty names to separate frames.
		struct frame_slot
		{
			tu_string	m_name;
			as_value	m_value;

			frame_slot() {}
			frame_slot(const tu_string& name, const as_value& val) : m_name(name), m_value(val) {}
		};
		array<frame_slot>	m_local_frames;


		as_environment()
			:
			m_target(0)
		{
		}

		movie*	get_target() { return m_target; }
		void	set_target(movie* target) { m_target = target; }

		// stack access/manipulation
		// @@ TODO do more checking on these
		template<class T>
		// stack access/manipulation
		void	push(T val) { push_val(as_value(val)); }
		void	push_val(const as_value& val) { m_stack.push_back(val); }


		/// Pops an as_value off the stack top and return it.
		as_value	pop() { as_value result = m_stack.back(); m_stack.pop_back(); return result; }

		/// Get stack value at the given distance from top.
		//
		/// top(0) is actual stack top
		///
		as_value&	top(int dist) { return m_stack[m_stack.size() - 1 - dist]; }

		/// Get stack value at the given distance from bottom.
		//
		/// bottom(0) is actual stack top
		///
		as_value&	bottom(int index) { return m_stack[index]; }

		/// Drop 'count' values off the top of the stack.
		void	drop(int count) { m_stack.resize(m_stack.size() - count); }

		/// Returns index of top stack element
		int	get_top_index() const { return m_stack.size() - 1; }

		/// Return the (possibly UNDEFINED) value of the named var.
		/// Variable name can contain path elements.
		as_value	get_variable(const tu_string& varname, const array<with_stack_entry>& with_stack) const;

		/// Same of the above, but no support for path.
		as_value	get_variable_raw(const tu_string& varname, const array<with_stack_entry>& with_stack) const;

		/// Given a path to variable, set its value.
		void	set_variable(const tu_string& path, const as_value& val, const array<with_stack_entry>& with_stack);

		/// Same of the above, but no support for path
		void	set_variable_raw(const tu_string& path, const as_value& val, const array<with_stack_entry>& with_stack);

		/// Set/initialize the value of the local variable.
		void	set_local(const tu_string& varname, const as_value& val);
		/// \brief
		/// Add a local var with the given name and value to our
		/// current local frame. 
		///
		/// Use this when you know the var
		/// doesn't exist yet, since it's faster than set_local();
		/// e.g. when setting up args for a function.
		void	add_local(const tu_string& varname, const as_value& val);

		/// Create the specified local var if it doesn't exist already.
		void	declare_local(const tu_string& varname);

		/// Retrieve the named member (variable).
		//
		/// If no member is found under the given name
		/// 'val' is untouched and 'false' is returned.
		/// 
		bool	get_member(const tu_stringi& varname, as_value* val) const;
		void	set_member(const tu_stringi& varname, const as_value& val);

		// Parameter/local stack frame management.
		int	get_local_frame_top() const { return m_local_frames.size(); }
		void	set_local_frame_top(int t) { assert(t <= m_local_frames.size()); m_local_frames.resize(t); }
		void	add_frame_barrier() { m_local_frames.push_back(frame_slot()); }

		// Local registers.
		void	add_local_registers(int register_count)
		{
			m_local_register.resize(m_local_register.size() + register_count);
		}
		void	drop_local_registers(int register_count)
		{
			m_local_register.resize(m_local_register.size() - register_count);
		}
		as_value*	local_register_ptr(int reg);

		// Internal.
		int	find_local(const tu_string& varname) const;
		bool	parse_path(const tu_string& var_path, tu_string* path, tu_string* var) const;
		movie*	find_target(const tu_string& path) const;
		movie*	find_target(const as_value& val) const;

		/// Dump content of the stack using the log_msg function
		void dump_stack()
		{
			for (int i=0, n=m_stack.size(); i<n; i++)
			{
				log_msg("Stack[%d]: %s\n",
					i, m_stack[i].to_string());
			}
		}
	};


	/// Parameters/environment for C functions callable from ActionScript.
	struct fn_call
	{
		as_value* result;
		as_object_interface* this_ptr;
		as_environment* env;
		int nargs;
		int first_arg_bottom_index;

		fn_call(as_value* res_in, as_object_interface* this_in,
				as_environment* env_in,
				int nargs_in, int first_in)
			:
			result(res_in),
			this_ptr(this_in),
			env(env_in),
			nargs(nargs_in),
			first_arg_bottom_index(first_in)
		{
		}

		/// Access a particular argument.
		as_value& arg(int n) const
		{
			assert(n < nargs);
			return env->bottom(first_arg_bottom_index - n);
		}

	};


	//
	// Some handy helpers
	//

	// Clean up any stray heap stuff we've allocated.
	void	action_clear();

	// Dispatching methods from C++.
	as_value	call_method0(const as_value& method, as_environment* env, as_object_interface* this_ptr);
	as_value	call_method1(
		const as_value& method, as_environment* env, as_object_interface* this_ptr,
		const as_value& arg0);
	as_value	call_method2(
		const as_value& method, as_environment* env, as_object_interface* this_ptr,
		const as_value& arg0, const as_value& arg1);
	as_value	call_method3(
		const as_value& method, as_environment* env, as_object_interface* this_ptr,
		const as_value& arg0, const as_value& arg1, const as_value& arg2);


	const char*	call_method_parsed(
		as_environment* env,
		as_object_interface* this_ptr,
		const char* method_name,
		const char* method_arg_fmt,
		va_list args);

	// tulrich: don't use this!  To register a class constructor,
	// just assign the classname to the constructor function.  E.g.:
	//
	// my_movie->set_member("MyClass", as_value(MyClassConstructorFunction));
	// 
	//void register_as_object(const char* object_name, as_c_function_ptr handler);

	/// Numerical indices for standard member names.  Can use this
	/// to help speed up get/set member calls, by using a switch()
	/// instead of nasty string compares.
	enum as_standard_member
	{
		M_INVALID_MEMBER = -1,
		M_X,
		M_Y,
		M_XSCALE,
		M_YSCALE,
		M_CURRENTFRAME,
		M_TOTALFRAMES,
		M_ALPHA,
		M_VISIBLE,
		M_WIDTH,
		M_HEIGHT,
		M_ROTATION,
		M_TARGET,
		M_FRAMESLOADED,
		M_NAME,
		M_DROPTARGET,
		M_URL,
		M_HIGHQUALITY,
		M_FOCUSRECT,
		M_SOUNDBUFTIME,
		M_XMOUSE,
		M_YMOUSE,
		M_PARENT,
		M_TEXT,
		M_TEXTWIDTH,
		M_TEXTCOLOR,
		M_ONLOAD,

		AS_STANDARD_MEMBER_COUNT
	};

	/// Return the standard enum, if the arg names a standard member.
	/// Returns M_INVALID_MEMBER if there's no match.
	as_standard_member	get_standard_member(const tu_stringi& name);

}	// end namespace gnash


#endif // GNASH_ACTION_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
