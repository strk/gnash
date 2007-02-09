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

/* $Id: movie_root.h,v 1.35 2007/02/09 05:52:49 rsavoye Exp $ */

/// \page events_handling Handling of user events
///
/// There are two kinds of events:
/// - system generated
/// - user generated
///
/// System generated events are those like load, data recive, unload,
/// enter frame, etc.
/// User generated events are mouse movements and clicks, keyboard activity.
///
/// Events can trigger actions execution, if "handlers" are specified for
/// a specific event with ActionScript code.
/// The actions triggered by user events are executed *immediately*, not
/// at the next frame iteration. Nonetheless, since rendering of the stage
/// usually happens at fixed rate (frame rate) you won't see the effects
/// of actions execution until next iteration... unless...
///
/// Well, *some* events actions always trigger immediate redisplay, while
/// some others require a call to a special function to do so.
///
/// The events actions that trigger immediate redisplay are Button actions.
/// Colin Mook, in his "ActionScript - The Definitive Guide" sais:
/// << Buttons naturally update between frames >>
///
/// Other events, in particular MovieClip events such as mouseDown, mouseUp,
/// mouseMove, keyDown and keyUp don't by default trigger redisplay, unless
/// the attached action code makes a call to the special function named
/// 'updateAfterEvent()'.
///
/// For this purpose, user events notification functions in gnash core 
/// library return a boolean value, which tells wheter any action triggered
/// by the event requires immediate redisplay.
///
/// At the time of writing (2006-10-19) this is not implemented yet and
/// the return code is always TRUE. We shall work on it :)
///
/// The events notification functions that currently support this interface
/// are:
///
/// - bool movie_root::notify_mouse_moved(int x, int y);
/// - bool movie_root::notify_mouse_clicked(bool mouse_pressed, int mask);
/// 
/// Note that the notify_key_event() method is a global function, which should
/// likely be moved somewhere else, and that has not been fixed yet to support
/// the new interface.
/// 


#ifndef GNASH_MOVIE_ROOT_H
#define GNASH_MOVIE_ROOT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h"

#include "mouse_button_state.h" // for composition
#include "drag_state.h" // for composition
#include "sprite_instance.h" // for inlines

// Forward declarations
namespace gnash {
	class Timer;
}

namespace gnash
{

/// The absolute top level movie
//
/// This is a wrapper around the top-level movie_instance that is being played.
/// There is a *single* instance of this class for each run;
/// loading external movies will *not* create a new instance of it.
///
class movie_root 
{

public:

	/// Default constructor
	//
	/// Make sure to call setRootMovie() 
	/// before using any of this class methods !
	///
	movie_root();

	~movie_root();

	/// Set the root movie, replacing the current one if any.
	//
	/// This is needed for the cases in which the top-level movie
	/// is replaced by another movie by effect of a loadMovie call
	/// or similar.
	///
	/// TODO: inspect what happens about VM version
	///	  (should the *new* movie drive VM operations?
	///	   -- hope not ! )
	///
	/// Make sure to call this method before using the movie_root,
	/// as most operations are delegated to the associated/wrapped
	/// movie_instance.
	///
	/// Note that the display viewport will be updated to match
	/// the size of given movie.
	///
	/// @param movie
	///	The movie_instance to wrap.
	///	Will be stored in an intrusive_ptr.
	///
	void setRootMovie(movie_instance* movie);

	/// @@ should this delegate to _movie?  probably !
	void set_member(
		const std::string& /*name*/,
		const as_value& /*val*/)
	{
	}

	/// @@ should this delegate to _movie?  probably !
	bool get_member(const std::string& /*name*/,
			as_value* /*val*/)
	{
		return false;
	}

	void set_display_viewport(int x0, int y0, int w, int h);

	/// \brief
        /// The host app can use this to tell the movie when
        /// user's mouse pointer has moved.
	//
	/// Coordinates are in pixels.
	///
	/// This function should return TRUE iff any action triggered
	/// by the event requires redraw, see \ref events_handling for
	/// more info.
	///
        bool notify_mouse_moved(int x, int y);

	/// \brief
        /// The host app can use this to tell the movie when the
        /// user clicked or released the mouse button.
	//
	/// @param mouse_pressed
	///	true if the mouse has been pressed, false if released
	///
	/// @param mask
	///	???
	///
	/// This function should return TRUE iff any action triggered
	/// by the event requires redraw, see \ref events_handling for
	/// more info.
	///
        bool notify_mouse_clicked(bool mouse_pressed, int mask);

	/// The host app can use this to tell the movie where the
	/// user's mouse pointer is.
	void notify_mouse_state(int x, int y, int buttons);

	/// \brief
	/// Use this to retrieve the last state of the mouse, as set via
	/// notify_mouse_state(). 
	//
	/// Coordinates are in PIXELS, NOT TWIPS.
	///
	void	get_mouse_state(int& x, int& y, int& buttons);

	void get_drag_state(drag_state& st);

	void set_drag_state(const drag_state& st);

	/// @return current top-level root sprite
	sprite_instance* get_root_movie() { return _movie.get(); }

	void stop_drag()
	{
		log_msg("stop_drag called");
		m_drag_state.reset();
	}

	movie_definition* get_movie_definition() const {
		assert(_movie);
		return _movie->get_movie_definition();
	}

	/// Add an interval timer
	//
	/// @return an integer indentifying the timer
	///         for subsequent call to clear_interval_timer
	///
	unsigned int add_interval_timer(Timer& timer);

