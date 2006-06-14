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

// Stateful live Sprite instance


#ifndef GNASH_SPRITE_INSTANCE_H
#define GNASH_SPRITE_INSTANCE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <vector>

#include "movie_definition.h"
#include "movie_root.h"
#include "dlist.h" // DisplayList 
#include "stream.h"
#include "log.h"
#include "as_environment.h" // for composition

namespace gnash
{
	// Forward declarations
	//struct sprite_instance;
	//struct sprite_definition;

/// Stateful Sprite object. Also known as a MovieClip.
class sprite_instance : public character
{

public:

	sprite_instance(movie_definition* def,
		movie_root* r, movie* parent, int id);

	virtual ~sprite_instance();


	enum mouse_state
	{
		UP = 0,
		DOWN,
		OVER
	};

	// sprite instance of add_interval_handler()
	virtual int    add_interval_timer(void *timer)
        {
            return m_root->add_interval_timer(timer);
        }

	virtual void  clear_interval_timer(int x)
        {
            m_root->clear_interval_timer(x);
        }
	

	/// Interval timer timeout executor
	virtual void    do_something(void *timer);

	movie_interface* get_root_interface() {
		return m_root;
	}

	movie_root* get_root() {
		return m_root;
	}

	movie*	get_root_movie() {
                return m_root->get_root_movie();
        }

        movie_definition* get_movie_definition() {
                return m_def.get_ptr();
        }

	float get_width();

	float get_height();

	int get_current_frame() const
	{
		return m_current_frame;
	}

	int get_frame_count() const
	{
		return m_def->get_frame_count();
	}

	/// Stop or play the sprite.
	void set_play_state(play_state s)
	{
	    if (m_play_state != s) m_time_remainder = 0;
	    m_play_state = s;
	}

	play_state get_play_state() const { return m_play_state; }

	character* get_character(int character_id);

	float	get_background_alpha() const
	{
	    // @@ this doesn't seem right...
	    return m_root->get_background_alpha();
	}

	float	get_pixel_scale() const
	{
		return m_root->get_pixel_scale();
	}

	virtual void	get_mouse_state(int* x, int* y, int* buttons)
	{
	    m_root->get_mouse_state(x, y, buttons);
	}

	void	set_background_color(const rgba& color)
	{
	    m_root->set_background_color(color);
	}

	float	get_timer() const { return m_root->get_timer(); }

	void	restart();


	virtual bool has_looped() const
	{
		return m_has_looped;
	}

	virtual bool get_accept_anim_moves() const
	{
		return m_accept_anim_moves;
	}

	/// Combine the flags to avoid a conditional.
	/// It would be faster with a macro.
	inline int transition(int a, int b) const
	{
	    return (a << 2) | b;
	}


	/// Return true if we have any mouse event handlers.
	bool can_handle_mouse_event();

	/// \brief
	/// Return the topmost entity that the given point
	/// covers that can receive mouse events.  NULL if
	/// none.  Coords are in parent's frame.
	virtual movie*	get_topmost_mouse_entity(float x, float y);

	virtual void	advance(float delta_time);
	virtual void	advance_root(float delta_time);
	virtual void	advance_sprite(float delta_time);

	/// Execute the tags associated with the specified frame.
	/// frame is 0-based
	void execute_frame_tags(int frame, bool state_only = false);


	/// Execute the tags associated with the specified frame,
	/// IN REVERSE.
	/// I.e. if it's an "add" tag, then we do a "remove" instead.
	/// Only relevant to the display-list manipulation tags:
	/// add, move, remove, replace.
	///
	/// frame is 0-based
	void execute_frame_tags_reverse(int frame);

		
	execute_tag* find_previous_replace_or_add_tag(int frame,
		int depth, int id);


	/// Execute any remove-object tags associated with
	/// the specified frame.
	/// frame is 0-based
	void	execute_remove_tags(int frame);


	/// Take care of this frame's actions.
	void do_actions();


