// action.cpp	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <typeinfo> 
#include <pthread.h> 

#include "action.h"
#include "impl.h"
#include "log.h"
#include "stream.h"
#include "tu_random.h"

#include "gstring.h"
#include "MovieClipLoader.h"
#include "Function.h"
#include "timers.h"
#include "textformat.h"
#include "sound.h"
#include "array.h"
#include "types.h"

#ifdef HAVE_LIBXML
#include "xml.h"
#include "xmlsocket.h"
#endif

#include "Boolean.h"
#include "Camera.h"
#include "Color.h"
#include "ContextMenu.h"
#include "CustomActions.h"
#include "Date.h"
#include "Error.h"
#include "Function.h"
#include "LoadVars.h"
#include "LocalConnection.h"
#include "Microphone.h"
#include "Mouse.h"
#include "NetConnection.h"
#include "NetStream.h"
#include "Selection.h"
#include "SharedObject.h"
#include "Stage.h"
#include "System.h"
#include "TextSnapshot.h"
#include "Video.h"
#include "swf.h"


#ifdef _WIN32
#define snprintf _snprintf
#endif // _WIN32

// NOTES:
//
// Buttons
// on (press)                 onPress
// on (release)               onRelease
// on (releaseOutside)        onReleaseOutside
// on (rollOver)              onRollOver
// on (rollOut)               onRollOut
// on (dragOver)              onDragOver
// on (dragOut)               onDragOut
// on (keyPress"...")         onKeyDown, onKeyUp      <----- IMPORTANT
//
// Sprites
// onClipEvent (load)         onLoad
// onClipEvent (unload)       onUnload                Hm.
// onClipEvent (enterFrame)   onEnterFrame
// onClipEvent (mouseDown)    onMouseDown
// onClipEvent (mouseUp)      onMouseUp
// onClipEvent (mouseMove)    onMouseMove
// onClipEvent (keyDown)      onKeyDown
// onClipEvent (keyUp)        onKeyUp
// onClipEvent (data)         onData

// Text fields have event handlers too!

// Sprite built in methods:
// play()
// stop()
// gotoAndStop()
// gotoAndPlay()
// nextFrame()
// startDrag()
// getURL()
// getBytesLoaded()
// getBytesTotal()

// Built-in functions: (do these actually exist in the VM, or are they just opcodes?)
// Number()
// String()


// TODO builtins
//
// Number.toString() -- takes an optional arg that specifies the base
//
// Boolean() type cast
//
// typeof operator --> "number", "string", "boolean", "object" (also
// for arrays), "null", "movieclip", "function", "undefined"
//
// Number.MAX_VALUE, Number.MIN_VALUE
//
// String.fromCharCode()

namespace gnash {
	
	//
	// action stuff
	//

	void	action_init();

	// Statics.
	bool	s_inited = false;
	smart_ptr<as_object>	s_global;
	fscommand_callback	s_fscommand_handler = NULL;


#define EXTERN_MOVIE
	
#ifdef EXTERN_MOVIE
	void attach_extern_movie(const char* url, const movie* target, const movie* root_movie)
	{
		tu_string infile = get_workdir();
		infile += url;

		movie_definition_sub*	md = create_library_movie_sub(infile.c_str());
		if (md == NULL)
		{
			log_error("can't create movie_definition_sub for %s\n", infile.c_str());
			return;
		}

		gnash::movie_interface* extern_movie;

		if (target == root_movie)
		{
			extern_movie = create_library_movie_inst_sub(md);			
			if (extern_movie == NULL)
			{
				log_error("can't create extern root movie_interface for %s\n", infile.c_str());
				return;
			}
			set_current_root(extern_movie);
			movie* m = extern_movie->get_root_movie();

			m->on_event(event_id::LOAD);
		}
		else
		{
			extern_movie = md->create_instance();
			if (extern_movie == NULL)
			{
				log_error("can't create extern movie_interface for %s\n", infile.c_str());
				return;
			}
      
			save_extern_movie(extern_movie);
      
			character* tar = (character*)target;
			const char* name = tar->get_name();
			Uint16 depth = tar->get_depth();
			bool use_cxform = false;
			cxform color_transform =  tar->get_cxform();
			bool use_matrix = false;
			matrix mat = tar->get_matrix();
			float ratio = tar->get_ratio();
			Uint16 clip_depth = tar->get_clip_depth();

			movie* parent = tar->get_parent();
			movie* new_movie = extern_movie->get_root_movie();

			assert(parent != NULL);

			((character*)new_movie)->set_parent(parent);
       
			parent->replace_display_object(
				(character*) new_movie,
				name,
				depth,
				use_cxform,
				color_transform,
				use_matrix,
				mat,
				ratio,
				clip_depth);
		}
	}
#endif // EXTERN_MOVIE

	void	register_fscommand_callback(fscommand_callback handler)
	// External interface.
	{
		s_fscommand_handler = handler;
	}



	static bool string_to_number(double* result, const char* str)
	// Utility.  Try to convert str to a number.  If successful,
	// put the result in *result, and return true.  If not
	// successful, put 0 in *result, and return false.
	{
		char* tail = 0;
		*result = strtod(str, &tail);
		if (tail == str || *tail != 0)
		{
			// Failed conversion to Number.
			return false;
		}
		return true;
	}


	//
	// Function/method dispatch.
	//


	as_value	call_method(
		const as_value& method,
		as_environment* env,
		as_object_interface* this_ptr, // this is ourself
		int nargs,
		int first_arg_bottom_index)
	// first_arg_bottom_index is the stack index, from the bottom, of the first argument.
	// Subsequent arguments are at *lower* indices.  E.g. if first_arg_bottom_index = 7,
	// then arg1 is at env->bottom(7), arg2 is at env->bottom(6), etc.
	{
		as_value	val;

		as_c_function_ptr	func = method.to_c_function();
		if (func)
		{
			// It's a C function.  Call it.
			(*func)(fn_call(&val, this_ptr, env, nargs, first_arg_bottom_index));
		}
		else if (function_as_object* as_func = method.to_as_function())
		{
			// It's an ActionScript function.  Call it.
			(*as_func)(fn_call(&val, this_ptr, env, nargs, first_arg_bottom_index));
		}
		else
		{
			log_error("error in call_method(): method is not a function\n");
		}

		return val;
	}


	as_value	call_method0(
		const as_value& method,
		as_environment* env,
		as_object_interface* this_ptr)
	{
		return call_method(method, env, this_ptr, 0, env->get_top_index() + 1);
	}
		
	const char*	call_method_parsed(
		as_environment* env,
		as_object_interface* this_ptr,
		const char* method_name,
		const char* method_arg_fmt,
		va_list args)
	// Printf-like vararg interface for calling ActionScript.
	// Handy for external binding.
	{
		log_msg("FIXME(%d): %s\n", __LINE__, __FUNCTION__);

#if 0
		static const int	BUFSIZE = 1000;
		char	buffer[BUFSIZE];
		array<const char*>	tokens;

		// Brutal crap parsing.  Basically null out any
		// delimiter characters, so that the method name and
		// args sit in the buffer as null-terminated C
		// strings.  Leave an intial ' character as the first
		// char in a string argument.
		// Don't verify parens or matching quotes or anything.
		{
			strncpy(buffer, method_call, BUFSIZE);
			buffer[BUFSIZE - 1] = 0;
			char*	p = buffer;

			char	in_quote = 0;
			bool	in_arg = false;
			for (;; p++)
			{
				char	c = *p;
				if (c == 0)
				{
					// End of string.
					break;
				}
				else if (c == in_quote)
				{
					// End of quotation.
					assert(in_arg);
					*p = 0;
					in_quote = 0;
					in_arg = false;
				}
				else if (in_arg)
				{
					if (in_quote == 0)
					{
						if (c == ')' || c == '(' || c == ',' || c == ' ')
						{
							// End of arg.
							*p = 0;
							in_arg = false;
						}
					}
				}
				else
				{
					// Not in arg.  Watch for start of arg.
					assert(in_quote == 0);
					if (c == '\'' || c == '\"')
					{
						// Start of quote.
						in_quote = c;
						in_arg = true;
						*p = '\'';	// ' at the start of the arg, so later we know this is a string.
						tokens.push_back(p);
					}
					else if (c == ' ' || c == ',')
					{
						// Non-arg junk, null it out.
						*p = 0;
					}
					else
					{
						// Must be the start of a numeric arg.
						in_arg = true;
						tokens.push_back(p);
					}
				}
			}
		}
#endif // 0


		// Parse va_list args
		int	starting_index = env->get_top_index();
		const char* p = method_arg_fmt;
		for (;; p++)
		{
			char	c = *p;
			if (c == 0)
			{
				// End of args.
				break;
			}
			else if (c == '%')
			{
				p++;
				c = *p;
				// Here's an arg.
				if (c == 'd')
				{
					// Integer.
					env->push(va_arg(args, int));
				}
				else if (c == 'f')
				{
					// Double
					env->push(va_arg(args, double));
				}
				else if (c == 's')
				{
					// String
					env->push(va_arg(args, const char *));
				}
				else if (c == 'l')
				{
					p++;
					c = *p;
					if (c == 's')
					{
						// Wide string.
						env->push(va_arg(args, const wchar_t *));
					}
					else
					{
						log_error("call_method_parsed('%s','%s') -- invalid fmt '%%l%c'\n",
							  method_name,
							  method_arg_fmt,
							  c);
					}
				}
				else
				{
					// Invalid fmt, warn.
					log_error("call_method_parsed('%s','%s') -- invalid fmt '%%%c'\n",
						  method_name,
						  method_arg_fmt,
						  c);
				}
			}
			else
			{
				// Ignore whitespace and commas.
				if (c == ' ' || c == '\t' || c == ',')
				{
					// OK
				}
				else
				{
					// Invalid arg; warn.
					log_error("call_method_parsed('%s','%s') -- invalid char '%c'\n",
						  method_name,
						  method_arg_fmt,
						  c);
				}
			}
		}

		array<with_stack_entry>	dummy_with_stack;
		as_value	method = env->get_variable(method_name, dummy_with_stack);

		// check method

		// Reverse the order of pushed args
		int	nargs = env->get_top_index() - starting_index;
		for (int i = 0; i < (nargs >> 1); i++)
		{
			int	i0 = starting_index + 1 + i;
			int	i1 = starting_index + nargs - i;
			assert(i0 < i1);

			swap(&(env->bottom(i0)), &(env->bottom(i1)));
		}

		// Do the call.
		as_value	result = call_method(method, env, this_ptr, nargs, env->get_top_index());
		env->drop(nargs);

		// Return pointer to static string for return value.
		static tu_string	s_retval;
		s_retval = result.to_tu_string();
		return s_retval.c_str();
	}


	//
	// sound object
	//

	struct sound_as_object : public as_object
	{
		tu_string sound;
		int sound_id;
	};

	void	movie_load()
	{
		IF_VERBOSE_ACTION(log_msg("-- start movie \n"));
	}

	void	sound_start(const fn_call& fn)
	{
		IF_VERBOSE_ACTION(log_msg("-- start sound \n"));
		sound_handler* s = get_sound_handler();
		if (s != NULL)
		{
			sound_as_object*	so = (sound_as_object*) (as_object*) fn.this_ptr;
			assert(so);
			s->play_sound(so->sound_id, 0);
		}
	}


	void	sound_stop(const fn_call& fn)
	{
		IF_VERBOSE_ACTION(log_msg("-- stop sound \n"));
		sound_handler* s = get_sound_handler();
		if (s != NULL)
		{
			sound_as_object*	so = (sound_as_object*) (as_object*) fn.this_ptr;
			assert(so);
			s->stop_sound(so->sound_id);
		}
	}

	void	sound_attach(const fn_call& fn)
	{
		IF_VERBOSE_ACTION(log_msg("-- attach sound \n"));
		if (fn.nargs < 1)
		{
			log_error("attach sound needs one argument\n");
			return;
		}

		sound_as_object*	so = (sound_as_object*) (as_object*) fn.this_ptr;
		assert(so);

		so->sound = fn.arg(0).to_tu_string();

		// check the import.
		movie_definition_sub*	def = (movie_definition_sub*)
			fn.env->get_target()->get_root_movie()->get_movie_definition();
		assert(def);
		smart_ptr<resource> res = def->get_exported_resource(so->sound);
		if (res == NULL)
		{
			log_error("import error: resource '%s' is not exported\n", so->sound.c_str());
			return;
		}

		int si = 0;
		sound_sample_impl* ss = (sound_sample_impl*) res->cast_to_sound_sample();

		if (ss != NULL)
		{
			si = ss->m_sound_handler_id;
		}
		else
		{
			log_error("sound sample is NULL\n");
			return;
		}

		// sanity check
		assert(si >= 0 && si < 1000);
		so->sound_id = si;
	}

	//
	// Built-in objects
	//


	//
	// math object
	//


#if 0
	// One-argument simple functions.
	#define MATH_WRAP_FUNC1(funcname)							\
	void	math_##funcname(as_value* result, as_object_interface* this_ptr,		\
				as_environment* env, int nargs, int first_arg_bottom_index)	\
	{											\
		double	arg = env->bottom(first_arg_bottom_index).to_number();			\
		result->set_double(funcname(arg));						\
	}
#else
	// One-argument simple functions.
	#define MATH_WRAP_FUNC1(funcname)							\
	void	math_##funcname(const fn_call& fn)						\
	{											\
		double	arg = fn.arg(0).to_number();						\
		fn.result->set_double(funcname(arg));						\
	}
#endif

	MATH_WRAP_FUNC1(fabs);
	MATH_WRAP_FUNC1(acos);
	MATH_WRAP_FUNC1(asin);
	MATH_WRAP_FUNC1(atan);
	MATH_WRAP_FUNC1(ceil);
	MATH_WRAP_FUNC1(cos);
	MATH_WRAP_FUNC1(exp);
	MATH_WRAP_FUNC1(floor);
	MATH_WRAP_FUNC1(log);
	MATH_WRAP_FUNC1(sin);
	MATH_WRAP_FUNC1(sqrt);
	MATH_WRAP_FUNC1(tan);

#if 0
	// Two-argument functions.
	#define MATH_WRAP_FUNC2_EXP(funcname, expr)										\
	void	math_##funcname(as_value* result, as_object_interface* this_ptr, as_environment* env, int nargs, int first_arg_bottom_index)	\
	{															\
		double	arg0 = env->bottom(first_arg_bottom_index).to_number();							\
		double	arg1 = env->bottom(first_arg_bottom_index - 1).to_number();						\
		result->set_double(expr);											\
	}
#else
	// Two-argument functions.
	#define MATH_WRAP_FUNC2_EXP(funcname, expr)										\
	void	math_##funcname(const fn_call& fn)										\
	{															\
		double	arg0 = fn.arg(0).to_number();										\
		double	arg1 = fn.arg(1).to_number();										\
		fn.result->set_double(expr);											\
	}
