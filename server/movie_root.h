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

/* $Id: movie_root.h,v 1.24 2006/11/11 22:44:54 strk Exp $ */

#ifndef GNASH_MOVIE_ROOT_H
#define GNASH_MOVIE_ROOT_H

#include "container.h"
#include "mouse_button_state.h" // for mouse_button_state
#include "timers.h" // for Timer
#include "fontlib.h"
#include "font.h"
#include "jpeg.h"
#include "tu_file.h"
#include "movie_def_impl.h"
#include "tu_config.h"
#include "sprite_instance.h" // for inlines

namespace gnash
{

// Forward declarations
class import_info;
class movie_def_impl;
class movie_root;
class import_visitor; // in gnash.h


/// Global, shared root state for a movie and all its characters.
class movie_root : public movie_interface // inheritance should be dropped
{
	boost::intrusive_ptr<movie_def_impl>	m_def;
	int			m_viewport_x0, m_viewport_y0;
	int			m_viewport_width, m_viewport_height;
	float			m_pixel_scale;

	rgba			m_background_color;
	float			m_timer;
	int			m_mouse_x, m_mouse_y, m_mouse_buttons;
	void *			m_userdata;

	mouse_button_state	m_mouse_button_state;
//	bool			m_on_event_load_called;

	// Flags for event handlers
	bool			m_on_event_xmlsocket_ondata_called;
	bool			m_on_event_xmlsocket_onxml_called;
	bool			m_on_event_load_progress_called;
	std::vector<Timer *>	m_interval_timers;
	std::vector< as_object* >	m_keypress_listeners;
	movie*	m_active_input_text;
	float m_time_remainder;

public:
	// XXXbastiaan: make these two variables private
	boost::intrusive_ptr<sprite_instance>	m_movie;
	/// @@ fold this into m_mouse_button_state?
	movie::drag_state	m_drag_state;

	movie_root(movie_def_impl* def);

	~movie_root();

	/// @@ should these delegate to m_movie? 
	virtual void set_member(
		const tu_stringi& /*name*/,
		const as_value& /*val*/)
	{
	}

	virtual bool get_member(const tu_stringi& /*name*/,
			as_value* /*val*/)
	{
		return false;
	}

	/// @@ should this return m_movie.get()?
	virtual movie* to_movie() { assert(0); return 0; }

	void set_root_movie(sprite_instance* root_movie);

	void set_display_viewport(int x0, int y0, int w, int h);

	// derived from movie_interface, see dox in movie_interface.h
        bool notify_mouse_moved(int x, int y);

	// derived from movie_interface, see dox in movie_interface.h
        bool notify_mouse_clicked(bool mouse_pressed, int mask);

	/// The host app can use this to tell the movie where the
	/// user's mouse pointer is.
	void notify_mouse_state(int x, int y, int buttons);

	/// Use this to retrieve the last state of the mouse, as set via
	/// notify_mouse_state().  Coordinates are in PIXELS, NOT TWIPS.
	virtual void	get_mouse_state(int* x, int* y, int* buttons);

	sprite_instance* get_root_movie() { return m_movie.get(); }

	void stop_drag() { m_drag_state.m_character = NULL; }

	movie_definition* get_movie_definition() {
		return m_movie->get_movie_definition();
	}

#if 0 // renamed to get_bytes_total
	uint32 get_file_bytes() const {
	    return m_def->get_file_bytes();
	}
#endif

	/// Get number of bytes loaded from input stream
	uint32 get_bytes_loaded() const {
	    return m_def->get_bytes_loaded();
	}

	/// Get total number of bytes in input stream
	uint32 get_bytes_total() const {
	    return m_def->get_bytes_total();
	}

	virtual void get_url(const char *url) {
		// SWFHandlers::ActionGetUrl calls get_url
		// on target, which is always a sprite_instance
		// (well, a character, at least)
		assert(0);
		// @@ delegate to actual sprite instance
		m_movie->get_url(url);
	}

	virtual int add_interval_timer(void *timer);
	virtual void clear_interval_timer(int x);
	virtual void do_something(void *timer);

	/// 0-based!!
	size_t get_current_frame() const {
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
	void goto_frame(size_t target_frame_number) {
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

	DSOEXPORT void notify_keypress_listeners(key::code k);
	void add_keypress_listener(as_object* listener);
	void remove_keypress_listener(as_object* listener);

	movie* get_active_entity();
	void set_active_entity(movie* ch);
	
	void get_invalidated_bounds(rect* bounds, bool force)
	{
	 
		if (m_invalidated)
		{
			// complete redraw (usually first frame)
			bounds->expand_to_point(-1e10f, -1e10f);
			bounds->expand_to_point(1e10f, 1e10f);
		}
		else
		{
			// browse characters to compute bounds
			// TODO: Use better start-values
			bounds->set_null();
			m_movie->get_invalidated_bounds(bounds,
				force||m_invalidated);
		}

          
	}

	// reimplemented from movie_interface, see dox there
	bool isMouseOverActiveEntity() const;

private:

	/// This function should return TRUE iff any action triggered
	/// by the event requires redraw, see \ref events_handling for
	/// more info.
	///
        bool fire_mouse_event();

};


} // namespace gnash

#endif // GNASH_MOVIE_ROOT_H