	/// Set the sprite state at the specified frame number.
	//
	/// 0-based frame numbers!! 
	///(in contrast to ActionScript and Flash MX)
	///
	void	goto_frame(int target_frame_number);


	/// Look up the labeled frame, and jump to it.
	bool goto_labeled_frame(const char* label);

		
	/// Display (render?) this Sprite/MovieClip, unless invisible
	void	display();

	/// Add an object to the DisplayList. 
	//
	/// @param replace_if_dept_is_occupied
	///	unused, always true
	///       
	character* add_display_object(
		uint16_t character_id,
		const char* name,
		const std::vector<swf_event*>& event_handlers,
		uint16_t depth,
		bool replace_if_depth_is_occupied,
		const cxform& color_transform,
		const matrix& matrix,
		float ratio,
		uint16_t clip_depth);


	/// Updates the transform properties of the object at
	/// the specified depth.
	void	move_display_object(
			uint16_t depth,
			bool use_cxform,
			const cxform& color_xform,
			bool use_matrix,
			const matrix& mat,
			float ratio,
			uint16_t clip_depth)
	{
	    m_display_list.move_display_object(depth, use_cxform, color_xform, use_matrix, mat, ratio, clip_depth);
	}


	void	replace_display_object(
			uint16_t character_id,
			const char* name,
			uint16_t depth,
			bool use_cxform,
			const cxform& color_transform,
			bool use_matrix,
			const matrix& mat,
			float ratio,
			uint16_t clip_depth);


	void	replace_display_object(
			character* ch,
			const char* name,
			uint16_t depth,
			bool use_cxform,
			const cxform& color_transform,
			bool use_matrix,
			const matrix& mat,
			float ratio,
			uint16_t clip_depth);


	/// \brief
	/// Remove the object at the specified depth.
	//
	/// NOTE: the id parameter is unused, but currently
	/// required to avoid break of inheritance from movie.h
	///
	void	remove_display_object(uint16_t depth, int id)
	{
	    m_display_list.remove_display_object(depth);
	}


	/// Add the given action buffer to the list of action
	/// buffers to be processed at the end of the next
	/// frame advance.
	void	add_action_buffer(action_buffer* a)
	{
	    m_action_list.push_back(a);
	}


	/// For debugging -- return the id of the character
	/// at the specified depth.
	/// Return -1 if nobody's home.
	int	get_id_at_depth(int depth);


	//
	// ActionScript support
	//


	/// Set the named variable to the value
	virtual void set_variable(const char* path_to_var,
		const char* new_value);

	/// Set the named variable to the wide value
	virtual void set_variable(const char* path_to_var,
		const wchar_t* new_value);

	/// Returns address to static buffer. NOT THREAD SAFE!
	virtual const char* get_variable(const char* path_to_var) const;

	// Set *val to the value of the named member and
	// return true, if we have the named member.
	// Otherwise leave *val alone and return false.
	bool get_member(const tu_stringi& name, as_value* val);

		
	/// Set the named member to the value. 
	//
	/// Return true if we have
	/// that member; false otherwise.
	virtual void set_member(const tu_stringi& name,
		const as_value& val);


	/// Find the movie which is one degree removed from us,
	/// given the relative pathname.
	///
	/// If the pathname is "..", then return our parent.
	/// If the pathname is ".", then return ourself.  If
	/// the pathname is "_level0" or "_root", then return
	/// the root movie.
	///
	/// Otherwise, the name should refer to one our our
	/// named characters, so we return it.
	///
	/// NOTE: In ActionScript 2.0, top level names (like
	/// "_root" and "_level0") are CASE SENSITIVE.
	/// Character names in a display list are CASE
	/// SENSITIVE. Member names are CASE INSENSITIVE.  Gah.
	///
	/// In ActionScript 1.0, everything seems to be CASE
	/// INSENSITIVE.
	virtual movie*	get_relative_target(const tu_string& name);


	/// Execute the actions for the specified frame. 
	//
	/// The frame_spec could be an integer or a string.
	///
	virtual void call_frame_actions(const as_value& frame_spec);


