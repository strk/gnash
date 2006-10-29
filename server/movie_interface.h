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

/* $Id: movie_interface.h,v 1.17 2006/10/29 18:34:11 rsavoye Exp $ */


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
/// - bool movie_interface::notify_mouse_moved(int x, int y);
/// - bool movie_interface::notify_mouse_clicked(bool mouse_pressed, int mask);
/// 
/// Note that the notify_key_event() method is a global function, which should
/// likely be moved somewhere else, and that has not been fixed yet to support
/// the new interface.
/// 

#ifndef GNASH_MOVIE_INTERFACE_H
#define GNASH_MOVIE_INTERFACE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include <cctype>	// for poxy wchar_t
#include <cstdarg>	// for va_list arg to movie_interface::call_method_args()

#include "as_object.h" // for inheritance
#include "rect.h"

// Forward declarations
namespace gnash {
	class sprite_instance;
}

namespace gnash {

/// An independent stateful live movie. (should be movie_instance?)
//
/// This is the client program's interface to an instance of a
/// movie. 
///
class movie_interface : public as_object
{
public:

	/// Set when the visual aspect this particular character or movie
	/// has been changed and redrawing is necessary.  
 	//
	/// TODO: make protected
	///
	bool m_invalidated;


	virtual movie_definition*	get_movie_definition() = 0;
	
	/// Frame counts in this API are 0-based (unlike ActionScript)
	virtual size_t	get_current_frame() const = 0;
	virtual bool	has_looped() const = 0;
	
	virtual void	restart() = 0;
	virtual void	advance(float delta_time) = 0;
	virtual void	goto_frame(size_t frame_number) = 0;
	/// Returns true if labeled frame is found.
	virtual bool	goto_labeled_frame(const char* label) = 0;
	virtual void	display() = 0;
	
	enum play_state
	{
		PLAY,
		STOP
	};
	
	virtual void	set_play_state(play_state s) = 0;
	virtual play_state	get_play_state() const = 0;
	
	virtual void	set_background_color(const rgba& bg_color) = 0;
	
	/// Set to 0 if you don't want the movie to render its
	/// background at all.  1 == full opacity.
	virtual void	set_background_alpha(float alpha) = 0;
	virtual float	get_background_alpha() const = 0;
	
	/// move/scale the movie...
	virtual void	set_display_viewport(int x0, int y0, int w, int h) = 0;
	
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
        virtual bool    notify_mouse_moved(int x, int y) = 0;

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
        virtual bool notify_mouse_clicked(bool mouse_pressed, int mask) = 0;

	virtual void	notify_mouse_state(int x, int y, int buttons) = 0;
	
	/// Set an ActionScript variable within this movie.
	/// You can use this to set the value of text fields,
	/// ordinary variables, or properties of characters
	/// within the script.
	///
	/// This version accepts UTF-8
	virtual void	set_variable(const char* path_to_var, const char* new_value) = 0;
	/// This version accepts UCS-2 or UCS-4, depending on sizeof(wchar_t)
	virtual void	set_variable(const char* path_to_var, const wchar_t* new_value) = 0;
	// @@ do we want versions that take a number?
	
	/// Get the value of an ActionScript variable.
	///
	/// Value is ephemeral & not thread safe!!!  Use it or
	/// copy it immediately.
	///
	/// Returns UTF-8
	virtual const char*	get_variable(const char* path_to_var) const = 0;
	// @@ do we want a version that returns a number?
	
	/// ActionScript method call.  Return value points to a
	/// static string buffer with the result; caller should
	/// use the value immediately before making more calls
	/// to gnash.  NOT THREAD SAFE!!!
	/// 
	/// method_name is the name of the method (possibly namespaced).
	///
	/// method_arg_fmt is a printf-style declaration of
	/// the method call, where the arguments are
	/// represented by { %d, %s, %f, %ls }, followed by the
	/// vararg list of actual arguments.
	/// 
	/// E.g.
	///
	/// m->call_method("path.to.method_name", "%d, %s, %f", i, "hello", 2.7f);
	///
	/// The format args are a small subset of printf, namely:
	///
	/// %d -- integer arg
	/// %s -- 0-terminated char* string arg
	/// %ls -- 0-terminated wchar_t* string arg
	/// %f -- float/double arg
	///
	/// Whitespace and commas in the format string are ignored.
	///
	/// This is not an ActionScript language parser, it
	/// doesn't recognize expressions or anything tricky.
#ifdef __GNUC__
	// use the following to catch errors: (only with gcc)
	virtual const char*	call_method(const char* method_name, const char* method_arg_fmt, ...)
		__attribute__((format (printf, 3, 4))) = 0;	// "this" is an implied param, so fmt is 3 and ... is 4!
#else	// not __GNUC__
	virtual const char*	call_method(const char* method_name, const char* method_arg_fmt, ...) = 0;
#endif	// not __GNUC__
	virtual const char*	call_method_args(const char* method_name, const char* method_arg_fmt, va_list args) = 0;
	
	
	/// Make the movie visible/invisible. 
	//
	/// An invisible
	/// movie does not advance and does not render.
	virtual void	set_visible(bool visible) = 0;
	
	/// Return visibility status.
	virtual bool	get_visible() const = 0;
	
	/// Get userdata, that's useful for the fs_command handler.
	virtual void   *get_userdata() = 0;
	
	/// Set userdata, that's useful for the fs_command handler.
	virtual void   set_userdata(void *) = 0;
	
	/// Display callbacks, for client rendering. 
	//
	/// Callback is called after rendering the object
	/// it's attached to.
	///
	/// Attach NULL to disable the callback.
	virtual void	attach_display_callback(const char* path_to_object, void (*callback)(void* user_ptr), void* user_ptr) = 0;

	virtual void get_url(const char *url) = 0;
	virtual int add_interval_timer(void *timer) = 0;
	virtual void clear_interval_timer(int x) = 0;

	/// for external movies
	virtual sprite_instance* get_root_movie() = 0;
	
  /// This method must be called whenever the visual aspect of this 
  /// character/movie did change. 
  /// VERY IMPORTANT!! This function *must* be called *before* the changes are
  /// applied!
	void set_invalidated() {
	
	  if (m_invalidated) return; // flag already set, don't do anything
	
	  m_invalidated = true;
	  
	  // Ok, at this point the instance will change it's visual aspect after the
	  // call to set_invalidated(). We save the *current* position of the 
    // instance because this region must be updated even (or first of all) if 
    // the character moves away from here.
    m_old_invalidated_bounds.set_null();
    get_invalidated_bounds(&m_old_invalidated_bounds, true);
     
  }
  
  // Should be called by display()
  void clear_invalidated() {
    m_invalidated = false;    
    m_old_invalidated_bounds.set_null();
  }
  
  
  /// Checks if the character instance is still enclosed in the given bounds.
  /// Otherwise it will expand them to surround the character. It is used to
  /// determine what area needs to be re-rendered. The coordinates are world
  /// coordinates. 
  /// Only instances with m_invalidated flag set are checked unless force
  /// is set.  
  virtual void get_invalidated_bounds(rect* bounds, bool force) = 0;

	/// Return true if the mouse pointer is over an active entity
	//
	/// @@ shouldn't these be globals instead ? (togheter with notify_mouse_moved and notify_mouse_clicked)
	virtual bool isMouseOverActiveEntity() const { return false; }
  
protected:

  /// Bounds of character instance before invalidating it
  rect m_old_invalidated_bounds;
  	
};

}	// namespace gnash

#endif // GNASH_MOVIE_INTERFACE_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
