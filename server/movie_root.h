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

#ifndef GNASH_MOVIE_ROOT_H
#define GNASH_MOVIE_ROOT_H

#include "container.h"
#include "button.h" // for mouse_button_state
#include "timers.h" // for Timer
#include "fontlib.h"
#include "font.h"
#include "jpeg.h"
#include "tu_file.h"
#include "movie_def_impl.h"

namespace gnash
{

// Forward declarations
class import_info;
struct movie_def_impl;
struct movie_root;
struct import_visitor; // in gnash.h


/// Global, shared root state for a movie and all its characters.
class movie_root : public movie_interface
{
	smart_ptr<movie_def_impl>	m_def;
	int			m_viewport_x0, m_viewport_y0;
	int			m_viewport_width, m_viewport_height;
	float			m_pixel_scale;

	rgba			m_background_color;
	float			m_timer;
	int			m_mouse_x, m_mouse_y, m_mouse_buttons;
	void *			m_userdata;

	mouse_button_state	m_mouse_button_state;
	bool			m_on_event_load_called;

	// Flags for event handlers
	bool			m_on_event_xmlsocket_ondata_called;
	bool			m_on_event_xmlsocket_onxml_called;
	bool			m_on_event_load_progress_called;
	std::vector<Timer *>	m_interval_timers;

public:
	// XXXbastiaan: make these two variables private
	smart_ptr<movie>	m_movie;
	/// @@ fold this into m_mouse_button_state?
	movie::drag_state	m_drag_state;

	movie_root(movie_def_impl* def);

	~movie_root();

	/// @@ should these delegate to m_movie?  Probably...
	virtual void	set_member(const tu_stringi& name,
		const as_value& val) {}
	virtual bool	get_member(const tu_stringi& name,
		as_value* val) { return false; }

	/// @@ should this return m_movie.get_ptr()?
	virtual movie*	to_movie() { assert(0); return 0; }

	void set_root_movie(movie* root_movie);

	void set_display_viewport(int x0, int y0, int w, int h);

	/// The host app uses this to tell the movie where the
	/// user's mouse pointer is.
	void notify_mouse_state(int x, int y, int buttons);

	/// Use this to retrieve the last state of the mouse, as set via
	/// notify_mouse_state().  Coordinates are in PIXELS, NOT TWIPS.
	virtual void	get_mouse_state(int* x, int* y, int* buttons);

	movie*	get_root_movie() { return m_movie.get_ptr(); }

	void stop_drag() { m_drag_state.m_character = NULL; }

	movie_definition* get_movie_definition() {
		return m_movie->get_movie_definition();
	}

	uint32 get_file_bytes() const {
	    return m_def->get_file_bytes();
	}

	virtual void get_url(const char *url);
	virtual int add_interval_timer(void *timer);
	virtual void clear_interval_timer(int x);
	virtual void do_something(void *timer);

	/// 0-based!!
	int get_current_frame() const {
		return m_movie->get_current_frame();
	}

	float get_frame_rate() const {
		return m_def->get_frame_rate();
	}

	/// Return the size of a logical movie pixel as
	/// displayed on-screen, with the current device
	/// coordinates.
	virtual float	get_pixel_scale() const
	{
	    return m_pixel_scale;
	}

	// @@ Is this one necessary?
	character* get_character(int character_id)
	{
	    return m_movie->get_character(character_id);
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

	void	restart() { m_movie->restart(); }

	void	advance(float delta_time);

	/// 0-based!!
	void goto_frame(int target_frame_number) {
		m_movie->goto_frame(target_frame_number);
	}

	virtual bool has_looped() const {
		return m_movie->has_looped();
	}

	void display();

	virtual bool goto_labeled_frame(const char* label);

	virtual void set_play_state(play_state s) {
		m_movie->set_play_state(s);
	}

	virtual play_state get_play_state() const {
		return m_movie->get_play_state();
	}

	virtual void set_variable(const char* path_to_var,
			const char* new_value)
	{
		m_movie->set_variable(path_to_var, new_value);
	}

	virtual void set_variable(const char* path_to_var,
			const wchar_t* new_value)
	{
		m_movie->set_variable(path_to_var, new_value);
	}

	virtual const char* get_variable(const char* path_to_var) const
	{
		return m_movie->get_variable(path_to_var);
	}

	/// For ActionScript interfacing convenience.
	virtual const char* call_method(const char* method_name,
			const char* method_arg_fmt, ...);
	virtual const char* call_method_args(const char* method_name,
			const char* method_arg_fmt, va_list args);

	virtual void set_visible(bool visible) {
		m_movie->set_visible(visible);
	}
	virtual bool get_visible() const {
		return m_movie->get_visible();
	}

	virtual void * get_userdata() { return m_userdata; }
	virtual void set_userdata(void * ud ) { m_userdata = ud;  }

	virtual void attach_display_callback(
			const char* path_to_object,
			void (*callback)(void* user_ptr),
			void* user_ptr)
	{
		m_movie->attach_display_callback(path_to_object,
			callback, user_ptr);
	}
};


} // namespace gnash

#endif // GNASH_MOVIE_ROOT_H