	virtual void set_drag_state(const drag_state& st) {
	    m_root->m_drag_state = st;
	}

	virtual void stop_drag() {
	    assert(m_parent == NULL);	// we must be the root movie!!!
			
	    m_root->stop_drag();
	}

	/* sprite_instance */
	virtual void	get_drag_state(drag_state* st)
	{
	    *st = m_root->m_drag_state;
	}


	/// Duplicate the object with the specified name
	/// and add it with a new name  at a new depth.
	void clone_display_object(const tu_string& name,
		const tu_string& newname, uint16_t depth);

	/// Remove the object with the specified name.
	//
	/// @@ what happens if the we have multiple objects
	///    with the same name ?
	//void remove_display_object(const tu_string& name);

	/// Dispatch event handler(s), if any.
	virtual bool	on_event(event_id id);


	/// Do the events that (appear to) happen as the movie
	/// loads.  frame1 tags and actions are executed (even
	/// before advance() is called).  Then the onLoad event
	/// is triggered.

//	virtual void	on_event_load()
//	{
//	    execute_frame_tags(0);
//	    do_actions();
//	    on_event(event_id::LOAD);
//	}

	/// Do the events that happen when there is XML data waiting
	/// on the XML socket connection.
	/// FIXME: unimplemented
	virtual void	on_event_xmlsocket_onxml()
	{
	    log_msg("FIXME: %s: unimplemented\n", __FUNCTION__);
	    on_event(event_id::SOCK_XML);
	}
		
	/// Do the events that (appear to) happen on a
	/// specified interval.
	virtual void	on_event_interval_timer()
	{
	    log_msg("FIXME: %s: unimplemented\n", __FUNCTION__);
	    on_event(event_id::TIMER);
	}

	/// Do the events that happen as a MovieClip (swf 7 only) loads.
	virtual void	on_event_load_progress()
	{
	    log_msg("FIXME: %s: unimplemented\n", __FUNCTION__);
	    on_event(event_id::LOAD_PROGRESS);
	}

	/// Call a method with a list of arguments
	virtual const char* call_method_args(const char* method_name,
		const char* method_arg_fmt, va_list args)
	{
	    // Keep m_as_environment alive during any method calls!
	    smart_ptr<as_object>	this_ptr(this);

	    return call_method_parsed(&m_as_environment, this,
			method_name, method_arg_fmt, args);
	}

	virtual void	attach_display_callback(
		const char* path_to_object,
		void (*callback)(void*), void* user_ptr)
	{
//                  GNASH_REPORT_FUNCTION;
	  
		// should only be called on the root movie.
		assert(m_parent == NULL);

		std::vector<with_stack_entry>	dummy;
		as_value obj = m_as_environment.get_variable(tu_string(path_to_object), dummy);
		as_object*	as_obj = obj.to_object();
		if (as_obj)
		{
			movie*	m = as_obj->to_movie();
			if (m)
			{
			m->set_display_callback(callback, user_ptr);
			}
		}
	}

private:

	mouse_state m_mouse_state;

	smart_ptr<movie_definition>	m_def;
	movie_root*	m_root;

	DisplayList	m_display_list;

	//std::vector<action_buffer*>	m_action_list;
	std::vector<action_buffer*>	m_action_list;

	play_state	m_play_state;
	int		m_current_frame;
	float		m_time_remainder;
	bool		m_update_frame;
	bool		m_has_looped;

	// once we've been moved by ActionScript,
	// don't accept moves from anim tags.
	bool	        m_accept_anim_moves;

	// a bit-array class would be ideal for this
	std::vector<bool>	m_init_actions_executed;

	as_environment	m_as_environment;

	// For built-in sprite ActionScript methods.
	static as_object as_builtins;
	static void init_builtins();

	/// Increment m_current_frame, and take care of looping.
	void increment_frame_and_check_for_loop();

	bool m_on_event_load_called;
	float	m_frame_time;

};


} // end of namespace gnash

#endif // GNASH_SPRITE_INSTANCE_H
