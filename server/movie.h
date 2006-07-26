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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//
//

#ifndef GNASH_MOVIE_H
#define GNASH_MOVIE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gnash.h"
//#include "action.h"
#include "types.h"
#include "log.h"
#include "container.h"
#include "utility.h"
#include "smart_ptr.h"
#include "movie_interface.h" // for inheritance
#include "action.h" // for event_id definitions

#include <cstdarg>
#include <cassert>

namespace gnash {

class movie_root;
struct swf_event;

/// \brief
/// @@@ To be obsoleted. This class is only confusing.
struct movie : public movie_interface
{
	virtual void set_extern_movie(movie_interface* /* m */)
	{
	}

	virtual movie_interface *get_extern_movie()
	{
		return NULL;
	}

	virtual movie_definition *get_movie_definition()
	{
		return NULL;
	}

	virtual movie_root *get_root()
	{
		return NULL;
	}

	virtual movie_interface *get_root_interface()
	{
		return NULL;
	}

	virtual sprite_instance *get_root_movie()
	{
		return NULL;
	}

	virtual float get_pixel_scale() const
	{
		return 1.0f;
	}

	virtual character *get_character(int /* id */)
	{
		return NULL;
	}

	virtual matrix get_world_matrix() const
	{
		return matrix::identity;
	}

	virtual cxform get_world_cxform() const
	{
		return cxform::identity;
	}

	//
	// display-list management.
	//

	virtual execute_tag *find_previous_replace_or_add_tag(
			int /* current_frame */,
			int /* depth */,
			int /* id */)
	{
	    return NULL;
	}

	virtual character*	add_display_object(
		uint16_t 		/* character_id */ , 
		const char*		/* name */ ,
		const std::vector<swf_event*>& /* event_handlers */ ,
		uint16_t		/* depth */ ,
		bool			/* replace_if_depth_is_occupied */ ,
		const cxform&		/* color_transform */ ,
		const matrix&		/* mat */ ,
		float			/* ratio  */ ,
		uint16_t		/* clip_depth */)
	{
	    return NULL;
	}

	virtual void	move_display_object(
		uint16_t	/* depth */ ,
		bool		/* use_cxform */ ,
		const cxform&	/* color_transform */ ,
		bool		/* use_matrix */ ,
		const matrix&	/* mat */ ,
		float		/* ratio */ ,
		uint16_t	/* clip_depth */ )
	{
	}

	virtual void	replace_display_object(
		uint16_t	/* character_id */ ,
		const char*	/* name */ ,
		uint16_t	/* depth */ ,
		bool		/* use_cxform */ ,
		const cxform&	/* color_transform */ ,
		bool		/* use_matrix */ ,
		const matrix&	/* mat */ ,
		float		/* ratio */ ,
		uint16_t	/* clip_depth */ )
	{
	}

	virtual void	replace_display_object(
		character*	/* ch */ ,
		const char*	/* name */ ,
		uint16_t	/* depth */ ,
		bool		/* use_cxform */ ,
		const cxform&	/* color_transform */ ,
		bool		/* use_matrix */ ,
		const matrix&	/* mat */ ,
		float		/* ratio */ ,
		uint16_t	/* clip_depth */ )
	{
	}

	virtual void	remove_display_object(uint16_t /*depth*/, int /*id*/)
	{
	}

	virtual void	set_background_color(const rgba& /*color*/)
	{
	}

	virtual void	set_background_alpha(float /*alpha*/)
	{
	}

	virtual float	get_background_alpha() const
	{
		return 1.0f;
	}

	virtual void	set_display_viewport(int /*x0*/, int /*y0*/,
				int /*width*/, int /*height*/)
	{
	}

	virtual void add_action_buffer(action_buffer* /*a*/)
	{
		assert(0);
	}

	virtual void goto_frame(int /*target_frame_number*/)
	{
		assert(0);
	}

	virtual bool	goto_labeled_frame(const char* /*label*/)
	{
		assert(0);
		return false;
	}

	virtual void set_play_state(play_state /*s*/)
	{
	}

	virtual play_state get_play_state() const
	{
		assert(0);
		return STOP;
	}

	/// \brief
        /// The host app can use this to tell the movie when
        /// user's mouse pointer has moved.
        virtual void notify_mouse_moved(int /*x*/, int /*y*/)
        {
	    GNASH_REPORT_FUNCTION;
        }

	/// \brief
        /// The host app can use this to tell the movie when a
        /// button on the user's mouse has been pressed or released.
        /// Set mouse_pressed to true on click, false on release.
        virtual void notify_mouse_clicked(bool /*mouse_pressed*/, int /*mask*/)
        {
	    GNASH_REPORT_FUNCTION;
        }
	/// \brief
	/// The host app uses this to tell the movie where the
	/// user's mouse pointer is.
	virtual void notify_mouse_state(int /*x*/, int /*y*/, int /*buttons*/)
	{
	    GNASH_REPORT_FUNCTION;
	}

