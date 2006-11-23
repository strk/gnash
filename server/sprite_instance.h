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

/* $Id: sprite_instance.h,v 1.41 2006/11/23 20:14:13 strk Exp $ */

// Stateful live Sprite instance

#ifndef GNASH_SPRITE_INSTANCE_H
#define GNASH_SPRITE_INSTANCE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "action.h" // for call_method_parsed (call_method_args)
#include "edit_text_character.h" // temp hack
#include "movie_definition.h" // for inlines
#include "dlist.h" // DisplayList 
#include "log.h"
#include "as_environment.h" // for composition

#include <vector>
#include <list>
#include <map>

namespace gnash
{

// Forward declarations
class movie_root; 
class swf_event;

/// Stateful Sprite object. Also known as a MovieClip.
//
/// Instance of this class are also known as "timelines".
/// This means that they define a variable scope (see
/// the as_environment member) and are divided into "frames"
///
class sprite_instance : public character
{

public:

	typedef std::list<action_buffer*> ActionList;
	// definition must match movie_definition::PlayList
	typedef std::vector<execute_tag*> PlayList;

	sprite_instance(movie_definition* def,
		movie_root* r, character* parent, int id);

	virtual ~sprite_instance();


	enum mouse_state
	{
		UP = 0,
		DOWN,
		OVER
	};

	enum play_state
	{
		PLAY,
		STOP
	};

	virtual void has_keypress_event();

	// sprite instance of add_interval_handler()
	// delegates to m_root
	virtual int    add_interval_timer(void *timer);

	// delegates to m_root
	virtual void  clear_interval_timer(int x);
	

	/// Interval timer timeout executor
	virtual void    do_something(void *timer);

	movie_root* get_root() {
		return m_root;
	}

	/// Get a pointer to the root sprite
	sprite_instance* get_root_movie();

	/// \brief
	/// Return the sprite_definition (or movie_definition)
	/// from which this sprite_instance has been created
        movie_definition* get_movie_definition() {
                return m_def.get();
        }

	float get_width() const;

	float get_height() const;

	size_t get_current_frame() const
	{
		return m_current_frame;
	}

	size_t get_frame_count() const
	{
		return m_def->get_frame_count();
	}

	/// Return number of completely loaded frames of this sprite/movie
	//
	/// Note: the number is also the last frame accessible (frames
	/// numberes are 1-based)
	///
	size_t get_loaded_frames() const
	{
		return m_def->get_loading_frame();
	}

	/// Return total number of bytes in the movie
	/// (not sprite!)
	size_t get_bytes_total() const
	{
		return m_def->get_bytes_total();
	}

	/// Return number of loaded bytes in the movie
	/// (not sprite!)
	size_t get_bytes_loaded() const
	{
		return m_def->get_bytes_loaded();
	}

	const rect& get_frame_size() const
	{
		return m_def->get_frame_size();
	}

	/// Stop or play the sprite.
	void set_play_state(play_state s)
	{
	    if (m_play_state != s) m_time_remainder = 0;
	    m_play_state = s;
	}

	play_state get_play_state() const { return m_play_state; }

	character* get_character(int character_id);

	float get_background_alpha() const;

	// delegates to m_root
	float	get_pixel_scale() const;

	// delegates to m_root
	virtual void get_mouse_state(int* x, int* y, int* buttons);

	// delegatest to m_root
	void	set_background_color(const rgba& color);

	float	get_timer() const;

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
	virtual bool can_handle_mouse_event();

	/// \brief
	/// Return the topmost entity that the given point
	/// covers that can receive mouse events.  NULL if
	/// none.  Coords are in parent's frame.
	virtual character* get_topmost_mouse_entity(float x, float y);

	virtual void	advance(float delta_time);
	//virtual void	advance_root(float delta_time);
	virtual void	advance_sprite(float delta_time);

	/// Execute the tags associated with the specified frame.
	/// frame is 0-based
	void execute_frame_tags(size_t frame, bool state_only = false);


	/// Execute the tags associated with the specified frame,
	/// IN REVERSE.
	/// I.e. if it's an "add" tag, then we do a "remove" instead.
	/// Only relevant to the display-list manipulation tags:
	/// add, move, remove, replace.
	///
	/// frame is 0-based
	void execute_frame_tags_reverse(size_t frame);

		
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
	void	goto_frame(size_t target_frame_number);

	/// \brief
	/// Parse frame spec and return frame number.
	/// Frame spec can either be a number of a string (label)
	///
	size_t get_frame_number(const as_value& frame_spec) const;


	/// Look up the labeled frame, and jump to it.
	bool goto_labeled_frame(const char* label);

		
	/// Display (render?) this Sprite/MovieClip, unless invisible
	void	display();

