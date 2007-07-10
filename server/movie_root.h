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

/* $Id: movie_root.h,v 1.65 2007/07/10 12:22:34 zoulunkai Exp $ */

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
/// - bool movie_root::notify_key_event(key::code k, bool down);
/// 
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
#include "timers.h" // for composition
#include "asobj/Key.h"

#include <vector>
#include <list>
#include <set>

// Forward declarations
namespace gnash {
	class ExecutableCode; // for ActionQueue
	class Stage;
}

namespace gnash
{

#ifdef NEW_KEY_LISTENER_LIST_DESIGN
class KeyListener{
	public:
		/// all constructed key listeners are not registered to the global Key object by default.
		KeyListener(boost::intrusive_ptr<as_object> obj, bool flag=false)
		: _listener(obj), _user_defined_handler_added(flag)
		{}

		as_object * get() const { return _listener.get(); }

		bool operator == (const KeyListener & rhs ) const { return _listener.get() == rhs.get(); }
		bool operator != (const KeyListener & rhs ) const { return _listener.get() != rhs.get(); }
	
		/// \brief
		/// Return true if the _listener has been registered by Key.addListener(),
		/// false if the _listener has not been registered or unregistered by Key.removeListener().
		/// The return value is used to decide if we should invoke the user defined handler.
		bool isRegistered() const { return _user_defined_handler_added; }
		
		/// unregister the key listener
		void unregisterUserHandler() { _user_defined_handler_added = false; }
		
		/// register the key listener
		void registerUserHandler() { _user_defined_handler_added = true; }

#ifdef GNASH_USE_GC
		/// Mark the wrapped object as reachable
		void setReachable() const
		{
			if ( _listener ) _listener->setReachable();
		}
#endif

	private:

		/// the listener object, could be a character or a general as_object
		boost::intrusive_ptr<as_object> _listener;

		/// mark if the object has been registered by Key.addListener()
		bool _user_defined_handler_added;
	};
#endif 

/// The movie stage (absolute top level node in the characters hierarchy)
//
/// This is a wrapper around the top-level movie_instance that is being played.
/// There is a *single* instance of this class for each run;
/// loading external movies will *not* create a new instance of it.
///
class DSOEXPORT movie_root 
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

	/// Change display viewport coordinates
	//
	/// This currently also change the display scale
	/// but we should instead only do it if rescaling
	/// is allowed.
	///
	void set_display_viewport(int x0, int y0, int w, int h);

	/// Get current viewport width, in pixels
	unsigned getWidth() const
	{
		return m_viewport_width;
	}

	/// Get current viewport height, in pixels
	unsigned getHeight() const
	{
		return m_viewport_height;
	}

	/// Set whether rescaling is allowed or not.
	//
	/// When rescaling is not allowed the Stage listeners
	/// will get notified on any resize attempt.
	///
	void allowRescaling(bool v)
	{
		_allowRescale=v;
	}

	bool isRescalingAllowed()
	{
		return _allowRescale;
	}

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
        /// The host app can use this to tell the movie when
        /// user pressed or released a key.
	//
	/// This function should return TRUE iff any action triggered
	/// by the event requires redraw, see \ref events_handling for
	/// more info.
	///
	bool notify_key_event(key::code k, bool down);

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
	/// @param timer
	///	A Timer, ownership will be transferred. Must not be NULL.
	///
	/// @param internal
	///	If true, this is an internal timer, so will get a negative id.
	///
	/// @return An integer indentifying the timer
	///         for subsequent call to clear_interval_timer.
	///         It will NEVER be zero.
	///
	unsigned int add_interval_timer(std::auto_ptr<Timer> timer, bool internal=false);

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

	DSOEXPORT void notify_key_listeners(key::code k, bool down);
#ifdef NEW_KEY_LISTENER_LIST_DESIGN
	// Push a new key listener to the container if it is not there,
	// otherwise, just register it.
	void add_key_listener(const KeyListener& listener);
	
	// remove the specified listener from the container if found
	void remove_key_listener(const KeyListener& listener);
	
	std::vector<KeyListener> & getKeyListeners() { return _keyListeners; }
#else
	void add_key_listener(as_object* listener);
	void remove_key_listener(as_object* listener);
#endif

	DSOEXPORT void notify_mouse_listeners(const event_id& event);
	void add_mouse_listener(as_object* listener);
	void remove_mouse_listener(as_object* listener);

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
	
	DSOEXPORT void add_invalidated_bounds(InvalidatedRanges& ranges, bool force);

	/// Return true if the mouse pointer is over an active entity
	bool isMouseOverActiveEntity() const;

	bool testInvariant() const;

	void clear_invalidated() {
		_movie->clear_invalidated();
	}

	/// Push an executable code to the ActionQueue
	void pushAction(std::auto_ptr<ExecutableCode> code);

	/// Push an executable code to the ActionQueue
	void pushAction(const action_buffer& buf, boost::intrusive_ptr<character> target);

	/// Push a function code to the ActionQueue
	void pushAction(boost::intrusive_ptr<as_function> func, boost::intrusive_ptr<character> target);

#ifdef GNASH_USE_GC
	/// Mark all reachable resources (for GC)
	//
	/// Resources reachable from movie_root are:
	///
	///	- All _level# movies (_movie)
	///	- Mouse entities (m_mouse_button_state)
	///	- Timer targets (_intervalTimers)
	///	- Resources reachable by ActionQueue code (_actionQueue)
	///	- Key listeners (_keyListeners || m_key_listeners)
	///
	void markReachableResources() const;
#endif // GNASH_USE_GC

private:

	/// Forbid copy 
	movie_root(const movie_root& ) { assert(0); }

	/// Forbid assignment
	movie_root& operator=(const movie_root& ) { assert(0); return *this; }

	/// Execute expired timers
	void executeTimers();

	/// Notify the global Key ActionScript object about a key status change
	key_as_object * notify_global_key(key::code k, bool down);
	
	/// Remove all listeners with a ref-count of 1
	/// (only referenced as key listeners)
	// in new design:
	// remove unloaded characters and unregistered as_objects 
	// from the key listeners container.
	void cleanup_key_listeners();

	/// Return the current Stage object
	//
	/// Can return NULL if it's been deleted or not
	/// yet initialized.
	///
	boost::intrusive_ptr<Stage> getStageObject();

	typedef std::list<ExecutableCode*> ActionQueue;

	ActionQueue _actionQueue;

	/// Process all actions in the queue
	void processActionQueue();

	// TODO: use Range2d<int> ?
	int			m_viewport_x0, m_viewport_y0;

	/// Width and height of viewport, in pixels
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

	typedef std::map<int, Timer*> TimerMap;

	TimerMap _intervalTimers;
	unsigned int _lastTimerId;

	/// A set of as_objects kept by intrusive_ptr
	typedef std::set< boost::intrusive_ptr<as_object> > ListenerSet;

	/// Objects listening for key events
#ifdef NEW_KEY_LISTENER_LIST_DESIGN
	typedef std::vector<KeyListener> KeyListeners;
	KeyListeners _keyListeners;
#else
	ListenerSet m_key_listeners;
#endif
	/// Objects listening for mouse events (down,up,move)
	ListenerSet m_mouse_listeners;

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

	/// If set to false, no rescale should be performed
	/// when changing viewport size
	bool _allowRescale;
};


} // namespace gnash

#endif // GNASH_MOVIE_ROOT_H

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