	/// \brief
	/// Use this to retrieve the last state of the mouse, as set via
	/// notify_mouse_state().
	virtual void get_mouse_state(int* /*x*/, int* /*y*/, int* /*buttons*/)
	{
	    assert(0);
	}

	struct drag_state
	{
		movie*	m_character;
		bool	m_lock_center;
		bool	m_bound;
		float	m_bound_x0;
		float	m_bound_y0;
		float	m_bound_x1;
		float	m_bound_y1;

		drag_state()
			:
			m_character(0), m_lock_center(0), m_bound(0),
			m_bound_x0(0), m_bound_y0(0), m_bound_x1(1),
			m_bound_y1(1)
		{
		}
	};

	virtual void	get_drag_state(drag_state* /* st */)
	{
		assert(0);
		// *st = drag_state(); 
	}

	virtual void set_drag_state(const drag_state& /* st */ )
	{
		assert(0);
	}

	virtual void stop_drag()
	{
		assert(0);
	}

	// External
	virtual void set_variable(const char* /* path_to_var */,
			const char* /* new_value */)
	{
	    assert(0);
	}

	// External
	virtual void set_variable(const char* /* path_to_var */,
			const wchar_t* /* new_value */)
	{
	    assert(0);
	}

	// External
	virtual const char* get_variable(const char* /* path_to_var */ ) const
	{
	    assert(0);
	    return "";
	}

	virtual void * get_userdata()
	{
		assert(0);
		return NULL;
	}

	virtual void set_userdata(void *)
	{
		assert(0);
	}

	// External
	virtual bool has_looped() const
	{
		// @@ why true ? shouldn't we assert(0) instead ?
		return true;
	}


	//
	// Mouse/Button interface.
	//

	virtual movie* get_topmost_mouse_entity(float /* x */, float /* y */)
	{
		return NULL;
	}

	virtual bool get_track_as_menu() const
	{
		return false;
	}

	virtual void on_button_event(event_id id)
	{
		on_event(id);
	}


	//
	// ActionScript.
	//


#if 0
	virtual movie* get_relative_target(const tu_string& /* name */)
	{
	    assert(0);	
	    return NULL;
	}
#endif

	/// ActionScript event handler.  Returns true if a handler was called.
	//
	/// Must be overridden or will always return false.
	///
	virtual bool on_event(event_id /* id */)
	{
		return false;
	}

	virtual void get_url(const char* /* url */)
	{
		GNASH_REPORT_FUNCTION;
	}
	    
	    
	int add_interval_timer(void* /* timer */)
	{
	    log_msg("FIXME: %s: unimplemented\n", __FUNCTION__);
	    return -1;	// ???
	}
		
	void clear_interval_timer(int /* x */)
	{
	    log_msg("FIXME: %s: unimplemented\n", __FUNCTION__);
	}
		
	virtual void do_something(void* /* timer */)
	{
	    log_msg("FIXME: %s: unimplemented\n", __FUNCTION__);
	}
		
	/// \brief
	/// Special event handler; sprites also execute their frame1
	/// actions on this event.
	//
	///

//	virtual void	on_event_load()
//	{
//		on_event(event_id::LOAD);
//	}

#if 0
	// as_object_interface stuff
	virtual void set_member(
			const tu_stringi& /* name */,
			const as_value& /* val */ )
	{
		assert(0);
	}

	virtual bool get_member(
			const tu_stringi& /* name */,
			as_value* /* val */ )
	{
		assert(0);
		return false;
	}
#endif


	virtual void call_frame_actions(const as_value& /* frame_spec */)
	{
		assert(0);
	}

	virtual float get_timer() const
	{
		return 0.0f;
	}

	virtual movie* to_movie()
	{
		return this;
	}

	virtual void clone_display_object(
			const tu_string& /* name */,
			const tu_string& /* newname */,
			uint16_t /* depth */ )
	{
		assert(0);
	}

	virtual void remove_display_object(const tu_string& /* name */)
	{
		assert(0);
	}

	// Forward vararg call to version taking va_list.
	virtual const char* call_method(
			const char* method_name,
			const char* method_arg_fmt, ...)
	{
	    va_list	args;
	    va_start(args, method_arg_fmt);
	    const char*	result = call_method_args(method_name, method_arg_fmt, args);
	    va_end(args);

	    return result;
	}

	/// Override this if you implement call_method.
	virtual const char* call_method_args(
			const char* /* method_name */,
			const char* /* method_arg_fmt */,
			va_list /* args */)
	{
	    assert(0);
	    return NULL;
	}

	virtual void execute_frame_tags(
			int /* frame */, bool state_only = false)
	{
	}

	// External.
	virtual void attach_display_callback(
			const char* /* path_to_object */,
			void (* /*callback*/)(void*),
			void* /* user_ptr */)
	{
	    assert(0);
	}

	// Override me to provide this functionality.
	virtual void set_display_callback(
			void (*callback)(void*),
			void* /* user_ptr */)
	{
	}

};


}	// end namespace gnash


#endif // GNASH_MOVIE_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