	void swap_characters(character* ch1, character* ch2);
	character* get_character_at_depth(int depth);
	character* add_empty_movieclip(const char* name, int depth);

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
	void	remove_display_object(uint16_t depth, int /* id */)
	{
	    set_invalidated();
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

	sprite_instance* to_movie () { return this; }


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

	/// Overridden to look in DisplayList for a match
	virtual character* get_relative_target(const std::string& name);

	/// Execute the actions for the specified frame. 
	//
	/// The frame_spec could be an integer or a string.
	///
	virtual void call_frame_actions(const as_value& frame_spec);

	// delegatest to m_root
	virtual void set_drag_state(const drag_state& st);

	virtual void stop_drag();

	// delegates to m_root
	virtual void get_drag_state(drag_state* st);


	/// Duplicate the object with the specified name
	/// and add it with a new name  at a new depth.
	void clone_display_object(const std::string& name,
		const std::string& newname, uint16_t depth);

	/// Remove the object with the specified name.
	//
	/// @@ what happens if the we have multiple objects
	///    with the same name ?
	void remove_display_object(const tu_string& name);

	/// Dispatch event handler(s), if any.
	virtual bool	on_event(const event_id& id);


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
	    boost::intrusive_ptr<as_object>	this_ptr(this);

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

		as_value obj = m_as_environment.get_variable(std::string(path_to_object));
		as_object* as_obj = obj.to_object();
		character* ch = dynamic_cast<character*>(as_obj);
		if (ch)
		{
			ch->set_display_callback(callback, user_ptr);
		}
	}

	// inherited from character class, see dox in character.h
	as_environment& get_environment() {
		return m_as_environment;
	}

	/// \brief
	/// Set a TextField variable to this timeline
	//
	/// A TextField variable is a variable that acts
	/// as a setter/getter for a TextField 'text' member.
	///
	void set_textfield_variable(const std::string& name,
			edit_text_character* ch);

	void get_invalidated_bounds(rect* bounds, bool force);
			

	const DisplayList& getDisplayList() const {
		return m_display_list;
	}

	/// Return the next highest available depth
	//
	/// Placing an object at the depth returned by
	/// this function should result in a character
	/// that is displayd above all others
	///
	int getNextHighestDepth() const {
		return m_display_list.getNextHighestDepth();
	}

	void testInvariant() const {
		assert(m_play_state == PLAY || m_play_state == STOP);
		assert(m_current_frame < m_def->get_frame_count());
		assert(get_ref_count() > 0); // or we're constructed but
		                             // not stored in a boost::intrusive_ptr
	}

	/// Set the current m_sound_stream_id
	virtual void set_sound_stream_id(int id){ m_sound_stream_id = id; }

	/// Get the current m_sound_stream_id
	virtual int get_sound_stream_id() { return m_sound_stream_id;}

private:


	mouse_state m_mouse_state;

	movie_root*	m_root;

	DisplayList	m_display_list;

	ActionList	m_action_list;
	ActionList	m_goto_frame_action_list;

	play_state	m_play_state;
	size_t		m_current_frame;
	float		m_time_remainder;
	bool		m_update_frame;
	bool		m_has_looped;

	// once we've been moved by ActionScript,
	// don't accept moves from anim tags.
	bool	        m_accept_anim_moves;

	// a bit-array class would be ideal for this
	std::vector<bool>	m_init_actions_executed;

	/// This timeline's variable scope
	as_environment	m_as_environment;

	// For built-in sprite ActionScript methods.
	static as_object as_builtins;

	/// Initialize built-ins for target SWF version
	//
	/// Some interfaces might be unavailable in certaing
	/// versions.
	///
	/// NOTE: if you call this multiple times with different
	///       target versions only the first invocation will
	///	  have an effect.
	///
	/// TODO: move to implementation file...
	///
	static void init_builtins(int target_version);

	/// Increment m_current_frame, and take care of looping.
	void increment_frame_and_check_for_loop();

	float	m_frame_time;
	bool m_has_keypress_event;

	/// A container for textfields, indexed by their variable name
	typedef std::map< std::string, boost::intrusive_ptr<edit_text_character> > TextfieldMap;

	/// We'll only allocate Textfield variables map if
	/// we need them (ie: anyone calls set_textfield_variable)
	///
	std::auto_ptr<TextfieldMap> _text_variables;

	/// \brief
	/// Returns a TextField given it's variable name,
	/// or NULL if no such variable name is known.
	//
	/// A TextField variable is a variable that acts
	/// as a setter/getter for a TextField 'text' member.
	///
	/// Search is case-sensitive.
	///
	/// @todo find out wheter we should be case sensitive or not
	///
	edit_text_character* get_textfield_variable(const std::string& name);

	/// soundid for current playing stream. If no stream set to -1
	int m_sound_stream_id;

protected:

	/// \brief
	/// This is either sprite_definition (for sprites defined by
	/// DefineSprite tag) or movie_def_impl (for the top-level movie).
	boost::intrusive_ptr<movie_definition>	m_def;

	bool m_on_event_load_called;

};


} // end of namespace gnash

#endif // GNASH_SPRITE_INSTANCE_H