	/// Remove timer identified by given integer
	//
	/// @return true on success, false on error (no such timer)
	///
	bool clear_interval_timer(unsigned int x);

	/// 0-based!!
	size_t get_current_frame() const {
		return _movie->get_current_frame();
	}

	// @@ should this be in movie_instance ?
	float get_frame_rate() const {
		return get_movie_definition()->get_frame_rate();
	}

	/// \brief
	/// Return the size of a logical movie pixel as
	/// displayed on-screen, with the current device
	/// coordinates.
	float	get_pixel_scale() const
	{
	    return m_pixel_scale;
	}

	// @@ Is this one necessary?
	character* get_character(int character_id)
	{
	    return _movie->get_character(character_id);
	}

	void set_background_color(const rgba& color)
	{
	    m_background_color = color;
	}

	void	set_background_alpha(float alpha)
	{
	    m_background_color.m_a = iclamp(frnd(alpha * 255.0f), 0, 255);
	}

	float get_background_alpha() const
	{
	    return m_background_color.m_a / 255.0f;
	}

	float	get_timer() const { return m_timer; }

	/// Delegate to current top-level root sprite
	void	restart() { _movie->restart(); }

	void	advance(float delta_time);

	/// 0-based!!
	void goto_frame(size_t target_frame_number) {
		_movie->goto_frame(target_frame_number);
	}

	bool has_looped() const {
		return _movie->has_looped();
	}

	void display();

	/// Delegate to wrapped movie_instance
	bool goto_labeled_frame(const char* label) {
		return _movie->goto_labeled_frame(label);
	}

	/// Delegate to wrapped movie_instance
	void set_play_state(sprite_instance::play_state s) {
		_movie->set_play_state(s);
	}

	/// Delegate to wrapped movie_instance
	sprite_instance::play_state get_play_state() const {
		return _movie->get_play_state();
	}

	/// Delegate to wrapped movie_instance
	void set_variable(const char* path_to_var,
			const char* new_value)
	{
		_movie->set_variable(path_to_var, new_value);
	}

	/// Delegate to wrapped movie_instance
	void set_variable(const char* path_to_var,
			const wchar_t* new_value)
	{
		_movie->set_variable(path_to_var, new_value);
	}

	/// Delegate to wrapped movie_instance
	const char* get_variable(const char* path_to_var) const
	{
		return _movie->get_variable(path_to_var);
	}

	/// For ActionScript interfacing convenience.
	const char* call_method(const char* method_name,
			const char* method_arg_fmt, ...);
	const char* call_method_args(const char* method_name,
			const char* method_arg_fmt, va_list args);

	/// Delegate to wrapped movie_instance
	void set_visible(bool visible) {
		_movie->set_visible(visible);
	}
	/// Delegate to wrapped movie_instance
	bool get_visible() const {
		return _movie->get_visible();
	}

	void * get_userdata() { return m_userdata; }
	void set_userdata(void * ud ) { m_userdata = ud;  }

	/// Delegate to wrapped movie_instance
	void attach_display_callback(
			const char* path_to_object,
			void (*callback)(void* user_ptr),
			void* user_ptr)
	{
		_movie->attach_display_callback(path_to_object,
			callback, user_ptr);
	}

	DSOEXPORT void notify_keypress_listeners(key::code k);
	void add_keypress_listener(as_object* listener);
	void remove_keypress_listener(as_object* listener);

	/// Get the character having focus
	//
	/// @return the character having focus or NULL of none.
	///
	character* get_active_entity();

	/// Set the character having focus
	//
	/// @param ch
	///	The character having focus. NULL to kill focus.
	///
	void set_active_entity(character* ch);
	
	DSOEXPORT void get_invalidated_bounds(rect* bounds, bool force);

	/// Return true if the mouse pointer is over an active entity
	bool isMouseOverActiveEntity() const;

	bool testInvariant() const;

	void clear_invalidated() {
		_movie->clear_invalidated();
	}

private:

	// TODO: use Range2d<int> ?
	int			m_viewport_x0, m_viewport_y0;
	int			m_viewport_width, m_viewport_height;

	float			m_pixel_scale;

	rgba			m_background_color;
	float			m_timer;
	int			m_mouse_x, m_mouse_y, m_mouse_buttons;
	void *			m_userdata;

	mouse_button_state	m_mouse_button_state;

	// Flags for event handlers
	bool			m_on_event_xmlsocket_ondata_called;
	bool			m_on_event_xmlsocket_onxml_called;
	bool			m_on_event_load_progress_called;

	// TODO: should maintain refcount ?
	// FIXME: std::vector is not an appropriate container
	//        for timers, as we'll be removing them from the
	//        list but still want Timer "identifiers" to be
	//        valid.
	typedef std::vector<Timer *> TimerList;
	TimerList _intervalTimers;

	std::vector< as_object* >	m_keypress_listeners;
	character*              m_active_input_text;
	float                   m_time_remainder;

	/// @@ fold this into m_mouse_button_state?
	drag_state m_drag_state;

	/// The movie instance wrapped by this movie_root
	//
	/// We keep a pointer to the base sprite_instance class
	/// to avoid having to replicate all of the base class
	/// interface to the movie_instance class definition
	///
	boost::intrusive_ptr<sprite_instance> _movie;

	/// This function should return TRUE iff any action triggered
	/// by the event requires redraw, see \ref events_handling for
	/// more info.
	///
        bool fire_mouse_event();

};


} // namespace gnash

#endif // GNASH_MOVIE_ROOT_H

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