#endif
	MATH_WRAP_FUNC2_EXP(atan2, (atan2(arg0, arg1)));
	MATH_WRAP_FUNC2_EXP(max, (arg0 > arg1 ? arg0 : arg1));
	MATH_WRAP_FUNC2_EXP(min, (arg0 < arg1 ? arg0 : arg1));
	MATH_WRAP_FUNC2_EXP(pow, (pow(arg0, arg1)));

	// A couple of oddballs.
	void	math_random(const fn_call& fn)
	{
		// Random number between 0 and 1.
		fn.result->set_double(tu_random::next_random() / double(Uint32(0x0FFFFFFFF)));
	}
	void	math_round(const fn_call& fn)
	{
		// round argument to nearest int.
		double	arg0 = fn.arg(0).to_number();
		fn.result->set_double(floor(arg0 + 0.5));
	}
	
	void math_init()
	{
		// Create built-in math object.
		as_object*	math_obj = new as_object;

		// constant
		math_obj->set_member("e", 2.7182818284590452354);
		math_obj->set_member("ln2", 0.69314718055994530942);
		math_obj->set_member("log2e", 1.4426950408889634074);
		math_obj->set_member("ln10", 2.30258509299404568402);
		math_obj->set_member("log10e", 0.43429448190325182765);
		math_obj->set_member("pi", 3.14159265358979323846);
		math_obj->set_member("sqrt1_2", 0.7071067811865475244);
		math_obj->set_member("sqrt2", 1.4142135623730950488);

		// math methods
		math_obj->set_member("abs", &math_fabs);
		math_obj->set_member("acos", &math_acos);
		math_obj->set_member("asin", &math_asin);
		math_obj->set_member("atan", &math_atan);
		math_obj->set_member("ceil", &math_ceil);
		math_obj->set_member("cos", &math_cos);
		math_obj->set_member("exp", &math_exp);
		math_obj->set_member("floor", &math_floor);
		math_obj->set_member("log", &math_log);
		math_obj->set_member("random", &math_random);
		math_obj->set_member("round", &math_round);
		math_obj->set_member("sin", &math_sin);
		math_obj->set_member("sqrt", &math_sqrt);
		math_obj->set_member("tan", &math_tan);

		math_obj->set_member("atan2", &math_atan2);
		math_obj->set_member("max", &math_max);
		math_obj->set_member("min", &math_min);
		math_obj->set_member("pow", &math_pow);

		s_global->set_member("math", math_obj);
	}
		
	void event_test(const fn_call& fn)
	{
		log_msg("FIXME: %s\n", __FUNCTION__);
	}
	
	//
	// key object
	//


	struct key_as_object : public as_object
	{
		Uint8	m_keymap[key::KEYCOUNT / 8 + 1];	// bit-array
		array<weak_ptr<as_object_interface> >	m_listeners;
		int	m_last_key_pressed;

		key_as_object()
			:
			m_last_key_pressed(0)
		{
			memset(m_keymap, 0, sizeof(m_keymap));
		}

		bool	is_key_down(int code)
		{
			if (code < 0 || code >= key::KEYCOUNT) return false;

			int	byte_index = code >> 3;
			int	bit_index = code - (byte_index << 3);
			int	mask = 1 << bit_index;

			assert(byte_index >= 0 && byte_index < int(sizeof(m_keymap)/sizeof(m_keymap[0])));

			if (m_keymap[byte_index] & mask)
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		void	set_key_down(int code)
		{
			if (code < 0 || code >= key::KEYCOUNT) return;

			m_last_key_pressed = code;

			int	byte_index = code >> 3;
			int	bit_index = code - (byte_index << 3);
			int	mask = 1 << bit_index;

			assert(byte_index >= 0 && byte_index < int(sizeof(m_keymap)/sizeof(m_keymap[0])));

			m_keymap[byte_index] |= mask;

			// Notify listeners.
			int i;
			int n = m_listeners.size();
			for (i = 0; i < n; i++)
			{
				smart_ptr<as_object_interface>	listener = m_listeners[i];
				as_value	method;
				if (listener != NULL
				    && listener->get_member(event_id(event_id::KEY_DOWN).get_function_name(), &method))
				{
					call_method(method, NULL /* or root? */, listener.get_ptr(), 0, 0);
				}
			}
		}

		void	set_key_up(int code)
		{
			if (code < 0 || code >= key::KEYCOUNT) return;

			int	byte_index = code >> 3;
			int	bit_index = code - (byte_index << 3);
			int	mask = 1 << bit_index;

			assert(byte_index >= 0 && byte_index < int(sizeof(m_keymap)/sizeof(m_keymap[0])));

			m_keymap[byte_index] &= ~mask;

			// Notify listeners.
			for (int i = 0, n = m_listeners.size(); i < n; i++)
			{
				smart_ptr<as_object_interface>	listener = m_listeners[i];

				as_value	method;
				if (listener != NULL
				    && listener->get_member(event_id(event_id::KEY_UP).get_function_name(), &method))
				{
					call_method(method, NULL /* or root? */, listener.get_ptr(), 0, 0);
				}
			}
		}

		void	cleanup_listeners()
		// Remove dead entries in the listeners list.  (Since
		// we use weak_ptr's, listeners can disappear without
		// notice.)
		{
			for (int i = m_listeners.size() - 1; i >= 0; i--)
			{
				if (m_listeners[i] == NULL)
				{
					m_listeners.erase(m_listeners.begin() + i);
				}
			}
		}

		void	add_listener(as_object_interface* listener)
		{
			cleanup_listeners();

			for (int i = 0, n = m_listeners.size(); i < n; i++)
			{
				if (m_listeners[i] == listener)
				{
					// Already in the list.
					return;
				}
			}

			m_listeners.push_back(listener);
		}

		void	remove_listener(as_object_interface* listener)
		{
			cleanup_listeners();

			for (int i = m_listeners.size() - 1; i >= 0; i--)
			{
				if (m_listeners[i] == listener)
				{
					m_listeners.erase(m_listeners.begin() + i);
				}
			}
		}

		int	get_last_key_pressed() const { return m_last_key_pressed; }
	};


	void	key_add_listener(const fn_call& fn)
	// Add a listener (first arg is object reference) to our list.
	// Listeners will have "onKeyDown" and "onKeyUp" methods
	// called on them when a key changes state.
	{
		if (fn.nargs < 1)
		{
			log_error("key_add_listener needs one argument (the listener object)\n");
			return;
		}

		as_object_interface*	listener = fn.arg(0).to_object();
		if (listener == NULL)
		{
			log_error("key_add_listener passed a NULL object; ignored\n");
			return;
		}

		key_as_object*	ko = (key_as_object*) (as_object*) fn.this_ptr;
		assert(ko);

		ko->add_listener(listener);
	}

	void	key_get_ascii(const fn_call& fn)
	// Return the ascii value of the last key pressed.
	{
		key_as_object*	ko = (key_as_object*) (as_object*) fn.this_ptr;
		assert(ko);

		fn.result->set_undefined();

		int	code = ko->get_last_key_pressed();
		if (code > 0)
		{
			// @@ Crude for now; just jamming the key code in a string, as a character.
			// Need to apply shift/capslock/numlock, etc...
			char	buf[2];
			buf[0] = (char) code;
			buf[1] = 0;

			fn.result->set_string(buf);
		}
	}

	void	key_get_code(const fn_call& fn)
	// Returns the keycode of the last key pressed.
	{
		key_as_object*	ko = (key_as_object*) (as_object*) fn.this_ptr;
		assert(ko);

		fn.result->set_int(ko->get_last_key_pressed());
	}

	void	key_is_down(const fn_call& fn)
	// Return true if the specified (first arg keycode) key is pressed.
	{
		if (fn.nargs < 1)
		{
			log_error("key_is_down needs one argument (the key code)\n");
			return;
		}

		int	code = (int) fn.arg(0).to_number();

		key_as_object*	ko = (key_as_object*) (as_object*) fn.this_ptr;
		assert(ko);

		fn.result->set_bool(ko->is_key_down(code));
	}

	void	key_is_toggled(const fn_call& fn)
	// Given the keycode of NUM_LOCK or CAPSLOCK, returns true if
	// the associated state is on.
	{
		// @@ TODO
		fn.result->set_bool(false);
	}

	void	key_remove_listener(const fn_call& fn)
	// Remove a previously-added listener.
	{
		if (fn.nargs < 1)
		{
			log_error("key_remove_listener needs one argument (the listener object)\n");
			return;
		}

		as_object_interface*	listener = fn.arg(0).to_object();
		if (listener == NULL)
		{
			log_error("key_remove_listener passed a NULL object; ignored\n");
			return;
		}

		key_as_object*	ko = (key_as_object*) (as_object*) fn.this_ptr;
		assert(ko);

		ko->remove_listener(listener);
	}


	void key_init()
	{
		// Create built-in key object.
		as_object*	key_obj = new key_as_object;

		// constants
#define KEY_CONST(k) key_obj->set_member(#k, key::k)
		KEY_CONST(BACKSPACE);
		KEY_CONST(CAPSLOCK);
		KEY_CONST(CONTROL);
		KEY_CONST(DELETEKEY);
		KEY_CONST(DOWN);
		KEY_CONST(END);
		KEY_CONST(ENTER);
		KEY_CONST(ESCAPE);
		KEY_CONST(HOME);
		KEY_CONST(INSERT);
		KEY_CONST(LEFT);
		KEY_CONST(PGDN);
		KEY_CONST(PGUP);
		KEY_CONST(RIGHT);
		KEY_CONST(SHIFT);
		KEY_CONST(SPACE);
		KEY_CONST(TAB);
		KEY_CONST(UP);

		// methods
		key_obj->set_member("addListener", &key_add_listener);
		key_obj->set_member("getAscii", &key_get_ascii);
		key_obj->set_member("getCode", &key_get_code);
		key_obj->set_member("isDown", &key_is_down);
		key_obj->set_member("isToggled", &key_is_toggled);
		key_obj->set_member("removeListener", &key_remove_listener);

		s_global->set_member("Key", key_obj);
	}


	void	notify_key_event(key::code k, bool down)
	// External interface for the host to report key events.
	{
		action_init();	// @@ put this in some global init somewhere else...

		static tu_string	key_obj_name("Key");

		as_value	kval;
		s_global->get_member(key_obj_name, &kval);
		if (kval.get_type() == as_value::OBJECT)
		{
			key_as_object*	ko = (key_as_object*) kval.to_object();
			assert(ko);

			if (down) ko->set_key_down(k);
			else ko->set_key_up(k);
		}
		else
		{
			log_error("gnash::notify_key_event(): no Key built-in\n");
		}
	}
	

	//
	// global init
	//


	void	as_global_trace(const fn_call& fn)
	{
		assert(fn.nargs >= 1);

		// Special case for objects: try the toString() method.
		if (fn.arg(0).get_type() == as_value::OBJECT)
		{
			as_object_interface* obj = fn.arg(0).to_object();
			assert(obj);

			as_value method;
			if (obj->get_member("toString", &method)
			    && method.is_function())
			{
				as_value result = call_method0(method, fn.env, obj);
				log_msg("%s\n", result.to_string());

				return;
			}
		}

		// Log our argument.
		//
		// @@ what if we get extra args?
		//
		// @@ Array gets special treatment.
		const char* arg0 = fn.arg(0).to_string();
		log_msg("%s\n", arg0);
	}


	void	as_global_sound_ctor(const fn_call& fn)
	// Constructor for ActionScript class Sound.
	{
		smart_ptr<as_object>	sound_obj(new sound_as_object);

		// methods
		sound_obj->set_member("attachSound", &sound_attach);
		sound_obj->set_member("start", &sound_start);
		sound_obj->set_member("stop", &sound_stop);

		fn.result->set_as_object_interface(sound_obj.get_ptr());
	}


	void	as_global_object_ctor(const fn_call& fn)
	// Constructor for ActionScript class Object.
	{
		as_object *new_obj;

		if ( fn.nargs == 0 )
		{
			new_obj = new as_object();
		}
		else if ( fn.nargs == 1 ) // copy constructor
		{
			as_object_interface *src_obj = fn.arg(0).to_object();
			new_obj = new as_object(src_obj);
		}
		else
		{
			IF_VERBOSE_DEBUG(log_msg("Too many args to Object constructor"));
			new_obj = new as_object();
		}

		fn.result->set_as_object_interface(new_obj);
	}

	void as_global_isnan(const fn_call& fn)
	{
		assert(fn.nargs == 1);

		fn.result->set_bool(fn.arg(0).is_nan());
	}

	void as_global_isfinite(const fn_call& fn)
	{
		assert(fn.nargs == 1);

		fn.result->set_bool(!fn.arg(0).is_finite());
	}

	void as_global_unescape(const fn_call& fn)
	{
		assert(fn.nargs == 1);

		std::string input = fn.arg(0).to_string();
		std::string insertst;
		int hexcode;

		for (unsigned int i=0;i<input.length();)
		{
			if ((input.length() > i + 2) && input[i] == '%' &&
				isxdigit(input[i+1]) && isxdigit(input[i+2]))
			{
				input[i+1] = toupper(input[i+1]);
				input[i+2] = toupper(input[i+2]);
				if (isdigit(input[i+1]))
					hexcode = (input[i+1] - '0') * 16;
				else
					hexcode = (input[i+1] - 'A' + 10) * 16;

				if (isdigit(input[i+2]))
					hexcode += (input[i+2] - '0');
				else
					hexcode += (input[i+2] - 'A' + 10);

				input.erase(i,3);
				
				switch (hexcode)
				{
				case 0x20: // space
					insertst = ' ';
					break;
				case 0x22: // "
					insertst = '\"';
					break;
				case 0x23: // #
					insertst = '#';
					break;
				case 0x24: // $
					insertst = '$';
					break;
				case 0x25: // %
					insertst = '%';
					break;
				case 0x26: // &
					insertst = '&';
					break;
				case 0x2B: // +
					insertst = '+';
					break;
				case 0x2C: // ,
					insertst = ',';
					break;
				case 0x2F: // /
					insertst = '/';
					break;
				case 0x3A: // :
					insertst = ':';
					break;
				case 0x3B: // ;
					insertst = ';';
					break;
				case 0x3C: // <
					insertst = '<';
					break;
				case 0x3D: // =
					insertst = '=';
					break;
				case 0x3E: // >
					insertst = '>';
					break;
				case 0x3F: // ?
					insertst = '?';
					break;
				case 0x40: // @
					insertst = '@';
					break;
				case 0x5B: // [
					insertst = '[';
					break;
				case 0x5C: // \ (backslash)
					insertst = '\\';
					break;
				case 0x5D: // ]
					insertst = ']';
					break;
				case 0x5E: // ^
					insertst = '^';
					break;
				case 0x60: // `
					insertst = '`';
					break;
				case 0x7B: // {
					insertst = '{';
					break;
				case 0x7C: // |
					insertst = '|';
					break;
				case 0x7D: // }
					insertst = '}';
					break;
				case 0x7E: // ~
					insertst = '~';
					break;
				default:
					IF_VERBOSE_ACTION(log_error("unescape() function reached "
						"unknown hexcode %d, aborting unescape()\n",hexcode));
					fn.result->set_string(fn.arg(0).to_string());
					return;
				}
				input.insert(i,insertst);
			}
			else
				i++;
		}
		fn.result->set_string(input.c_str());
	}

	void as_global_parsefloat(const fn_call& fn)
	{
		assert(fn.nargs == 1);

		float result;

		// sscanf will handle the whitespace / unneeded characters etc. automatically
		if (1 == sscanf(fn.arg(0).to_string(), "%f", &result))
			fn.result->set_double(double(result));
		else
			// if sscanf didn't find anything, return NaN
			fn.result->set_nan();
	}

	void	as_global_parseint(const fn_call& fn)
	{
		assert(fn.nargs == 2 || fn.nargs == 1);

		// Make sure our argument is the correct type
		if (fn.nargs > 1)
			fn.arg(1).convert_to_number();

		// Set up some variables
		const std::string digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		char *input = new char[strlen(fn.arg(0).to_string())+1];
		strcpy(input,fn.arg(0).to_string());
		int base, result = 0, i;
		bool bNegative;

		// Skip leading whitespace
		while (0 != isblank(input[0]))
			input++;

		if (input[0] == '-')
		{
			bNegative = true;
			input++;
		}
		else
			bNegative = false;

		// Convert the string to uppercase
		for (i=0;i<int(strlen(input));i++)
			input[i] = toupper(input[i]);

		// if we were sent a second argument, that's our base
		if (fn.nargs > 1)
		{
			base = int(fn.arg(1).to_number());
		}
		// if the string starts with "0x" then a hex digit
		else if (strlen(input) > 2 && input[0] == '0' && input[1] == 'X'
			&& (isdigit(input[2]) || (input[2] >= 'A' && input[2] <= 'F')))
		{
			base = 16;	// the base is 16
			input = input + 2; // skip the leading "0x"
		}
		// if the string starts with "0" then an octal digit
		else if (strlen(input) > 1 && input[0] == '0' &&
			(input[1] >= '0' && input[1] <= '7'))
		{
			base = 8;
			input++; // skip the leading '0'
		}
		else
			// default base is 10
			base = 10;

		assert (base >= 2 && base <= 36);

		int numdigits = 0;

		// Start at the beginning, see how many valid digits we have
		// in the base we're dealing with
		while (numdigits < int(strlen(input)) 
			&& int(digits.find(input[numdigits])) < base
			&& digits.find(input[numdigits]) != std::string::npos)
			numdigits++;

		// If we didn't get any digits, we should return NaN
		if (numdigits == 0)
		{
			fn.result->set_nan();
			return;
		}

		for (i=0;i<numdigits;i++)
		{
			result += digits.find(input[i])*int(pow(base,numdigits - i - 1));
		}

		if (bNegative)
			result = -result;

		// Now return the parsed string
		fn.result->set_int(result);
	}

	void	as_global_assetpropflags(const fn_call& fn)
	// ASSetPropFlags function
	{
		const int version = fn.env->get_target()->get_movie_definition()->get_version();

		// Check the arguments
		assert(fn.nargs == 3 || fn.nargs == 4);
		assert((version == 5) ? (fn.nargs == 3) : true);

		// object
		as_object_interface* const obj = fn.arg(0).to_object();
		assert(obj != NULL);

		// list of child names
		as_object_interface* props = fn.arg(1).to_object();
		if (props == NULL) {
			// tulrich: this fires in test_ASSetPropFlags -- is it correct?
			assert(fn.arg(1).get_type() == as_value::NULLTYPE);
		}

		// a number which represents three bitwise flags which
		// are used to determine whether the list of child names should be hidden,
		// un-hidden, protected from over-write, un-protected from over-write,
		// protected from deletion and un-protected from deletion
		int set_true = int(fn.arg(2).to_number()) & as_prop_flags::as_prop_flags_mask;

		// Is another integer bitmask that works like set_true,
		// except it sets the attributes to false. The
		// set_false bitmask is applied before set_true is applied

		// ASSetPropFlags was exposed in Flash 5, however the fourth argument 'set_false'
		// was not required as it always defaulted to the value '~0'. 
		int set_false = (fn.nargs == 3 ? 
				 (version == 5 ? ~0 : 0) : int(fn.arg(3).to_number()))
			& as_prop_flags::as_prop_flags_mask;

		// Evan: it seems that if set_true == 0 and set_false == 0, this function
		// acts as if the parameters where (object, null, 0x1, 0) ...
		if (set_false == 0 && set_true == 0)
		{
			props = NULL;
			set_false = 0;
			set_true = 0x1;
		}

		if (props == NULL)
		{
			// Take all the members of the object

			as_object* object = (as_object*) obj;

			stringi_hash<as_member>::const_iterator it = object->m_members.begin();
			while (it != object->m_members.end())
			{
				as_member member = it.get_value();

				as_prop_flags f = member.get_member_flags();
				//const int oldflags = 
				f.get_flags();
				//const int newflags =
				f.set_flags(set_true, set_false);
				member.set_member_flags(f);

				object->m_members.set(it.get_key(), member);

				++it;
			}

			if (object->m_prototype != NULL)
			{
				const as_object* prototype = (as_object*) object->m_prototype;

				it = prototype->m_members.begin();
				while (it != prototype->m_members.end())
				{
					as_member member = it.get_value();

					as_prop_flags f = member.get_member_flags();
					//const int oldflags =
					f.get_flags();
					//const int newflags = 
					f.set_flags(set_true, set_false);
					member.set_member_flags(f);

					object->m_members.set(it.get_key(), member);

					++it;
				}
			}
		} else {
			as_object* object = (as_object*) obj;
			as_object* object_props = (as_object*) props;

			stringi_hash<as_member>::iterator it = object_props->m_members.begin();
			while(it != object_props->m_members.end())
			{
				const tu_stringi key = (it.get_value()).get_member_value().to_string();
				stringi_hash<as_member>::iterator it2 = object->m_members.find(key);

				if (it2 != object->m_members.end())
				{
					as_member member = it2.get_value();

					as_prop_flags f = member.get_member_flags();
					//const int oldflags =
					f.get_flags();
					//const int newflags =
					f.set_flags(set_true, set_false);
					member.set_member_flags(f);

					object->m_members.set((it.get_value()).get_member_value().to_string(), member);
				}

				++it;
			}
		}
	}



	void	action_init()
	// Create/hook built-ins.
	{
		if (s_inited == false)
		{
			s_inited = true;

			// @@ s_global should really be a
			// client-visible player object, which
			// contains one or more actual movie
			// instances.  We're currently just hacking it
			// in as an app-global mutable object :(
			assert(s_global == NULL);
			s_global = new as_object;
			s_global->set_member("trace", as_value(as_global_trace));
			s_global->set_member("Object", as_value(as_global_object_ctor));
			s_global->set_member("Sound", as_value(as_global_sound_ctor));
			s_global->set_member("Array", as_value(as_global_array_ctor));

			s_global->set_member("TextFormat", as_value(textformat_new));
#ifdef HAVE_LIBXML
			s_global->set_member("XML", as_value(xml_new));
			s_global->set_member("XMLNode", as_value(xmlnode_new));
			//s_global->set_member("XML", as_value(xmlsocket_xml_new));
			s_global->set_member("XMLSocket", as_value(xmlsocket_new));
#endif // HAVE_LIBXML
			s_global->set_member("MovieClipLoader", as_value(moviecliploader_new));
			s_global->set_member("String", as_value(string_ctor));
			// This next set are all the unimplemented classes whose
			// code was machine generated.
			s_global->set_member("Boolean", as_value(boolean_new));
			s_global->set_member("Camera", as_value(camera_new));
			s_global->set_member("Color", as_value(color_new));
			s_global->set_member("ContextMenu", as_value(contextmenu_new));
			s_global->set_member("CustomActions", as_value(customactions_new));
			s_global->set_member("Date", as_value(date_new));
			s_global->set_member("Error", as_value(error_new));
			s_global->set_member("Function", as_value(function_new));
			s_global->set_member("LoadVars", as_value(loadvars_new));
			s_global->set_member("LocalConnection", as_value(localconnection_new));
			s_global->set_member("Microphone", as_value(microphone_new));
			s_global->set_member("Mouse", as_value(mouse_new));
			s_global->set_member("NetConnection", as_value(netconnection_new));
			s_global->set_member("NetStream", as_value(netstream_new));
			s_global->set_member("Selection", as_value(selection_new));
			s_global->set_member("SharedObject", as_value(sharedobject_new));
			s_global->set_member("Stage", as_value(stage_new));
			s_global->set_member("System", as_value(system_new));
			s_global->set_member("TextSnapshot", as_value(textsnapshot_new));
			s_global->set_member("Video", as_value(video_new));
			// ASSetPropFlags
			s_global->set_member("ASSetPropFlags", as_global_assetpropflags);
			// unescape
			s_global->set_member("unescape", as_global_unescape);
			// parseFloat
			s_global->set_member("parseFloat", as_global_parsefloat);
			// parseInt
			s_global->set_member("parseInt", as_global_parseint);
			// isNan
			s_global->set_member("isNan", as_global_isnan);
			// isFinite
			s_global->set_member("isFinite", as_global_isfinite);

			math_init();
			key_init();
		}
	}


	void	action_clear()
	{
		if (s_inited)
		{
			s_inited = false;

			s_global->clear();
			s_global = NULL;
		}
	}


	//
	// properties by number
	//

	static const tu_string	s_property_names[] =
	{
		tu_string("_x"),
		tu_string("_y"),
		tu_string("_xscale"),
		tu_string("_yscale"),
		tu_string("_currentframe"),
		tu_string("_totalframes"),
		tu_string("_alpha"),
		tu_string("_visible"),
		tu_string("_width"),
		tu_string("_height"),
		tu_string("_rotation"),
		tu_string("_target"),
		tu_string("_framesloaded"),
		tu_string("_name"),
		tu_string("_droptarget"),
		tu_string("_url"),
		tu_string("_highquality"),
		tu_string("_focusrect"),
		tu_string("_soundbuftime"),
		tu_string("@@ mystery quality member"),
		tu_string("_xmouse"),
		tu_string("_ymouse"),
	};


	static as_value	get_property(as_object_interface* obj, int prop_number)
	{
		as_value	val;
		if (prop_number >= 0 && prop_number < int(sizeof(s_property_names)/sizeof(s_property_names[0])))
		{
			obj->get_member(s_property_names[prop_number], &val);
		}
		else
		{
			log_error("error: invalid property query, property number %d\n", prop_number);
		}
		return val;
	}

	static void	set_property(as_object_interface* obj, int prop_number, const as_value& val)
	{
		if (prop_number >= 0 && prop_number < int(sizeof(s_property_names)/sizeof(s_property_names[0])))
		{
			obj->set_member(s_property_names[prop_number], val);
		}
		else
		{
			log_error("error: invalid set_property, property number %d\n", prop_number);
		}
	}


	//
	// do_action
	//


	/// Thin wrapper around action_buffer.
	struct do_action : public execute_tag
	{
		action_buffer	m_buf;

		void	read(stream* in)
		{
			m_buf.read(in);
		}

		virtual void	execute(movie* m)
		{
			m->add_action_buffer(&m_buf);
		}

		// Don't override because actions should not be replayed when seeking the movie.
		//void	execute_state(movie* m) {}

		virtual bool	is_action_tag() const
		// Tell the caller that we are an action tag.
		{
			return true;
		}
	};

	void	do_action_loader(stream* in, int tag_type, movie_definition_sub* m)
	{
		IF_VERBOSE_PARSE(log_msg("tag %d: do_action_loader\n", tag_type));

		IF_VERBOSE_ACTION(log_msg("-- actions in frame %d\n", m->get_loading_frame()));

		assert(in);
		assert(tag_type == 12);
		assert(m);
		
		do_action*	da = new do_action;
		da->read(in);

		m->add_execute_tag(da);
	}

	
	//
	// do_init_action
	//


	void	do_init_action_loader(stream* in, int tag_type, movie_definition_sub* m)
	{
		assert(tag_type == 59);

		int	sprite_character_id = in->read_u16();

		IF_VERBOSE_PARSE(log_msg("  tag %d: do_init_action_loader\n", tag_type));
		IF_VERBOSE_ACTION(log_msg("  -- init actions for sprite %d\n", sprite_character_id));

		do_action*	da = new do_action;
		da->read(in);
		m->add_init_action(sprite_character_id, da);
	}


	//
	// action_buffer
	//

	// Disassemble one instruction to the log.
	static void	log_disasm(const unsigned char* instruction_data);

	action_buffer::action_buffer()
		:
		m_decl_dict_processed_at(-1)
	{
	}


	void	action_buffer::read(stream* in)
	{
		// Read action bytes.
		for (;;)
		{
			int	instruction_start = m_buffer.size();

			int	pc = m_buffer.size();

			int	action_id = in->read_u8();
			m_buffer.push_back(action_id);

			if (action_id & 0x80)
			{
				// Action contains extra data.  Read it.
				int	length = in->read_u16();
				m_buffer.push_back(length & 0x0FF);
				m_buffer.push_back((length >> 8) & 0x0FF);
				for (int i = 0; i < length; i++)
				{
					unsigned char	b = in->read_u8();
					m_buffer.push_back(b);
				}
			}

			IF_VERBOSE_ACTION(log_msg("%4d\t", pc); log_disasm(&m_buffer[instruction_start]); );

			if (action_id == 0)
			{
				// end of action buffer.
				break;
			}
		}
	}


	/*private*/
	void	action_buffer::process_decl_dict(int start_pc, int stop_pc)
	// Interpret the decl_dict opcode.  Don't read stop_pc or
	// later.  A dictionary is some static strings embedded in the
	// action buffer; there should only be one dictionary per
	// action buffer.
	//
	// NOTE: Normally the dictionary is declared as the first
	// action in an action buffer, but I've seen what looks like
	// some form of copy protection that amounts to:
	//
	// <start of action buffer>
	//          push true
	//          branch_if_true label
	//          decl_dict   [0]   // this is never executed, but has lots of orphan data declared in the opcode
	// label:   // (embedded inside the previous opcode; looks like an invalid jump)
	//          ... "protected" code here, including the real decl_dict opcode ...
	//          <end of the dummy decl_dict [0] opcode>
	//
	// So we just interpret the first decl_dict we come to, and
	// cache the results.  If we ever hit a different decl_dict in
	// the same action_buffer, then we log an error and ignore it.
	{
		assert(stop_pc <= m_buffer.size());

		if (m_decl_dict_processed_at == start_pc)
		{
			// We've already processed this decl_dict.
			int	count = m_buffer[start_pc + 3] | (m_buffer[start_pc + 4] << 8);
			assert(m_dictionary.size() == count);
			UNUSED(count);
			return;
		}

		if (m_decl_dict_processed_at != -1)
		{
			log_error("error: process_decl_dict(%d, %d): decl_dict was already processed at %d\n",
				  start_pc,
				  stop_pc,
				  m_decl_dict_processed_at);
			return;
		}

		m_decl_dict_processed_at = start_pc;

		// Actual processing.
		int	i = start_pc;
		int	length = m_buffer[i + 1] | (m_buffer[i + 2] << 8);
		int	count = m_buffer[i + 3] | (m_buffer[i + 4] << 8);
		i += 2;

		UNUSED(length);

		assert(start_pc + 3 + length == stop_pc);

		m_dictionary.resize(count);

		// Index the strings.
		for (int ct = 0; ct < count; ct++)
		{
			// Point into the current action buffer.
			m_dictionary[ct] = (const char*) &m_buffer[3 + i];

			while (m_buffer[3 + i])
			{
				// safety check.
				if (i >= stop_pc)
				{
					log_error("error: action buffer dict length exceeded\n");

					// Jam something into the remaining (invalid) entries.
					while (ct < count)
					{
						m_dictionary[ct] = "<invalid>";
						ct++;
					}
					return;
				}
				i++;
			}
			i++;
		}
	}


	void	action_buffer::execute(as_environment* env)
	// Interpret the actions in this action buffer, and evaluate
	// them in the given environment.  Execute our whole buffer,
	// without any arguments passed in.
	{
		int	local_stack_top = env->get_local_frame_top();
		env->add_frame_barrier();

		array<with_stack_entry>	empty_with_stack;
		execute(env, 0, m_buffer.size(), NULL, empty_with_stack, false /* not function2 */);

		env->set_local_frame_top(local_stack_top);
	}


	/*private*/
	void
	action_buffer::doActionNew(as_environment* env, 
		array<with_stack_entry>& with_stack)
	{
		as_value	classname = env->pop();
		IF_VERBOSE_ACTION(log_msg("---new object: %s\n",
					  classname.to_tu_string().c_str()));
		int	nargs = (int) env->pop().to_number();
		as_value constructor = env->get_variable(classname.to_tu_string(), with_stack);
		as_value new_obj;
		if (constructor.get_type() == as_value::C_FUNCTION)
		{
			// C function is responsible for creating the new object and setting members.
			(constructor.to_c_function())(fn_call(&new_obj, NULL, env, nargs, env->get_top_index()));
		}
		else if (function_as_object* ctor_as_func = constructor.to_as_function())
		{
			// This function is being used as a constructor; make sure
			// it has a prototype object.
			ctor_as_func->lazy_create_properties();
			assert(ctor_as_func->m_properties);

			// Set up the prototype.
			as_value	proto;
			ctor_as_func->m_properties->get_member("prototype", &proto);

			assert(proto.to_object() != NULL);

			// Create an empty object, with a ref to the constructor's prototype.
			smart_ptr<as_object>	new_obj_ptr(new as_object(proto.to_object()));
			
			new_obj.set_as_object_interface(new_obj_ptr.get_ptr());

			// Call the actual constructor function; new_obj is its 'this'.
			// We don't need the function result.
			call_method(constructor, env, new_obj_ptr.get_ptr(), nargs, env->get_top_index());
		}
		else
		{
			if (classname != "String") {
				log_error("can't create object with unknown class '%s'\n",
					  classname.to_tu_string().c_str());
			} else {
				log_msg("Created special String class\n");
			}
		}

		env->drop(nargs);
		env->push(new_obj);
#if 0
		log_msg("new object %s at %p\n", classname.to_tu_string().c_str(), new_obj);
#endif
	}

	/*private*/
	void
	action_buffer::doActionInstanceOf(as_environment* env)
	{
		// Get the "super" function
		as_value& super = env->top(0);

		// Get the "instance" 
		as_value& instance = env->top(1);


		//IF_VERBOSE_ACTION(
			log_msg("-- %s intance_of %s\n",
				instance.to_string(),
				super.to_string());
		//);

		as_value result;

		// If any of the two is undefined, result is false
		if ( instance.get_type() == as_value::UNDEFINED || \
				super.get_type() == as_value::UNDEFINED ) 
		{
			result.set_bool(false);
		}
		else
		{

			// @@ TODO

			// Check if 'instance' implements all
			// methods of 'super'

			// @@ TODO

			result.set_undefined();


	#if 0
			if ( instance->implements(super) ) {
				env->push(true);
			} else {
				env->push(false);
			}
	#endif
		}
		// @@ TODO
		log_error("tocheck opcode: ACTION_INSTANCEOF (%02X)\n", SWF::ACTION_INSTANCEOF);

		env->drop(1);
		env->top(0) = result;
	}

	/*private*/
	void
	action_buffer::doActionCallMethod(as_environment* env)
	{
		// Some corner case behaviors depend on the SWF file version.
		//int version = env->get_target()->get_movie_definition()->get_version();

		// Get name of the method
		const tu_string&	method_name = env->top(0).to_tu_string();
		//log_msg(" method name: %s\n", method_name.c_str());

		// Get an object
		as_value& obj_value = env->top(1);
		as_object_interface*	obj = obj_value.to_object();
		//log_msg(" method object: %p\n", obj);

		// Get number of arguments
		int	nargs = (int) env->top(2).to_number();
		//log_msg(" method nargs: %d\n", nargs);

		as_value	result;

		if (!obj)
		{
			log_error("error: call_method invoked in something that "
				"doesn't cast to an as_object: %s\n",
				typeid(obj_value).name());
		}
		else
		{
			as_value	method;
			if (obj->get_member(method_name, &method))
			{
				if (method.get_type() != as_value::AS_FUNCTION &&
					method.get_type() != as_value::C_FUNCTION)
				{
					log_error("error: call_method: '%s' is not a method\n",
						  method_name.c_str());
				}
				else
				{
					result = call_method( method, env, obj, nargs,
						env->get_top_index() - 3);
				}
			}
			else
			{
				log_error("error: call_method can't find method %s "
					"for object %s (%p)\n", method_name.c_str(), 
					typeid(*obj).name(), obj);
			}
		}

		env->drop(nargs + 2);
		env->top(0) = result;
	}

	/*private*/
	void
	action_buffer::doActionCallFunction(as_environment* env,
			array<with_stack_entry>& with_stack)
	{
		as_value	function;
		if (env->top(0).get_type() == as_value::STRING)
		{
			// Function is a string; lookup the function.
			const tu_string&	function_name = env->top(0).to_tu_string();
			function = env->get_variable(function_name, with_stack);

			if (function.get_type() != as_value::AS_FUNCTION &&
				function.get_type() != as_value::C_FUNCTION) 
			{
				log_error("error in call_function: '%s' is not a function\n",
					  function_name.c_str());
			}
		}
		else
		{
			// Hopefully the actual
			// function object is here.
			// QUESTION: would this be
			// an ActionScript-defined
			// function ?
			function = env->top(0);
		}
		int	nargs = (int) env->top(1).to_number();
		as_value	result = call_method(function, env, NULL, nargs, env->get_top_index() - 2);
		env->drop(nargs + 1);
		env->top(0) = result;
	}

	/*private*/
	void
	action_buffer::doActionDefineFunction(as_environment* env,
			array<with_stack_entry>& with_stack, int pc, int* next_pc)
	{
		function_as_object* func = new function_as_object(this, env, *next_pc, with_stack);

		int	i = pc;
		i += 3;

		// Extract name.
		// @@ security: watch out for possible missing terminator here!
		tu_string	name = (const char*) &m_buffer[i];
		i += name.length() + 1;

		// Get number of arguments.
		int	nargs = m_buffer[i] | (m_buffer[i + 1] << 8);
		i += 2;

		// Get the names of the arguments.
		for (int n = 0; n < nargs; n++)
		{
			// @@ security: watch out for possible missing terminator here!
			func->add_arg(0, (const char*) &m_buffer[i]);
			i += func->m_args.back().m_name.length() + 1;
		}

		// Get the length of the actual function code.
		int	length = m_buffer[i] | (m_buffer[i + 1] << 8);
		i += 2;
		func->set_length(length);

		// Skip the function body (don't interpret it now).
		*next_pc += length;

		// If we have a name, then save the function in this
		// environment under that name.
		as_value	function_value(func);
		if (name.length() > 0)
		{
			// @@ NOTE: should this be m_target->set_variable()???
			env->set_member(name, function_value);
		}

		// Also leave it on the stack.
		env->push_val(function_value);

	}

	/*private*/
	void
	action_buffer::doActionDefineFunction2(as_environment* env,
			array<with_stack_entry>& with_stack, int pc, int* next_pc)
	{
		function_as_object*	func = new function_as_object(this, env, *next_pc, with_stack);
		func->set_is_function2();

		int	i = pc;
		i += 3;

		// Extract name.
		// @@ security: watch out for possible missing terminator here!
		tu_string	name = (const char*) &m_buffer[i];
		i += name.length() + 1;

		// Get number of arguments.
		int	nargs = m_buffer[i] | (m_buffer[i + 1] << 8);
		i += 2;

		// Get the count of local registers used by this function.
		uint8	register_count = m_buffer[i];
		i += 1;
		func->set_local_register_count(register_count);

		// Flags, for controlling register assignment of implicit args.
		uint16	flags = m_buffer[i] | (m_buffer[i + 1] << 8);
		i += 2;
		func->set_function2_flags(flags);

		// Get the register assignments and names of the arguments.
		for (int n = 0; n < nargs; n++)
		{
			int	arg_register = m_buffer[i];
			i++;

			// @@ security: watch out for possible missing terminator here!
			func->add_arg(arg_register, (const char*) &m_buffer[i]);
			i += func->m_args.back().m_name.length() + 1;
		}

		// Get the length of the actual function code.
		int	length = m_buffer[i] | (m_buffer[i + 1] << 8);
		i += 2;
		func->set_length(length);

		// Skip the function body (don't interpret it now).
		*next_pc += length;

		// If we have a name, then save the function in this
		// environment under that name.
		as_value	function_value(func);
		if (name.length() > 0)
		{
			// @@ NOTE: should this be m_target->set_variable()???
			env->set_member(name, function_value);
		}

		// Also leave it on the stack.
		env->push_val(function_value);

	}

	/*private*/
	void
	action_buffer::doActionGetMember(as_environment* env)
	{
		// Some corner case behaviors depend on the SWF file version.
		int version = env->get_target()->get_movie_definition()->get_version();

		as_value& target = env->top(1);
		as_object_interface* obj = target.to_object();

//		IF_VERBOSE_DEBUG(log_msg(" doActionGetMember: target: %p\n", obj));

		// Special case: String has a member "length"
		// @@ FIXME: we shouldn't have all this "special" cases --strk;
		if (obj == NULL && env->top(1).get_type() == as_value::STRING && env->top(0).to_tu_stringi() == "length")
		{
			int     len = env->top(1).to_tu_string_versioned(version).utf8_length();
			env->top(1).set_int(len);
		}
		else
		{
			env->top(1).set_undefined();
			// int	nargs = (int) env->top(1).to_number();
			if (obj) {
				obj->get_member(env->top(0).to_tu_string(), &(env->top(1)));
				if (env->top(1).to_object() == NULL) {
					IF_VERBOSE_ACTION(log_msg("-- get_member %s=%s\n",
								  env->top(0).to_tu_string().c_str(),
								  env->top(1).to_tu_string().c_str()));
				} else {
					IF_VERBOSE_ACTION(log_msg("-- get_member %s=%s at %p\n",
								  env->top(0).to_tu_string().c_str(),
								  env->top(1).to_tu_string().c_str(), env->top(1).to_object()));
				}
			}
			else
			{
				// @@ log error?
			}
		}
		env->drop(1);
	}



	void	action_buffer::execute(
		as_environment* env,
		int start_pc,
		int exec_bytes,
		as_value* retval,
		const array<with_stack_entry>& initial_with_stack,
		bool is_function2)
	// Interpret the specified subset of the actions in our
	// buffer.  Caller is responsible for cleaning up our local
	// stack frame (it may have passed its arguments in via the
	// local stack frame).
	// 
	// The is_function2 flag determines whether to use global or local registers.
	{
		action_init();	// @@ stick this somewhere else; need some global static init function

		assert(env);

		array<with_stack_entry>	with_stack(initial_with_stack);

		// Some corner case behaviors depend on the SWF file version.
		int version = env->get_target()->get_movie_definition()->get_version();

#if 0
		// Check the time
		if (periodic_events.expired()) {
			periodic_events.poll_event_handlers(env);
		}
#endif
		
		movie*	original_target = env->get_target();
		UNUSED(original_target);		// Avoid warnings.

		int	stop_pc = start_pc + exec_bytes;

		for (int pc = start_pc; pc < stop_pc; )
		{
			// Cleanup any expired "with" blocks.
			while (with_stack.size() > 0
			       && pc >= with_stack.back().m_block_end_pc)
			{
				// Drop this stack element
				with_stack.resize(with_stack.size() - 1);
			}

			// Get the opcode.
			int	action_id = m_buffer[pc];
			if ((action_id & 0x80) == 0)
			{
				IF_VERBOSE_ACTION(log_msg("EX:\t"); log_disasm(&m_buffer[pc]));

				// IF_VERBOSE_ACTION(log_msg("Action ID is: 0x%x\n", action_id));
			
				// Simple action; no extra data.
				switch (action_id)
				{
				default:
					break;

				case SWF::ACTION_END:	// end of actions.
					return;

				case SWF::ACTION_NEXTFRAME:	// next frame.
					env->get_target()->goto_frame(env->get_target()->get_current_frame() + 1);
					break;

				case SWF::ACTION_PREVFRAME:	// prev frame.
					env->get_target()->goto_frame(env->get_target()->get_current_frame() - 1);
					break;

				case SWF::ACTION_PLAY:	// action play
					env->get_target()->set_play_state(movie::PLAY);
					break;

				case SWF::ACTION_STOP:	// action stop
					env->get_target()->set_play_state(movie::STOP);
					break;

				case SWF::ACTION_TOGGLEQUALITY:	// toggle quality
				case SWF::ACTION_STOPSOUNDS:	// stop sounds
					break;

				case SWF::ACTION_ADD:	// add
				{
					env->top(1) += env->top(0);
					env->drop(1);
					break;
				}
				case SWF::ACTION_SUBTRACT:	// subtract
				{
					env->top(1) -= env->top(0);
					env->drop(1);
					break;
				}
				case SWF::ACTION_MULTIPLY:	// multiply
				{
					env->top(1) *= env->top(0);
					env->drop(1);
					break;
				}
				case SWF::ACTION_DIVIDE:	// divide
				{
					env->top(1) /= env->top(0);
					env->drop(1);
					break;
				}
				case SWF::ACTION_EQUAL:	// equal
				{
					env->top(1).set_bool(env->top(1) == env->top(0));
					env->drop(1);
					break;
				}
				case SWF::ACTION_LESSTHAN:	// less than
				{
					env->top(1).set_bool(env->top(1) < env->top(0));
					env->drop(1);
					break;
				}
				case SWF::ACTION_LOGICALAND:	// logical and
				{
					env->top(1).set_bool(env->top(1).to_bool() && env->top(0).to_bool());
					env->drop(1);
					break;
				}
				case SWF::ACTION_LOGICALOR:	// logical or
				{
					env->top(1).set_bool(env->top(1).to_bool() && env->top(0).to_bool());
					env->drop(1);
					break;
				}
				case SWF::ACTION_LOGICALNOT:	// logical not
				{
					env->top(0).set_bool(! env->top(0).to_bool());
					break;
				}
				case SWF::ACTION_STRINGEQ:	// string equal
				{
					env->top(1).set_bool(env->top(1).to_tu_string() == env->top(0).to_tu_string());
					env->drop(1);
					break;
				}
				case SWF::ACTION_STRINGLENGTH:	// string length
				{
					env->top(0).set_int(env->top(0).to_tu_string_versioned(version).utf8_length());
					break;
				}
				case SWF::ACTION_SUBSTRING:	// substring
				{
					int	size = int(env->top(0).to_number());
					int	base = int(env->top(1).to_number()) - 1;  // 1-based indices
					const tu_string&	str = env->top(2).to_tu_string_versioned(version);

					// Keep base within range.
					base = iclamp(base, 0, str.length());

					// Truncate if necessary.
					size = imin(str.length() - base, size);

					// @@ This can be done without new allocations if we get dirtier w/ internals
					// of as_value and tu_string...
					tu_string	new_string = str.c_str() + base;
					new_string.resize(size);

					env->drop(2);
					env->top(0).set_tu_string(new_string);

					break;
				}
				case SWF::ACTION_POP:	// pop
				{
					env->drop(1);
					break;
				}
				case SWF::ACTION_INT:	// int
				{
					env->top(0).set_int(int(floor(env->top(0).to_number())));
					break;
				}
				case SWF::ACTION_GETVARIABLE:	// get variable
				{
					as_value var_name = env->pop();
					tu_string var_string = var_name.to_tu_string();

					as_value variable = env->get_variable(var_string, with_stack);
					env->push(variable);
					if (variable.to_object() == NULL) {
						IF_VERBOSE_ACTION(log_msg("-- get var: %s=%s\n",
									  var_string.c_str(),
									  variable.to_tu_string().c_str()));
					} else {
						IF_VERBOSE_ACTION(log_msg("-- get var: %s=%s at %p\n",
									  var_string.c_str(),
									  variable.to_tu_string().c_str(), variable.to_object()));
					}
					

					break;
				}
				case SWF::ACTION_SETVARIABLE:	// set variable
				{
					env->set_variable(env->top(1).to_tu_string(), env->top(0), with_stack);
					IF_VERBOSE_ACTION(log_msg("-- set var: %s \n",
								  env->top(1).to_tu_string().c_str()));

					env->drop(2);
					break;
				}
				case SWF::ACTION_SETTARGETEXPRESSION:	// set target expression
				{
					const char * target_name = env->top(0).to_string();
					env->drop(1); // pop the target name off the stack
					movie *new_target;

					// if the string is blank, we set target to the root movie
					// TODO - double check this is correct?
					if (target_name[0] == '\0')
						new_target = env->find_target((tu_string)"/");
					else
						new_target = env->find_target((tu_string)target_name);

					if (new_target == NULL)
					{
						IF_VERBOSE_ACTION(log_error(
							"Couldn't find movie \"%s\" to set target to!"
							" Not setting target at all...",
							(const char *)target_name));
					}
					else
						env->set_target(new_target);

					break;
				}
				case SWF::ACTION_STRINGCONCAT:	// string concat
				{
					env->top(1).convert_to_string_versioned(version);
					env->top(1).string_concat(env->top(0).to_tu_string_versioned(version));
					env->drop(1);
					break;
				}
				case SWF::ACTION_GETPROPERTY:	// get property
				{

					movie*	target = env->find_target(env->top(1));
					if (target)
					{
						env->top(1) = get_property(target, (int) env->top(0).to_number());
					}
					else
					{
						env->top(1) = as_value();
					}
					env->drop(1);
					break;
				}

				case SWF::ACTION_SETPROPERTY:	// set property
				{

					movie*	target = env->find_target(env->top(2));
					if (target)
					{
						set_property(target, (int) env->top(1).to_number(), env->top(0));
					}
					env->drop(3);
					break;
				}

				case SWF::ACTION_DUPLICATECLIP:	// duplicate clip (sprite?)
				{
					env->get_target()->clone_display_object(
						env->top(2).to_tu_string(),
						env->top(1).to_tu_string(),
						(int) env->top(0).to_number());
					env->drop(3);
					break;
				}

				case SWF::ACTION_REMOVECLIP:	// remove clip
					env->get_target()->remove_display_object(env->top(0).to_tu_string());
					env->drop(1);
					break;

				case SWF::ACTION_TRACE:	// trace
				{
					// Log the stack val.
					as_global_trace(fn_call(&env->top(0), NULL, env, 1, env->get_top_index()));
					env->drop(1);
					break;
				}

				case SWF::ACTION_STARTDRAGMOVIE:	// start drag movie
				{
					movie::drag_state	st;

					st.m_character = env->find_target(env->top(0));
					if (st.m_character == NULL)
					{
						log_error("error: start_drag of invalid target '%s'.\n",
							  env->top(0).to_string());
					}

					st.m_lock_center = env->top(1).to_bool();
					st.m_bound = env->top(2).to_bool();
					if (st.m_bound)
					{
						st.m_bound_x0 = (float) env->top(6).to_number();
						st.m_bound_y0 = (float) env->top(5).to_number();
						st.m_bound_x1 = (float) env->top(4).to_number();
						st.m_bound_y1 = (float) env->top(3).to_number();
						env->drop(4);
					}
					env->drop(3);

					movie*	root_movie = env->get_target()->get_root_movie();
					assert(root_movie);

					if (root_movie && st.m_character)
					{
						root_movie->set_drag_state(st);
					}
					
					break;
				}

				case SWF::ACTION_STOPDRAGMOVIE:	// stop drag movie
				{
					movie*	root_movie = env->get_target()->get_root_movie();
					assert(root_movie);

					root_movie->stop_drag();

					break;
				}

				case SWF::ACTION_STRINGCOMPARE:	// string less than
				{
					env->top(1).set_bool(env->top(1).to_tu_string() < env->top(0).to_tu_string());
					break;
				}

				case SWF::ACTION_THROW: // 0x2A
				{
					log_error("todo opcode: %02X\n", action_id);
					break;
				}

				case SWF::ACTION_CASTOP: // 0x2B
				{
					// TODO
					//
					// Pop o1, pop s2
					// Make sure o1 is an instance of s2.
					// If the cast succeeds, push o1, else push NULL.
					//
					// The cast doesn't appear to coerce at all, it's more
					// like a dynamic_cast<> in C++ I think.
					log_error("todo opcode: %02X\n", action_id);
					break;
				}

				case SWF::ACTION_IMPLEMENTSOP: // 0x2C
				{
					// Declare that a class s1 implements one or more
					// interfaces (i2 == number of interfaces, s3..sn are the names
					// of the interfaces).
					log_error("todo opcode: %02X\n", action_id);
					break;
				}

				case SWF::ACTION_RANDOM:	// random
				{
					int	max = int(env->top(0).to_number());
					if (max < 1) max = 1;
					env->top(0).set_int(tu_random::next_random() % max);
					break;
				}
				case SWF::ACTION_MBLENGTH:	// mb length
				{
					// @@ TODO
					log_error("todo opcode: %02X\n", action_id);
					break;
				}
				case SWF::ACTION_ORD:	// ord
				{
					// ASCII code of first character
					env->top(0).set_int(env->top(0).to_string()[0]);
					break;
				}
				case SWF::ACTION_CHR:	// chr
				{
					char	buf[2];
					buf[0] = int(env->top(0).to_number());
					buf[1] = 0;
					env->top(0).set_string(buf);
					break;
				}

				case SWF::ACTION_GETTIMER:	// get timer
					// Push milliseconds since we started playing.
					env->push(floorf(env->m_target->get_timer() * 1000.0f));
					break;

				case SWF::ACTION_MBSUBSTRING:	// mb substring
				{
					// @@ TODO
					log_error("todo opcode: %02X\n", action_id);
					break;
				}
				case SWF::ACTION_MBCHR:	// mb chr
				{
					// @@ TODO
					log_error("todo opcode: %02X\n", action_id);
					break;
				}
				case SWF::ACTION_DELETEVAR:	// delete
				{
					// @@ TODO
					
					// Apparently this can be used to remove properties from
					// an object?

					log_error("todo opcode: %02X\n", action_id);
					break;
				}
				case SWF::ACTION_DELETE:	// delete2
				{
					// @@ tulrich: delete is not valid here!  Do we actually just want to 
					// NULL out the object pointer in the environment (to drop the ref)?
					// Should at least check the ref count before deleting anything!!!
//					as_value	obj_name = env->pop();
					as_value obj_ptr = env->get_variable_raw(env->top(0).to_tu_string(), with_stack);
///x					delete obj_ptr.to_object();
// 	 				log_error("%08X\n", obj_ptr.to_object());
					log_error("todo opcode: %02X\n", action_id);
					break;
				}

				case SWF::ACTION_VAREQUALS:	// set local
				{
					as_value	value = env->pop();
					as_value	varname = env->pop();
					env->set_local(varname.to_tu_string(), value);
					break;
				}

				case SWF::ACTION_CALLFUNCTION:	// call function
					doActionCallFunction(env, with_stack);
					break;
				case SWF::ACTION_RETURN:	// return
				{
					// Put top of stack in the provided return slot, if
					// it's not NULL.
					if (retval)
					{
						*retval = env->top(0);
					}
					env->drop(1);

					// Skip the rest of this buffer (return from this action_buffer).
					pc = stop_pc;

					break;
				}
				case SWF::ACTION_MODULO:	// modulo
				{
					as_value	result;
					double	y = env->pop().to_number();
					double	x = env->pop().to_number();
					// Don't need to check for y being 0 here - if it's zero, fmod returns NaN, which is what flash would do too
					result = fmod(x, y);
//					env->top(1).set_double(fmod(env->top(1).to_bool() && env->top(0).to_bool());
//					env->drop(1);
//					log_error("modulo x=%f, y=%f, z=%f\n",x,y,result.to_number());
					env->push(result);
					break;
				}
				case SWF::ACTION_NEW:	// new
					doActionNew(env, with_stack);
					break;
				case SWF::ACTION_VAR:	// declare local
				{
					const tu_string&	varname = env->top(0).to_tu_string();
					env->declare_local(varname);
					env->drop(1);
					break;
				}
				case SWF::ACTION_INITARRAY:	// init array
				{
					int	array_size = (int) env->pop().to_number();

					//log_msg("xxx init array: size = %d, top of stack = %d\n", array_size, env->get_top_index());//xxxxx

 					// Call the array constructor, to create an empty array.
 					as_value	result;
 					as_global_array_ctor(fn_call(&result, NULL, env, 0, env->get_top_index()));

 					as_object_interface*	ao = result.to_object();
 					assert(ao);

					// Fill the elements with the initial values from the stack.
 					as_value	index_number;
 					for (int i = 0; i < array_size; i++)
 					{
 						// @@ TODO a set_member that takes an int or as_value?
 						index_number.set_int(i);
 						ao->set_member(index_number.to_string(), env->pop());
 					}

 					env->push(result);

					//log_msg("xxx init array end: top of stack = %d, trace(top(0)) =", env->get_top_index());//xxxxxxx

					//as_global_trace(fn_call(NULL, NULL, env, 1, env->get_top_index()));	//xxxx

					break;
				}
				case SWF::ACTION_INITOBJECT:	// declare object
				{
					// 
					//    SWFACTION_PUSH
					//     [000]   Constant: 1 "obj"
					//     [001]   Constant: 0 "member" <-- we handle up to here
					//     [002]   Integer: 1
					//     [003]   Integer: 1
					//    SWFACTION_INITOBJECT

					int nmembers = (int) env->pop().to_number();

					smart_ptr<as_object> new_obj_ptr(new as_object); 

					// Set provided members
					for (int i=0; i<nmembers; ++i) {
						as_value member_value = env->pop();
						tu_stringi member_name = env->pop().to_tu_stringi();
						new_obj_ptr->set_member(member_name, member_value);
					}

					// @@ TODO
					//log_error("checkme opcode: %02X\n", action_id);

					as_value new_obj;
					new_obj.set_as_object_interface(new_obj_ptr.get_ptr());

					//env->drop(nmembers*2);
					env->push(new_obj); 

					break;
				}
				case SWF::ACTION_TYPEOF:	// type of
				{
					switch(env->top(0).get_type())
					{
					case as_value::UNDEFINED:
						env->top(0).set_string("undefined");
						break;
					case as_value::STRING:
						env->top(0).set_string("string");
						break;
					case as_value::NUMBER:
						env->top(0).set_string("number");
						break;
					case as_value::BOOLEAN:
						env->top(0).set_string("boolean");
						break;
					case as_value::OBJECT:
						env->top(0).set_string("object");
						break;
					case as_value::NULLTYPE:
						env->top(0).set_string("null");
						break;
					case as_value::AS_FUNCTION:
						env->top(0).set_string("function");
						break;
					default:
						log_error("typeof unknown type: %02X\n", env->top(0).get_type());
						break;
					}
					break;
				}
				case SWF::ACTION_TARGETPATH:	// get target
				{
					// @@ TODO
					log_error("todo opcode: %02X\n", action_id);
					break;
				}
				case SWF::ACTION_ENUMERATE:	// enumerate
				{
					as_value var_name = env->pop();
					const tu_string& var_string = var_name.to_tu_string();

					as_value variable = env->get_variable(var_string, with_stack);

					if (variable.to_object() == NULL)
					{
						break;
					}
					const as_object* object = (as_object*) (variable.to_object());

					// The end of the enumeration
					as_value nullvalue;
					nullvalue.set_null();
					env->push(nullvalue);
					IF_VERBOSE_ACTION(log_msg("---enumerate - push: NULL\n"));

					stringi_hash<as_member>::const_iterator it = object->m_members.begin();
					while (it != object->m_members.end())
					{
						const as_member member = (it.get_value());

						if (! member.get_member_flags().get_dont_enum())
						{
							env->push(as_value(it.get_key()));

							IF_VERBOSE_ACTION(log_msg("---enumerate - push: %s\n",
										  it.get_key().c_str()));
						}
							
						++it;
					}

					const as_object * prototype = (as_object *) object->m_prototype;
					if (prototype != NULL)
					{
						stringi_hash<as_member>::const_iterator it = prototype->m_members.begin();
						while (it != prototype->m_members.end())
						{
							const as_member member = (it.get_value());

							if (! member.get_member_flags().get_dont_enum())
							{
								env->push(as_value(it.get_key()));

								IF_VERBOSE_ACTION(log_msg("---enumerate - push: %s\n",
											  it.get_key().c_str()));
							}
								
							++it;
						};
					}

					break;
				}
				case SWF::ACTION_NEWADD:	// add_t (typed)
				{
					if (env->top(0).get_type() == as_value::STRING
					    || env->top(1).get_type() == as_value::STRING)
					{
						env->top(1).convert_to_string_versioned(version);
						env->top(1).string_concat(env->top(0).to_tu_string_versioned(version));
					}
					else
					{
						env->top(1) += env->top(0);
					}
					env->drop(1);
					break;
				}
				case SWF::ACTION_NEWLESSTHAN:	// less than (typed)
				{
					if (env->top(1).get_type() == as_value::STRING)
					{
						env->top(1).set_bool(env->top(1).to_tu_string() < env->top(0).to_tu_string());
					}
					else
					{
						env->top(1).set_bool(env->top(1) < env->top(0));
					}
					env->drop(1);
					break;
				}
				case SWF::ACTION_NEWEQUALS:	// equal (typed)
				{
					// @@ identical to untyped equal, as far as I can tell...
					env->top(1).set_bool(env->top(1) == env->top(0));
					env->drop(1);
					break;
				}
				case SWF::ACTION_TONUMBER:	// to number
				{
					env->top(0).convert_to_number();
					break;
				}
				case SWF::ACTION_TOSTRING:	// to string
				{
					env->top(0).convert_to_string_versioned(version);
					break;
				}
				case SWF::ACTION_DUP:	// dup
					env->push(env->top(0));
					break;
				
				case SWF::ACTION_SWAP:	// swap
				{
					as_value	temp = env->top(1);
					env->top(1) = env->top(0);
					env->top(0) = temp;
					break;
				}
				case SWF::ACTION_GETMEMBER:	// get member
					doActionGetMember(env);
					break;
				case SWF::ACTION_SETMEMBER:	// set member
				{
					as_object_interface*	obj = env->top(2).to_object();
					if (obj)
					{
						obj->set_member(env->top(1).to_tu_string(), env->top(0));
						IF_VERBOSE_ACTION(
							log_msg("-- set_member %s.%s=%s\n",
								env->top(2).to_tu_string().c_str(),
								env->top(1).to_tu_string().c_str(),
								env->top(0).to_tu_string().c_str()));
					}
					else
					{
						// Invalid object, can't set.
						IF_VERBOSE_ACTION(
							log_msg("-- set_member %s.%s=%s on invalid object!\n",
								env->top(2).to_tu_string().c_str(),
								env->top(1).to_tu_string().c_str(),
								env->top(0).to_tu_string().c_str()));
					}
					env->drop(3);
					break;
				}
				case SWF::ACTION_INCREMENT:	// increment
					env->top(0) += 1;
					break;
				case SWF::ACTION_DECREMENT:	// decrement
					env->top(0) -= 1;
					break;

				case SWF::ACTION_CALLMETHOD:	// call method
					doActionCallMethod(env);
					break;
				case SWF::ACTION_NEWMETHOD:	// new method
					// @@ TODO
					log_error("todo opcode: %02X\n", action_id);
					break;
				case SWF::ACTION_INSTANCEOF:	// instance of
					// @@ TODO
					log_error("todo opcode: %02X\n", action_id);
					break;
				case SWF::ACTION_ENUM2:	// enumerate object
					// @@ TODO
					log_error("todo opcode: %02X\n", action_id);
					break;
				case SWF::ACTION_BITWISEAND:	// bitwise and
					env->top(1) &= env->top(0);
					env->drop(1);
					break;
				case SWF::ACTION_BITWISEOR:	// bitwise or
					env->top(1) |= env->top(0);
					env->drop(1);
					break;
				case SWF::ACTION_BITWISEXOR:	// bitwise xor
					env->top(1) ^= env->top(0);
					env->drop(1);
					break;
				case SWF::ACTION_SHIFTLEFT:	// shift left
					env->top(1).shl(env->top(0));
					env->drop(1);
					break;
				case SWF::ACTION_SHIFTRIGHT:	// shift right (signed)
					env->top(1).asr(env->top(0));
					env->drop(1);
					break;
				case SWF::ACTION_SHIFTRIGHT2:	// shift right (unsigned)
					env->top(1).lsr(env->top(0));
					env->drop(1);
					break;
				case SWF::ACTION_STRICTEQ:	// strict equal
					if (env->top(1).get_type() != env->top(0).get_type())
					{
						// Types don't match.
						env->top(1).set_bool(false);
						env->drop(1);
					}
					else
					{
						env->top(1).set_bool(env->top(1) == env->top(0));
						env->drop(1);
					}
					break;
				case SWF::ACTION_GREATER: // 0x67:
					if (env->top(1).get_type() == as_value::STRING)
					{
						env->top(1).set_bool(env->top(1).to_tu_string() > env->top(0).to_tu_string());
					}
					else
					{
						env->top(1).set_bool(env->top(1).to_number() > env->top(0).to_number());
					}
					env->drop(1);
					break;
				case SWF::ACTION_STRINGGREATER: // 0x68
					env->top(1).set_bool(env->top(1).to_tu_string() > env->top(0).to_tu_string());
					env->drop(1);
					break;

				case SWF::ACTION_EXTENDS: // 0x69
					log_error("todo opcode: %02X\n", action_id);
					break;

				}
				pc++;	// advance to next action.
			}
			else
			{
				IF_VERBOSE_ACTION(log_msg("EX:\t"); log_disasm(&m_buffer[pc]));

				// Action containing extra data.
				int	length = m_buffer[pc + 1] | (m_buffer[pc + 2] << 8);
				int	next_pc = pc + length + 3;

				switch (action_id)
				{
				default:
					break;

				case SWF::ACTION_GOTOFRAME:	// goto frame
				{
					int	frame = m_buffer[pc + 3] | (m_buffer[pc + 4] << 8);
					// 0-based already?
					//// Convert from 1-based to 0-based
					//frame--;
					env->get_target()->goto_frame(frame);
					break;
				}

				case SWF::ACTION_GETURL:	// get url
				{
					// If this is an FSCommand, then call the callback
					// handler, if any.

					// Two strings as args.
					const char*	url = (const char*) &(m_buffer[pc + 3]);
					int	url_len = strlen(url);
					const char*	target = (const char*) &(m_buffer[pc + 3 + url_len + 1]);

					// If the url starts with "FSCommand:", then this is
					// a message for the host app.
					if (strncmp(url, "FSCommand:", 10) == 0)
					{
						if (s_fscommand_handler)
						{
							// Call into the app.
							(*s_fscommand_handler)(env->get_target()->get_root_interface(), url + 10, target);
						}
					}
					else
					{
#ifdef EXTERN_MOVIE
//						log_error("get url: target=%s, url=%s\n", target, url);
            
						tu_string tu_target = target;
						movie* target_movie = env->find_target(tu_target);
						if (target_movie != NULL)
						{
							movie*	root_movie = env->get_target()->get_root_movie();
							attach_extern_movie(url, target_movie, root_movie);
						}
						else
						{
							log_error("get url: target %s not found\n", target);
						}
#endif // EXTERN_MOVIE
					}

					break;
				}

				case SWF::ACTION_SETREGISTER:	// store_register
				{
					int	reg = m_buffer[pc + 3];
					// Save top of stack in specified register.
					if (is_function2)
					{
						*(env->local_register_ptr(reg)) = env->top(0);

						IF_VERBOSE_ACTION(
							log_msg("-------------- local register[%d] = '%s'\n",
								reg,
								env->top(0).to_string()));
					}
					else if (reg >= 0 && reg < 4)
					{
						env->m_global_register[reg] = env->top(0);
					
						IF_VERBOSE_ACTION(
							log_msg("-------------- global register[%d] = '%s'\n",
								reg,
								env->top(0).to_string()));
					}
					else
					{
						log_error("store_register[%d] -- register out of bounds!", reg);
					}

					break;
				}

				case SWF::ACTION_CONSTANTPOOL:	// decl_dict: declare dictionary
				{
					int	i = pc;
					//int	count = m_buffer[pc + 3] | (m_buffer[pc + 4] << 8);
					i += 2;

					process_decl_dict(pc, next_pc);

					break;
				}

				case SWF::ACTION_WAITFORFRAME:	// wait for frame
				{
					// If we haven't loaded a specified frame yet, then we're supposed to skip
					// some specified number of actions.
					//
					// Since we don't load incrementally, just ignore this opcode.
					break;
				}

				case SWF::ACTION_SETTARGET:	// set target
				{
					// Change the movie we're working on.
					const char* target_name = (const char*) &m_buffer[pc + 3];
					movie *new_target;

					// if the string is blank, we set target to the root movie
					// TODO - double check this is correct?
					if (target_name[0] == '\0')
						new_target = env->find_target((tu_string)"/");
					else
						new_target = env->find_target((tu_string)target_name);

					if (new_target == NULL)
					{
						IF_VERBOSE_ACTION(log_error(
							"Couldn't find movie \"%s\" to set target to!"
							" Not setting target at all...",
							(const char *)target_name));
					}
					else
						env->set_target(new_target);

					break;
				}

				case SWF::ACTION_GOTOLABEL:	// go to labeled frame, goto_frame_lbl
				{
					char*	frame_label = (char*) &m_buffer[pc + 3];
					movie *target = env->get_target();
					target->goto_labeled_frame(frame_label);
					break;
				}

				case SWF::ACTION_WAITFORFRAMEEXPRESSION:	// wait for frame expression (?)
				{
					// Pop the frame number to wait for; if it's not loaded skip the
					// specified number of actions.
					//
					// Since we don't support incremental loading, pop our arg and
					// don't do anything.
					env->drop(1);
					break;
				}

				case SWF::ACTION_DEFINEFUNCTION2: // 0x8E
					doActionDefineFunction2(env, with_stack, pc, &next_pc);
					break;

				case SWF::ACTION_WITH:	// with
				{
					int	frame = m_buffer[pc + 3] | (m_buffer[pc + 4] << 8);
					UNUSED(frame);
					IF_VERBOSE_ACTION(log_msg("-------------- with block start: stack size is %d\n", with_stack.size()));
					if (with_stack.size() < 8)
					{
 						int	block_length = m_buffer[pc + 3] | (m_buffer[pc + 4] << 8);
 						int	block_end = next_pc + block_length;
 						as_object_interface*	with_obj = env->top(0).to_object();
 						with_stack.push_back(with_stack_entry(with_obj, block_end));
					}
					env->drop(1);
					break;
				}
				case SWF::ACTION_PUSHDATA:	// push_data
				{
					int i = pc;
					while (i - pc < length)
					{
						int	type = m_buffer[3 + i];
						i++;
						if (type == 0)
						{
							// string
							const char*	str = (const char*) &m_buffer[3 + i];
							i += strlen(str) + 1;
							env->push(str);

							IF_VERBOSE_ACTION(log_msg("-------------- pushed '%s'\n", str));
						}
						else if (type == 1)
						{
							// float (little-endian)
							union {
								float	f;
								Uint32	i;
							} u;
							compiler_assert(sizeof(u) == sizeof(u.i));

							memcpy(&u.i, &m_buffer[3 + i], 4);
							u.i = swap_le32(u.i);
							i += 4;

							env->push(u.f);

							IF_VERBOSE_ACTION(log_msg("-------------- pushed '%f'\n", u.f));
						}
						else if (type == 2)
						{
							as_value nullvalue;
							nullvalue.set_null();
							env->push(nullvalue);	

							IF_VERBOSE_ACTION(log_msg("-------------- pushed NULL\n"));
						}
						else if (type == 3)
						{
							env->push(as_value());

							IF_VERBOSE_ACTION(log_msg("-------------- pushed UNDEFINED\n"));
						}
						else if (type == 4)
						{
							// contents of register
							int	reg = m_buffer[3 + i];
							UNUSED(reg);
							i++;
							if (is_function2)
							{
								env->push(*(env->local_register_ptr(reg)));
								IF_VERBOSE_ACTION(
									log_msg("-------------- pushed local register[%d] = '%s'\n",
										reg,
										env->top(0).to_string()));
							}
							else if (reg < 0 || reg >= 4)
							{
								env->push(as_value());
								log_error("push register[%d] -- register out of bounds!\n", reg);
							}
							else
							{
								env->push(env->m_global_register[reg]);
								IF_VERBOSE_ACTION(
									log_msg("-------------- pushed global register[%d] = '%s'\n",
										reg,
										env->top(0).to_string()));
							}

						}
						else if (type == 5)
						{
							bool	bool_val = m_buffer[3 + i] ? true : false;
							i++;
//							log_msg("bool(%d)\n", bool_val);
							env->push(bool_val);

							IF_VERBOSE_ACTION(log_msg("-------------- pushed %s\n", bool_val ? "true" : "false"));
						}
						else if (type == 6)
						{
							// double
							// wacky format: 45670123
							union {
								double	d;
								Uint64	i;
								struct {
									Uint32	lo;
									Uint32	hi;
								} sub;
							} u;
							compiler_assert(sizeof(u) == sizeof(u.i));

							memcpy(&u.sub.hi, &m_buffer[3 + i], 4);
							memcpy(&u.sub.lo, &m_buffer[3 + i + 4], 4);
							u.i = swap_le64(u.i);
							i += 8;

							env->push(u.d);

							IF_VERBOSE_ACTION(log_msg("-------------- pushed double %f\n", u.d));
						}
						else if (type == 7)
						{
							// int32
							Sint32	val = m_buffer[3 + i]
								| (m_buffer[3 + i + 1] << 8)
								| (m_buffer[3 + i + 2] << 16)
								| (m_buffer[3 + i + 3] << 24);
							i += 4;
						
							env->push(val);

							IF_VERBOSE_ACTION(log_msg("-------------- pushed int32 %d\n", val));
						}
						else if (type == 8)
						{
							int	id = m_buffer[3 + i];
							i++;
							if (id < m_dictionary.size())
							{
								env->push(m_dictionary[id]);

								IF_VERBOSE_ACTION(log_msg("-------------- pushed '%s'\n", m_dictionary[id]));
							}
							else
							{
								log_error("error: dict_lookup(%d) is out of bounds!\n", id);
								env->push(0);
								IF_VERBOSE_ACTION(log_msg("-------------- pushed 0\n"));
							}
						}
						else if (type == 9)
						{
							int	id = m_buffer[3 + i] | (m_buffer[4 + i] << 8);
							i += 2;
							if (id < m_dictionary.size())
							{
								env->push(m_dictionary[id]);
								IF_VERBOSE_ACTION(log_msg("-------------- pushed '%s'\n", m_dictionary[id]));
							}
							else
							{
								log_error("error: dict_lookup(%d) is out of bounds!\n", id);
								env->push(0);

								IF_VERBOSE_ACTION(log_msg("-------------- pushed 0"));
							}
						}
					}
					
					break;
				}
				case SWF::ACTION_BRANCHALWAYS:	// branch always (goto)
				{
					Sint16	offset = m_buffer[pc + 3] | (m_buffer[pc + 4] << 8);
					next_pc += offset;
					// @@ TODO range checks
					break;
				}
				case SWF::ACTION_GETURL2:	// get url 2
				{
					int	method = m_buffer[pc + 3];
					UNUSED(method);

					const char*	target = env->top(0).to_string();
					const char*	url = env->top(1).to_string();

					// If the url starts with "FSCommand:", then this is
					// a message for the host app.
					if (strncmp(url, "FSCommand:", 10) == 0)
					{
						if (s_fscommand_handler)
						{
							// Call into the app.
							(*s_fscommand_handler)(env->get_target()->get_root_interface(), url + 10, target);
						}
					}
					else
					{
#ifdef EXTERN_MOVIE
//            log_error("get url2: target=%s, url=%s\n", target, url);

						movie* target_movie = env->find_target(env->top(0));
						if (target_movie != NULL)
						{
							movie*	root_movie = env->get_target()->get_root_movie();
							attach_extern_movie(url, target_movie, root_movie);
						}
						else
						{
							log_error("get url2: target %s not found\n", target);
						}
#endif // EXTERN_MOVIE
					}
					env->drop(2);
					break;
				}

				case SWF::ACTION_DEFINEFUNCTION: // declare function
					doActionDefineFunction(env, with_stack, pc, &next_pc);
					break;

				case SWF::ACTION_BRANCHIFTRUE:	// branch if true
				{
					Sint16	offset = m_buffer[pc + 3] | (m_buffer[pc + 4] << 8);
					
					bool	test = env->top(0).to_bool();
					env->drop(1);
					if (test)
					{
						next_pc += offset;

						if (next_pc > stop_pc)
						{
							log_error("branch to offset %d -- this section only runs to %d\n",
								  next_pc,
								  stop_pc);
						}
					}
					break;
				}
				case SWF::ACTION_CALLFRAME:	// call frame
				{
					// Note: no extra data in this instruction!
					assert(env->m_target);
					env->m_target->call_frame_actions(env->top(0));
					env->drop(1);

					break;
				}

				case SWF::ACTION_GOTOEXPRESSION:	// goto frame expression, goto_frame_exp
				{
					// From Alexi's SWF ref:
					//
					// Pop a value or a string and jump to the specified
					// frame. When a string is specified, it can include a
					// path to a sprite as in:
					// 
					//   /Test:55
					// 
					// When f_play is ON, the action is to play as soon as
					// that frame is reached. Otherwise, the
					// frame is shown in stop mode.

					unsigned char	play_flag = m_buffer[pc + 3];
					movie::play_state	state = play_flag ? movie::PLAY : movie::STOP;

					movie* target = env->get_target();
					bool success = false;

					if (env->top(0).get_type() == as_value::UNDEFINED)
					{
						// No-op.
					}
					else if (env->top(0).get_type() == as_value::STRING)
					{
						// @@ TODO: parse possible sprite path...
						
						// Also, if the frame spec is actually a number (not a label), then
						// we need to do the conversion...

						const char* frame_label = env->top(0).to_string();
						if (target->goto_labeled_frame(frame_label))
						{
							success = true;
						}
						else
						{
							// Couldn't find the label.  Try converting to a number.
							double num;
							if (string_to_number(&num, env->top(0).to_string()))
							{
								int frame_number = int(num);
								target->goto_frame(frame_number);
								success = true;
							}
							// else no-op.
						}
					}
					else if (env->top(0).get_type() == as_value::OBJECT)
					{
						// This is a no-op; see test_goto_frame.swf
					}
					else if (env->top(0).get_type() == as_value::NUMBER)
					{
						// Frame numbers appear to be 0-based!  @@ Verify.
						int frame_number = int(env->top(0).to_number());
						target->goto_frame(frame_number);
						success = true;
					}

					if (success)
					{
						target->set_play_state(state);
					}
					
					env->drop(1);

					break;
				}
				
				}
				pc = next_pc;
			}
		}

		env->set_target(original_target);
	}


	//
	// as_value -- ActionScript value type
	//

	
	as_value::as_value(as_object_interface* obj)
		:
		m_type(OBJECT),
		m_object_value(obj)
	{
		if (m_object_value)
		{
			m_object_value->add_ref();
		}
	}


	as_value::as_value(function_as_object* func)
		:
		m_type(AS_FUNCTION),
		m_as_function_value(func)
	{
		if (m_as_function_value)
		{
			m_as_function_value->add_ref();
		}
	}


	const char*	as_value::to_string() const
	// Conversion to string.
	{
		return to_tu_string().c_str();
	}


	const tu_stringi&	as_value::to_tu_stringi() const
	{
		return reinterpret_cast<const tu_stringi&>(to_tu_string());
	}


	const tu_string&	as_value::to_tu_string() const
	// Conversion to const tu_string&.
	{
		if (m_type == STRING) { /* don't need to do anything */ }
		else if (m_type == NUMBER)
		{
			// @@ Moock says if value is a NAN, then result is "NaN"
			// INF goes to "Infinity"
			// -INF goes to "-Infinity"
			if (isnan(m_number_value)) m_string_value = "NaN";
			else if (isinf(m_number_value))
			{
				if (m_number_value > 0.0) m_string_value = "+Infinity";
				else m_string_value = "-Infinity";
			}
			else
			{
				char buffer[50];
				snprintf(buffer, 50, "%.14g", m_number_value);
				m_string_value = buffer;
			}
		}
		else if (m_type == UNDEFINED)
		{
			// Behavior depends on file version.  In
			// version 7+, it's "undefined", in versions
			// 6-, it's "".
			//
			// We'll go with the v7 behavior by default,
			// and conditionalize via _versioned()
			// functions.
			m_string_value = "undefined";
		}
		else if (m_type == NULLTYPE)
		{ 
			m_string_value = "null";
		}
		else if (m_type == BOOLEAN)
		{
			m_string_value = this->m_boolean_value ? "true" : "false";
		}
		else if (m_type == OBJECT)
		{
			// @@ Moock says, "the value that results from
			// calling toString() on the object".
			//
			// The default toString() returns "[object
			// Object]" but may be customized.
			//
			// A Movieclip returns the absolute path of the object.

			const char*	val = NULL;
			if (m_object_value)
			{
				val = m_object_value->get_text_value();
			}

			if (val)
			{
				m_string_value = val;
			}
			else
			{
				// Do we have a "toString" method?
				//
				// TODO: we need an environment in order to call toString()!

				// This is the default.
				//m_string_value = "[object Object]";
				char buffer[50];
				snprintf(buffer, 50, "<as_object %p>", (void *) m_object_value);
				m_string_value = buffer;
			}
		}
		else if (m_type == C_FUNCTION)
		{
			char buffer[50];
			snprintf(buffer, 50, "<c_function %p>", (void *) m_c_function_value);
			m_string_value = buffer;
		}
		else if (m_type == AS_FUNCTION)
		{
			char buffer[50];
			snprintf(buffer, 50, "<as_function %p>", (void *) m_as_function_value);
			m_string_value = buffer;
		}
		else
		{
			m_string_value = "<bad type> "+m_type;
			assert(0);
		}
		
		return m_string_value;
	}


	const tu_string&	as_value::to_tu_string_versioned(int version) const
	// Conversion to const tu_string&.
	{
		if (m_type == UNDEFINED)
		{
			// Version-dependent behavior.
			if (version <= 6)
			{
				m_string_value = "";
			}
			else
			{
				m_string_value = "undefined";
			}
			return m_string_value;
		}
		
		return to_tu_string();
	}

	double	as_value::to_number() const
	// Conversion to double.
	{
		if (m_type == STRING)
		{
			// @@ Moock says the rule here is: if the
			// string is a valid float literal, then it
			// gets converted; otherwise it is set to NaN.
			//
			// Also, "Infinity", "-Infinity", and "NaN"
			// are recognized.
			if (! string_to_number(&m_number_value, m_string_value.c_str()))
			{
				// Failed conversion to Number.
				double temp = 0.0; // avoid divide by zero compiler warning by using a variable
				m_number_value = temp / temp;	// this division by zero creates a NaN value in the double
			}
			return m_number_value;
		}
		else if (m_type == NULLTYPE)
		{
 			// Evan: from my tests
			return 0;
		}
		else if (m_type == BOOLEAN)
		{
			// Evan: from my tests
			return (this->m_boolean_value) ? 1 : 0;
		}
		else if (m_type == NUMBER)
		{
			return m_number_value;
		}
		else if (m_type == OBJECT && m_object_value != NULL)
		{
			// @@ Moock says the result here should be
			// "the return value of the object's valueOf()
			// method".
			//
			// Arrays and Movieclips should return NaN.

			// Text characters with var names could get in
			// here.
			const char* textval = m_object_value->get_text_value();
			if (textval)
			{
				return atof(textval);
			}

			return 0.0;
		}
		else
		{
			return 0.0;
		}
	}


	bool	as_value::to_bool() const
	// Conversion to boolean.
	{
		// From Moock
		if (m_type == STRING)
		{
			if (m_string_value == "false")
			{
				return false;
			}
			else if (m_string_value == "true")
			{
				return true;
			}
			else
			{
				// @@ Moock: "true if the string can
				// be converted to a valid nonzero
				// number".
				//
				// Empty string --> false
				return to_number() != 0.0;
			}
		}
		else if (m_type == NUMBER)
		{
			// If m_number_value is NaN, comparison will automatically be false, as it should
			return m_number_value != 0.0;
		}
		else if (m_type == BOOLEAN)
		{
			return this->m_boolean_value;
		}
		else if (m_type == OBJECT)
		{
			return m_object_value != NULL;
		}
		else if (m_type == C_FUNCTION)
		{
			return m_c_function_value != NULL;
		}
		else if (m_type == AS_FUNCTION)
		{
			return m_as_function_value != NULL;
		}
		else
		{
			assert(m_type == UNDEFINED || m_type == NULLTYPE);
			return false;
		}
	}

	
	as_object_interface*	as_value::to_object() const
	// Return value as an object.
	{
		if (m_type == OBJECT)
		{
			// OK.
			return m_object_value;
		}
		else if (m_type == AS_FUNCTION && m_as_function_value != NULL)
		{
			// Make sure this as_function has properties &
			// a prototype object attached to it, since those
			// may be about to be referenced.
			m_as_function_value->lazy_create_properties();
			assert(m_as_function_value->m_properties);

			return m_as_function_value->m_properties;
		}
		else
		{
			return NULL;
		}
	}


	as_c_function_ptr	as_value::to_c_function() const
	// Return value as a C function ptr.  Returns NULL if value is
	// not a C function.
	{
		if (m_type == C_FUNCTION)
		{
			// OK.
			return m_c_function_value;
		}
		else
		{
			return NULL;
		}
	}

	function_as_object*	as_value::to_as_function() const
	// Return value as an ActionScript function.  Returns NULL if value is
	// not an ActionScript function.
	{
		if (m_type == AS_FUNCTION)
		{
			// OK.
			return m_as_function_value;
		}
		else
		{
			return NULL;
		}
	}


	void	as_value::convert_to_number()
	// Force type to number.
	{
		set_double(to_number());
	}


	void	as_value::convert_to_string()
	// Force type to string.
	{
		to_tu_string();	// init our string data.
		m_type = STRING;	// force type.
	}


	void	as_value::convert_to_string_versioned(int version)
	// Force type to string.
	{
		to_tu_string_versioned(version);	// init our string data.
		m_type = STRING;	// force type.
	}


	void	as_value::set_as_object_interface(as_object_interface* obj)
	{
		if (m_type != OBJECT || m_object_value != obj)
		{
			drop_refs();
			m_type = OBJECT;
			m_object_value = obj;
			if (m_object_value)
			{
				m_object_value->add_ref();
			}
		}
	}

	void	as_value::set_function_as_object(function_as_object* func)
	{
		if (m_type != AS_FUNCTION || m_as_function_value != func)
		{
			drop_refs();
			m_type = AS_FUNCTION;
			m_as_function_value = func;
			if (m_as_function_value)
			{
				m_as_function_value->add_ref();
			}
		}
	}


	bool	as_value::operator==(const as_value& v) const
	// Return true if operands are equal.
	{
		bool this_nulltype = (m_type == UNDEFINED || m_type == NULLTYPE);
		bool v_nulltype = (v.get_type() == UNDEFINED || v.get_type() == NULLTYPE);
		if (this_nulltype || v_nulltype)
		{
			return this_nulltype == v_nulltype;
		}
		else if (m_type == STRING)
		{
			return m_string_value == v.to_tu_string();
		}
		else if (m_type == NUMBER)
		{
			return m_number_value == v.to_number();
		}
		else if (m_type == BOOLEAN)
		{
			return m_boolean_value == v.to_bool();
		}
		else
		{
			// Evan: what about objects???
			// TODO
			return m_type == v.m_type;
		}
	}

	
	bool	as_value::operator!=(const as_value& v) const
	// Return true if operands are not equal.
	{
		return ! (*this == v);
	}

	
	void	as_value::string_concat(const tu_string& str)
	// Sets *this to this string plus the given string.
	{
		to_tu_string();	// make sure our m_string_value is initialized
		m_type = STRING;
		m_string_value += str;
	}

	void	as_value::drop_refs()
	// Drop any ref counts we have; this happens prior to changing our value.
	{
		if (m_type == AS_FUNCTION)
		{
			if (m_as_function_value)
			{
				m_as_function_value->drop_ref();
				m_as_function_value = 0;
			}
		}
		else if (m_type == OBJECT)
		{
			if (m_object_value)
			{
				m_object_value->drop_ref();
				m_object_value = 0;
			}
		}
	}


	//
	// as_environment
	//


	as_value	as_environment::get_variable(const tu_string& varname, const array<with_stack_entry>& with_stack) const
	// Return the value of the given var, if it's defined.
	{
		// Path lookup rigamarole.
		movie*	target = m_target;
		tu_string	path;
		tu_string	var;
		if (parse_path(varname, &path, &var))
		{
			target = find_target(path);	// @@ Use with_stack here too???  Need to test.
			if (target)
			{
				as_value	val;
				target->get_member(var, &val);
				return val;
			}
			else
			{
				log_error("find_target(\"%s\") failed\n", path.c_str());
				return as_value();
			}
		}
		else
		{
			return this->get_variable_raw(varname, with_stack);
		}
	}


	as_value	as_environment::get_variable_raw(
		const tu_string& varname,
		const array<with_stack_entry>& with_stack) const
	// varname must be a plain variable name; no path parsing.
	{
		assert(strchr(varname.c_str(), ':') == NULL);
		assert(strchr(varname.c_str(), '/') == NULL);
		assert(strchr(varname.c_str(), '.') == NULL);

		as_value	val;

		// Check the with-stack.
		for (int i = with_stack.size() - 1; i >= 0; i--)
		{
			as_object_interface*	obj = with_stack[i].m_object.get_ptr();
			if (obj && obj->get_member(varname, &val))
			{
				// Found the var in this context.
				return val;
			}
		}

		// Check locals.
		int	local_index = find_local(varname);
		if (local_index >= 0)
		{
			// Get local var.
			return m_local_frames[local_index].m_value;
		}

		// Looking for "this"?
		if (varname == "this")
		{
			val.set_as_object_interface(m_target);
			return val;
		}

		// Check movie members.
		if (m_target->get_member(varname, &val))
		{
			return val;
		}

		// Check built-in constants.
		if (varname == "_root" || varname == "_level0")
		{
			return as_value(m_target->get_root_movie());
		}
		if (varname == "_global")
		{
			return as_value(s_global.get_ptr());
		}
		if (s_global->get_member(varname, &val))
		{
			return val;
		}
	
		// Fallback.
		IF_VERBOSE_ACTION(log_msg("get_variable_raw(\"%s\") failed, returning UNDEFINED.\n", varname.c_str()));
		return as_value();
	}


	void	as_environment::set_variable(
		const tu_string& varname,
		const as_value& val,
		const array<with_stack_entry>& with_stack)
	// Given a path to variable, set its value.
	{
		IF_VERBOSE_ACTION(log_msg("-------------- %s = %s\n", varname.c_str(), val.to_string()));//xxxxxxxxxx

		// Path lookup rigamarole.
		movie*	target = m_target;
		tu_string	path;
		tu_string	var;
		if (parse_path(varname, &path, &var))
		{
			target = find_target(path);
			if (target)
			{
				target->set_member(var, val);
			}
		}
		else
		{
			this->set_variable_raw(varname, val, with_stack);
		}
	}


	void	as_environment::set_variable_raw(
		const tu_string& varname,
		const as_value& val,
		const array<with_stack_entry>& with_stack)
	// No path rigamarole.
	{
		// Check the with-stack.
		for (int i = with_stack.size() - 1; i >= 0; i--)
		{
			as_object_interface*	obj = with_stack[i].m_object.get_ptr();
			as_value	dummy;
			if (obj && obj->get_member(varname, &dummy))
			{
				// This object has the member; so set it here.
				obj->set_member(varname, val);
				return;
			}
		}

		// Check locals.
		int	local_index = find_local(varname);
		if (local_index >= 0)
		{
			// Set local var.
			m_local_frames[local_index].m_value = val;
			return;
		}

		assert(m_target);

		m_target->set_member(varname, val);
	}


	void	as_environment::set_local(const tu_string& varname, const as_value& val)
	// Set/initialize the value of the local variable.
	{
		// Is it in the current frame already?
		int	index = find_local(varname);
		if (index < 0)
		{
			// Not in frame; create a new local var.

			assert(varname.length() > 0);	// null varnames are invalid!
    			m_local_frames.push_back(frame_slot(varname, val));
		}
		else
		{
			// In frame already; modify existing var.
			m_local_frames[index].m_value = val;
		}
	}

	
	void	as_environment::add_local(const tu_string& varname, const as_value& val)
	// Add a local var with the given name and value to our
	// current local frame.  Use this when you know the var
	// doesn't exist yet, since it's faster than set_local();
	// e.g. when setting up args for a function.
	{
		assert(varname.length() > 0);
		m_local_frames.push_back(frame_slot(varname, val));
	}


	void	as_environment::declare_local(const tu_string& varname)
	// Create the specified local var if it doesn't exist already.
	{
		// Is it in the current frame already?
		int	index = find_local(varname);
		if (index < 0)
		{
			// Not in frame; create a new local var.
			assert(varname.length() > 0);	// null varnames are invalid!
    			m_local_frames.push_back(frame_slot(varname, as_value()));
		}
		else
		{
			// In frame already; don't mess with it.
		}
	}

	
	bool	as_environment::get_member(const tu_stringi& varname, as_value* val) const
	{
		return m_variables.get(varname, val);
	}


	void	as_environment::set_member(const tu_stringi& varname, const as_value& val)
	{
		m_variables.set(varname, val);
	}

	as_value*	as_environment::local_register_ptr(int reg)
	// Return a pointer to the specified local register.
	// Local registers are numbered starting with 1.
	//
	// Return value will never be NULL.  If reg is out of bounds,
	// we log an error, but still return a valid pointer (to
	// global reg[0]).  So the behavior is a bit undefined, but
	// not dangerous.
	{
		// We index the registers from the end of the register
		// array, so we don't have to keep base/frame
		// pointers.

		if (reg <= 0 || reg > m_local_register.size())
		{
			log_error("Invalid local register %d, stack only has %d entries\n",
				  reg, m_local_register.size());

			// Fallback: use global 0.
			return &m_global_register[0];
		}

		return &m_local_register[m_local_register.size() - reg];
	}


	int	as_environment::find_local(const tu_string& varname) const
	// Search the active frame for the named var; return its index
	// in the m_local_frames stack if found.
	// 
	// Otherwise return -1.
	{
		// Linear search sucks, but is probably fine for
		// typical use of local vars in script.  There could
		// be pathological breakdowns if a function has tons
		// of locals though.  The ActionScript bytecode does
		// not help us much by using strings to index locals.

		for (int i = m_local_frames.size() - 1; i >= 0; i--)
		{
			const frame_slot&	slot = m_local_frames[i];
			if (slot.m_name.length() == 0)
			{
				// End of local frame; stop looking.
				return -1;
			}
			else if (slot.m_name == varname)
			{
				// Found it.
				return i;
			}
		}
		return -1;
	}


	bool	as_environment::parse_path(const tu_string& var_path, tu_string* path, tu_string* var) const
	// See if the given variable name is actually a sprite path
	// followed by a variable name.  These come in the format:
	//
	// 	/path/to/some/sprite/:varname
	//
	// (or same thing, without the last '/')
	//
	// or
	//	path.to.some.var
	//
	// If that's the format, puts the path part (no colon or
	// trailing slash) in *path, and the varname part (no color)
	// in *var and returns true.
	//
	// If no colon, returns false and leaves *path & *var alone.
	{
		// Search for colon.
		int	colon_index = 0;
		int	var_path_length = var_path.length();
		for ( ; colon_index < var_path_length; colon_index++)
		{
			if (var_path[colon_index] == ':')
			{
				// Found it.
				break;
			}
		}

		if (colon_index >= var_path_length)
		{
			// No colon.  Is there a '.'?  Find the last
			// one, if any.
			for (colon_index = var_path_length - 1; colon_index >= 0; colon_index--)
			{
				if (var_path[colon_index] == '.')
				{
					// Found it.
					break;
				}
			}
			if (colon_index < 0) return false;
		}

		// Make the subparts.

		// Var.
		*var = &var_path[colon_index + 1];

		// Path.
		if (colon_index > 0)
		{
			if (var_path[colon_index - 1] == '/')
			{
				// Trim off the extraneous trailing slash.
				colon_index--;
			}
		}
		// @@ could be better.  This whole usage of tu_string is very flabby...
		*path = var_path;
		path->resize(colon_index);

		return true;
	}


	movie*	as_environment::find_target(const as_value& val) const
	// Find the sprite/movie represented by the given value.  The
	// value might be a reference to the object itself, or a
	// string giving a relative path name to the object.
	{
		if (val.get_type() == as_value::OBJECT)
		{
			if (val.to_object() != NULL)
			{
				return val.to_object()->to_movie();
			}
			else
			{
				return NULL;
			}
		}
		else if (val.get_type() == as_value::STRING)
		{
			return find_target(val.to_tu_string());
		}
		else
		{
			log_error("error: invalid path; neither string nor object");
			return NULL;
		}
	}


	static const char*	next_slash_or_dot(const char* word)
	// Search for next '.' or '/' character in this word.  Return
	// a pointer to it, or to NULL if it wasn't found.
	{
		for (const char* p = word; *p; p++)
		{
			if (*p == '.' && p[1] == '.')
			{
				p++;
			}
			else if (*p == '.' || *p == '/')
			{
				return p;
			}
		}

		return NULL;
	}


	movie*	as_environment::find_target(const tu_string& path) const
	// Find the sprite/movie referenced by the given path.
	{
		if (path.length() <= 0)
		{
			return m_target;
		}

		assert(path.length() > 0);

		movie*	env = m_target;
		assert(env);
		
		const char*	p = path.c_str();
		tu_string	subpart;

		if (*p == '/')
		{
			// Absolute path.  Start at the root.
			env = env->get_relative_target("_level0");
			p++;
		}

		if (*p == '\0')
			return env;

		for (;;)
		{
			const char*	next_slash = next_slash_or_dot(p);
			subpart = p;
			if (next_slash == p)
			{
				log_error("error: invalid path '%s'\n", path.c_str());
				break;
			}
			else if (next_slash)
			{
				// Cut off the slash and everything after it.
				subpart.resize(next_slash - p);
			}

			env = env->get_relative_target(subpart);
			//@@   _level0 --> root, .. --> parent, . --> this, other == character

			if (env == NULL || next_slash == NULL)
			{
				break;
			}

			p = next_slash + 1;
		}
		return env;
	}


	//
	// event_id
	//

	const tu_string&	event_id::get_function_name() const
	{
		static tu_string	s_function_names[EVENT_COUNT] =
		{
			"INVALID",		 // INVALID
			"onPress",		 // PRESS
			"onRelease",		 // RELEASE
			"onRelease_Outside",	 // RELEASE_OUTSIDE
			"onRoll_Over",		 // ROLL_OVER
			"onRoll_Out",		 // ROLL_OUT
			"onDrag_Over",		 // DRAG_OVER
			"onDrag_Out",		 // DRAG_OUT
			"onKeyPress",		 // KEY_PRESS
			"onInitialize",		 // INITIALIZE

			"onLoad",		 // LOAD
			"onUnload",		 // UNLOAD
			"onEnterFrame",		 // ENTER_FRAME
			"onMouseDown",		 // MOUSE_DOWN
			"onMouseUp",		 // MOUSE_UP
			"onMouseMove",		 // MOUSE_MOVE
			"onKeyDown",		 // KEY_DOWN
			"onKeyUp",		 // KEY_UP
			"onData",		 // DATA
			// These are for the MoveClipLoader ActionScript only
			"onLoadStart",		 // LOAD_START
			"onLoadError",		 // LOAD_ERROR
			"onLoadProgress",	 // LOAD_PROGRESS
			"onLoadInit",		 // LOAD_INIT
			// These are for the XMLSocket ActionScript only
			"onSockClose",		 // CLOSE
			"onSockConnect",	 // CONNECT
			"onSockXML",		 // XML
			// These are for the XML ActionScript only
			"onXMLLoad",		 // XML_LOAD
			"onXMLData",		 // XML_DATA
			"onTimer",	         // setInterval Timer expired
		};

		assert(m_id > INVALID && m_id < EVENT_COUNT);
		return s_function_names[m_id];
	}


	// Standard member lookup.
	as_standard_member	get_standard_member(const tu_stringi& name)
	{
		static bool	s_inited = false;
		static stringi_hash<as_standard_member>	s_standard_member_map;
		if (!s_inited)
		{
			s_inited = true;

			s_standard_member_map.set_capacity(int(AS_STANDARD_MEMBER_COUNT));

			s_standard_member_map.add("_x", M_X);
			s_standard_member_map.add("_y", M_Y);
			s_standard_member_map.add("_xscale", M_XSCALE);
			s_standard_member_map.add("_yscale", M_YSCALE);
			s_standard_member_map.add("_currentframe", M_CURRENTFRAME);
			s_standard_member_map.add("_totalframes", M_TOTALFRAMES);
			s_standard_member_map.add("_alpha", M_ALPHA);
			s_standard_member_map.add("_visible", M_VISIBLE);
			s_standard_member_map.add("_width", M_WIDTH);
			s_standard_member_map.add("_height", M_HEIGHT);
			s_standard_member_map.add("_rotation", M_ROTATION);
			s_standard_member_map.add("_target", M_TARGET);
			s_standard_member_map.add("_framesloaded", M_FRAMESLOADED);
			s_standard_member_map.add("_name", M_NAME);
			s_standard_member_map.add("_droptarget", M_DROPTARGET);
			s_standard_member_map.add("_url", M_URL);
			s_standard_member_map.add("_highquality", M_HIGHQUALITY);
			s_standard_member_map.add("_focusrect", M_FOCUSRECT);
			s_standard_member_map.add("_soundbuftime", M_SOUNDBUFTIME);
			s_standard_member_map.add("_xmouse", M_XMOUSE);
			s_standard_member_map.add("_ymouse", M_YMOUSE);
			s_standard_member_map.add("_parent", M_PARENT);
			s_standard_member_map.add("text", M_TEXT);
			s_standard_member_map.add("textWidth", M_TEXTWIDTH);
			s_standard_member_map.add("textColor", M_TEXTCOLOR);
			s_standard_member_map.add("onLoad", M_ONLOAD);
		}

		as_standard_member	result = M_INVALID_MEMBER;
		s_standard_member_map.get(name, &result);

		return result;
	}


	//
	// Disassembler
	//


#define COMPILE_DISASM 1

#ifndef COMPILE_DISASM

	void	log_disasm(const unsigned char* instruction_data)
	// No disassembler in this version...
	{
		log_msg("<no disasm>\n");
	}

#else // COMPILE_DISASM

	void	log_disasm(const unsigned char* instruction_data)
	// Disassemble one instruction to the log.
	{
		enum arg_format {
			ARG_NONE = 0,
			ARG_STR,
			ARG_HEX,	// default hex dump, in case the format is unknown or unsupported
			ARG_U8,
			ARG_U16,
			ARG_S16,
			ARG_PUSH_DATA,
			ARG_DECL_DICT,
			ARG_FUNCTION2
		};
		struct inst_info
		{
			int	m_action_id;
			const char*	m_instruction;

			arg_format	m_arg_format;
		};

		static inst_info	s_instruction_table[] = {
			{ SWF::ACTION_NEXTFRAME, "next_frame", ARG_NONE },
			{ SWF::ACTION_PREVFRAME, "prev_frame", ARG_NONE },
			{ SWF::ACTION_PLAY, "play", ARG_NONE },
			{ SWF::ACTION_STOP, "stop", ARG_NONE },
			{ SWF::ACTION_TOGGLEQUALITY, "toggle_qlty", ARG_NONE },
			{ SWF::ACTION_STOPSOUNDS, "stop_sounds", ARG_NONE },
			{ SWF::ACTION_ADD, "add", ARG_NONE },
			{ SWF::ACTION_SUBTRACT, "sub", ARG_NONE },
			{ SWF::ACTION_MULTIPLY, "mul", ARG_NONE },
			{ SWF::ACTION_DIVIDE, "div", ARG_NONE },
			{ SWF::ACTION_EQUAL, "equ", ARG_NONE },
			{ SWF::ACTION_LESSTHAN, "lt", ARG_NONE },
			{ SWF::ACTION_LOGICALAND, "and", ARG_NONE },
			{ SWF::ACTION_LOGICALOR, "or", ARG_NONE },
			{ SWF::ACTION_LOGICALNOT, "not", ARG_NONE },
			{ SWF::ACTION_STRINGEQ, "str_eq", ARG_NONE },
			{ SWF::ACTION_STRINGLENGTH, "str_len", ARG_NONE },
			{ SWF::ACTION_SUBSTRING, "substr", ARG_NONE },
			{ SWF::ACTION_POP, "pop", ARG_NONE },
			{ SWF::ACTION_INT, "floor", ARG_NONE },
			{ SWF::ACTION_GETVARIABLE, "get_var", ARG_NONE },
			{ SWF::ACTION_SETVARIABLE, "set_var", ARG_NONE },
			{ SWF::ACTION_SETTARGETEXPRESSION, "set_target_exp", ARG_NONE },
			{ SWF::ACTION_STRINGCONCAT, "str_cat", ARG_NONE },
			{ SWF::ACTION_GETPROPERTY, "get_prop", ARG_NONE },
			{ SWF::ACTION_SETPROPERTY, "set_prop", ARG_NONE },
			{ SWF::ACTION_DUPLICATECLIP, "dup_sprite", ARG_NONE },
			{ SWF::ACTION_REMOVECLIP, "rem_sprite", ARG_NONE },
			{ SWF::ACTION_TRACE, "trace", ARG_NONE },
			{ SWF::ACTION_STARTDRAGMOVIE, "start_drag", ARG_NONE },
			{ SWF::ACTION_STOPDRAGMOVIE, "stop_drag", ARG_NONE },
			{ SWF::ACTION_STRINGCOMPARE, "str_lt", ARG_NONE },
			{ SWF::ACTION_THROW, "throw_fixme", ARG_NONE },
			{ SWF::ACTION_CASTOP, "cast_fixme", ARG_NONE },
			{ SWF::ACTION_IMPLEMENTSOP, "implements_fixme", ARG_NONE },
			{ SWF::ACTION_RANDOM, "random", ARG_NONE },
			{ SWF::ACTION_MBLENGTH, "mb_length_fixme", ARG_NONE },
			{ SWF::ACTION_ORD, "ord", ARG_NONE },
			{ SWF::ACTION_CHR, "chr", ARG_NONE },
			{ SWF::ACTION_GETTIMER, "get_timer", ARG_NONE },
			{ SWF::ACTION_MBSUBSTRING, "substr_mb_fixme", ARG_NONE },
			{ SWF::ACTION_MBORD, "ord_mb", ARG_NONE },
			{ SWF::ACTION_MBCHR, "chr_mb_fixme", ARG_NONE },
			{ SWF::ACTION_DELETEVAR, "delete_fixme", ARG_NONE },
			{ SWF::ACTION_DELETE, "delete_all_fixme", ARG_NONE },
			{ SWF::ACTION_VAREQUALS, "set_local", ARG_NONE },
			{ SWF::ACTION_CALLFUNCTION, "call_func", ARG_NONE },
			{ SWF::ACTION_RETURN, "return", ARG_NONE },
			{ SWF::ACTION_MODULO, "mod", ARG_NONE },
			{ SWF::ACTION_NEW, "new", ARG_NONE },
			{ SWF::ACTION_VAR, "decl_local", ARG_NONE },
			{ SWF::ACTION_INITARRAY, "decl_array", ARG_NONE },
			{ SWF::ACTION_INITOBJECT, "decl_obj_fixme", ARG_NONE },
			{ SWF::ACTION_TYPEOF, "type_of", ARG_NONE },
			{ SWF::ACTION_TARGETPATH, "get_target_fixme", ARG_NONE },
			{ SWF::ACTION_ENUMERATE, "enumerate", ARG_NONE },
			{ SWF::ACTION_NEWADD, "add_t", ARG_NONE },
			{ SWF::ACTION_NEWLESSTHAN, "lt_t", ARG_NONE },
			{ SWF::ACTION_NEWEQUALS, "eq_t", ARG_NONE },
			{ SWF::ACTION_TONUMBER, "number", ARG_NONE },
			{ SWF::ACTION_TOSTRING, "string", ARG_NONE },
			{ SWF::ACTION_DUP, "dup", ARG_NONE },
			{ SWF::ACTION_SWAP, "swap", ARG_NONE },
			{ SWF::ACTION_GETMEMBER, "get_member", ARG_NONE },
			{ SWF::ACTION_SETMEMBER, "set_member", ARG_NONE },
			{ SWF::ACTION_INCREMENT, "inc", ARG_NONE },
			{ SWF::ACTION_DECREMENT, "dec", ARG_NONE },
			{ SWF::ACTION_CALLMETHOD, "call_method", ARG_NONE },
			{ SWF::ACTION_NEWMETHOD, "new_method_fixme", ARG_NONE },
			{ SWF::ACTION_INSTANCEOF, "is_inst_of_fixme", ARG_NONE },
			{ SWF::ACTION_ENUM2, "enum_object_fixme", ARG_NONE },
			{ SWF::ACTION_BITWISEAND, "bit_and", ARG_NONE },
			{ SWF::ACTION_BITWISEOR, "bit_or", ARG_NONE },
			{ SWF::ACTION_BITWISEXOR, "bit_xor", ARG_NONE },
			{ SWF::ACTION_SHIFTLEFT, "shl", ARG_NONE },
			{ SWF::ACTION_SHIFTRIGHT, "asr", ARG_NONE },
			{ SWF::ACTION_SHIFTRIGHT2, "lsr", ARG_NONE },
			{ SWF::ACTION_STRICTEQ, "eq_strict", ARG_NONE },
			{ SWF::ACTION_GREATER, "gt_t", ARG_NONE },
			{ SWF::ACTION_EXTENDS, "extends_fixme", ARG_NONE },
			{ SWF::ACTION_GOTOFRAME, "goto_frame", ARG_U16 },
			{ SWF::ACTION_GETURL, "get_url", ARG_STR },
			{ SWF::ACTION_SETREGISTER, "store_register", ARG_U8 },
			{ SWF::ACTION_CONSTANTPOOL, "decl_dict", ARG_DECL_DICT },
			{ SWF::ACTION_WAITFORFRAME, "wait_for_frame", ARG_HEX },
			{ SWF::ACTION_SETTARGET, "set_target", ARG_STR },
			{ SWF::ACTION_GOTOLABEL, "goto_frame_lbl", ARG_STR },
			{ SWF::ACTION_WAITFORFRAMEEXPRESSION, "wait_for_fr_exp", ARG_HEX },
			{ SWF::ACTION_DEFINEFUNCTION2, "function2", ARG_FUNCTION2 },
			{ SWF::ACTION_TRY, "try_fixme", ARG_FUNCTION2 },
			{ SWF::ACTION_WITH, "with", ARG_U16 },
			{ SWF::ACTION_PUSHDATA, "push_data", ARG_PUSH_DATA },
			{ SWF::ACTION_BRANCHALWAYS, "goto", ARG_S16 },
			{ SWF::ACTION_GETURL2, "get_url2", ARG_HEX },
			{ SWF::ACTION_DEFINEFUNCTION, "func", ARG_HEX },
			{ SWF::ACTION_BRANCHIFTRUE, "branch_if_true", ARG_S16 },
			{ SWF::ACTION_CALLFRAME, "call_frame", ARG_HEX },
			{ SWF::ACTION_GOTOEXPRESSION, "goto_frame_exp", ARG_HEX },
			{ SWF::ACTION_END, "<end>", ARG_NONE }
		};

		int	action_id = instruction_data[0];
		inst_info*	info = NULL;

		for (int i = 0; ; i++)
		{
			if (s_instruction_table[i].m_action_id == action_id)
			{
				info = &s_instruction_table[i];
			}

			if (s_instruction_table[i].m_action_id == 0)
			{
				// Stop at the end of the table and give up.
				break;
			}
		}

		arg_format	fmt = ARG_HEX;

		// Show instruction.
		if (info == NULL)
		{
			log_msg("<unknown>[0x%02X]", action_id);
		}
		else
		{
			log_msg("%-15s", info->m_instruction);
			fmt = info->m_arg_format;
		}

		// Show instruction argument(s).
		if (action_id & 0x80)
		{
			assert(fmt != ARG_NONE);

			int	length = instruction_data[1] | (instruction_data[2] << 8);

			// log_msg(" [%d]", length);

			if (fmt == ARG_HEX)
			{
				for (int i = 0; i < length; i++)
				{
					log_msg(" 0x%02X", instruction_data[3 + i]);
				}
				log_msg("\n");
			}
			else if (fmt == ARG_STR)
			{
				log_msg(" \"");
				for (int i = 0; i < length; i++)
				{
					log_msg("%c", instruction_data[3 + i]);
				}
				log_msg("\"\n");
			}
			else if (fmt == ARG_U8)
			{
				int	val = instruction_data[3];
				log_msg(" %d\n", val);
			}
			else if (fmt == ARG_U16)
			{
				int	val = instruction_data[3] | (instruction_data[4] << 8);
				log_msg(" %d\n", val);
			}
			else if (fmt == ARG_S16)
			{
				int	val = instruction_data[3] | (instruction_data[4] << 8);
				if (val & 0x8000) val |= ~0x7FFF;	// sign-extend
				log_msg(" %d\n", val);
			}
			else if (fmt == ARG_PUSH_DATA)
			{
				log_msg("\n");
				int i = 0;
				while (i < length)
				{
					int	type = instruction_data[3 + i];
					i++;
					log_msg("\t\t");	// indent
					if (type == 0)
					{
						// string
						log_msg("\"");
						while (instruction_data[3 + i])
						{
							log_msg("%c", instruction_data[3 + i]);
							i++;
						}
						i++;
						log_msg("\"\n");
					}
					else if (type == 1)
					{
						// float (little-endian)
						union {
							float	f;
							Uint32	i;
						} u;
						compiler_assert(sizeof(u) == sizeof(u.i));

						memcpy(&u.i, instruction_data + 3 + i, 4);
						u.i = swap_le32(u.i);
						i += 4;

						log_msg("(float) %f\n", u.f);
					}
					else if (type == 2)
					{
						log_msg("NULL\n");
					}
					else if (type == 3)
					{
						log_msg("undef\n");
					}
					else if (type == 4)
					{
						// contents of register
						int	reg = instruction_data[3 + i];
						i++;
						log_msg("reg[%d]\n", reg);
					}
					else if (type == 5)
					{
						int	bool_val = instruction_data[3 + i];
						i++;
						log_msg("bool(%d)\n", bool_val);
					}
					else if (type == 6)
					{
						// double
						// wacky format: 45670123
						union {
							double	d;
							Uint64	i;
							struct {
								Uint32	lo;
								Uint32	hi;
							} sub;
						} u;
						compiler_assert(sizeof(u) == sizeof(u.i));

						memcpy(&u.sub.hi, instruction_data + 3 + i, 4);
						memcpy(&u.sub.lo, instruction_data + 3 + i + 4, 4);
						u.i = swap_le64(u.i);
						i += 8;

						log_msg("(double) %f\n", u.d);
					}
					else if (type == 7)
					{
						// int32
						Sint32	val = instruction_data[3 + i]
							| (instruction_data[3 + i + 1] << 8)
							| (instruction_data[3 + i + 2] << 16)
							| (instruction_data[3 + i + 3] << 24);
						i += 4;
						log_msg("(int) %d\n", val);
					}
					else if (type == 8)
					{
						int	id = instruction_data[3 + i];
						i++;
						log_msg("dict_lookup[%d]\n", id);
					}
					else if (type == 9)
					{
						int	id = instruction_data[3 + i] | (instruction_data[3 + i + 1] << 8);
						i += 2;
						log_msg("dict_lookup_lg[%d]\n", id);
					}
				}
			}
			else if (fmt == ARG_DECL_DICT)
			{
				int	i = 0;
				int	count = instruction_data[3 + i] | (instruction_data[3 + i + 1] << 8);
				i += 2;

				log_msg(" [%d]\n", count);

				// Print strings.
				for (int ct = 0; ct < count; ct++)
				{
					log_msg("\t\t");	// indent

					log_msg("\"");
					while (instruction_data[3 + i])
					{
						// safety check.
						if (i >= length)
						{
							log_msg("<disasm error -- length exceeded>\n");
							break;
						}

						log_msg("%c", instruction_data[3 + i]);
						i++;
					}
					log_msg("\"\n");
					i++;
				}
			}
			else if (fmt == ARG_FUNCTION2)
			{
				// Signature info for a function2 opcode.
				int	i = 0;
				const char*	function_name = (const char*) &instruction_data[3 + i];
				i += strlen(function_name) + 1;

				int	arg_count = instruction_data[3 + i] | (instruction_data[3 + i + 1] << 8);
				i += 2;

				int	reg_count = instruction_data[3 + i];
				i++;

				log_msg("\n\t\tname = '%s', arg_count = %d, reg_count = %d\n",
					function_name, arg_count, reg_count);

				uint16	flags = (instruction_data[3 + i]) | (instruction_data[3 + i + 1] << 8);
				i += 2;

				// @@ What is the difference between "super" and "_parent"?

				bool	preload_global = (flags & 0x100) != 0;
				bool	preload_parent = (flags & 0x80) != 0;
				bool	preload_root   = (flags & 0x40) != 0;
				bool	suppress_super = (flags & 0x20) != 0;
				bool	preload_super  = (flags & 0x10) != 0;
				bool	suppress_args  = (flags & 0x08) != 0;
				bool	preload_args   = (flags & 0x04) != 0;
				bool	suppress_this  = (flags & 0x02) != 0;
				bool	preload_this   = (flags & 0x01) != 0;

				log_msg("\t\t        pg = %d\n"
					"\t\t        pp = %d\n"
					"\t\t        pr = %d\n"
					"\t\tss = %d, ps = %d\n"
					"\t\tsa = %d, pa = %d\n"
					"\t\tst = %d, pt = %d\n",
					int(preload_global),
					int(preload_parent),
					int(preload_root),
					int(suppress_super),
					int(preload_super),
					int(suppress_args),
					int(preload_args),
					int(suppress_this),
					int(preload_this));

				for (int argi = 0; argi < arg_count; argi++)
				{
					int	arg_register = instruction_data[3 + i];
					i++;
					const char*	arg_name = (const char*) &instruction_data[3 + i];
					i += strlen(arg_name) + 1;

					log_msg("\t\targ[%d] - reg[%d] - '%s'\n", argi, arg_register, arg_name);
				}

				int	function_length = instruction_data[3 + i] | (instruction_data[3 + i + 1] << 8);
				i += 2;

				log_msg("\t\tfunction length = %d\n", function_length);
			}
		}
		else
		{
			log_msg("\n");
		}
	}

#endif // COMPILE_DISASM


};


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
