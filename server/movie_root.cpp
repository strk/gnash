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
#ifdef HAVE_PTHREADS
#include <pthread.h>
#endif

#include <iostream>
#include <string>

#include "movie_root.h"
//#include "tu_file.h"
//#include "zlib_adapter.h"
//#include "stream.h"
//#include "jpeg.h"
//#include "fontlib.h"
//#include "font.h"
#include "log.h"
#include "sprite_instance.h"
#include "render.h"

using namespace std;

namespace gnash
{

movie_root::movie_root(movie_def_impl* def)
    :
    m_def(def),
    m_viewport_x0(0),
    m_viewport_y0(0),
    m_viewport_width(1),
    m_viewport_height(1),
    m_pixel_scale(1.0f),
    m_background_color(0, 0, 0, 255),
    m_timer(0.0f),
    m_mouse_x(0),
    m_mouse_y(0),
    m_mouse_buttons(0),
    m_userdata(NULL),
//    m_on_event_load_called(false),
    m_on_event_xmlsocket_ondata_called(false),
    m_on_event_xmlsocket_onxml_called(false),
    m_on_event_load_progress_called(false),
		m_active_input_text(NULL)
	{
	assert(m_def != NULL);

	set_display_viewport(0, 0,
		(int) m_def->get_width_pixels(),
		(int) m_def->get_height_pixels());
}

movie_root::~movie_root()
{
    assert(m_def != NULL);
    m_movie = NULL;
    m_def = NULL;
}

		
void
movie_root::set_root_movie(sprite_instance* root_movie)
{
    m_movie = root_movie;
    assert(m_movie != NULL);
}

void
movie_root::set_display_viewport(int x0, int y0, int w, int h)
{
    m_viewport_x0 = x0;
    m_viewport_y0 = y0;
    m_viewport_width = w;
    m_viewport_height = h;

    // Recompute pixel scale.
    const rect& frame_size = m_def->get_frame_size();

    float	scale_x = m_viewport_width / TWIPS_TO_PIXELS(frame_size.width());
    float	scale_y = m_viewport_height / TWIPS_TO_PIXELS(frame_size.height());
    m_pixel_scale = fmax(scale_x, scale_y);
}

void
movie_root::notify_mouse_moved(int x, int y)
{
    m_mouse_x = x;
    m_mouse_y = y;
    fire_mouse_event();
}

void
movie_root::notify_mouse_clicked(bool mouse_pressed, int button_mask)
{
    if (mouse_pressed) {
        m_mouse_buttons |= button_mask;
    } else {
        m_mouse_buttons &= ~button_mask;
    }
    fire_mouse_event();
}

void
movie_root::notify_mouse_state(int x, int y, int buttons)
{
    m_mouse_x = x;
    m_mouse_y = y;
    m_mouse_buttons = buttons;
    fire_mouse_event();
}

void
movie_root::fire_mouse_event()
{
//	    GNASH_REPORT_FUNCTION;
//             dbglogfile << "X is: " << x << " Y is: " << y << " Button is: "
//                        << buttons << endl;

    // Generate a mouse event
    m_mouse_button_state.m_topmost_entity =
        m_movie->get_topmost_mouse_entity(PIXELS_TO_TWIPS(m_mouse_x), PIXELS_TO_TWIPS(m_mouse_y));
    m_mouse_button_state.m_mouse_button_state_current = (m_mouse_buttons & 1);
    generate_mouse_button_events(&m_mouse_button_state);
}

void
movie_root::get_mouse_state(int* x, int* y, int* buttons)
{
//	    GNASH_REPORT_FUNCTION;

    assert(x);
    assert(y);
    assert(buttons);

//             dbglogfile << "X is: " << m_mouse_x << " Y is: " << m_mouse_y
//                        << " Button is: "
//                        << m_mouse_buttons << endl;
    *x = m_mouse_x;
    *y = m_mouse_y;
    *buttons = m_mouse_buttons;
}

void
movie_root::get_url(const char *url)
{
    GNASH_REPORT_FUNCTION;
    
    string command = "mozilla -remote \"openurl";
    command += url;
    command += ")\"";
    dbglogfile << "Launching URL... " << command << endl;
    system(command.c_str());
}	

int
movie_root::add_interval_timer(void *timer)
{
    Timer *ptr = static_cast<Timer *>(timer);
			
    m_interval_timers.push_back(ptr);
    return m_interval_timers.size();
}
	
void
movie_root::clear_interval_timer(int x)
{
    m_interval_timers.erase(m_interval_timers.begin() + x-1);
    //m_interval_timers[x]->clearInterval();
}
	
void
movie_root::do_something(void *timer)
{
    log_msg("FIXME: %s: unimplemented\n", __FUNCTION__);
}
		
void
movie_root::advance(float delta_time)
{
//            GNASH_REPORT_FUNCTION;

//	if (m_on_event_load_called == false)
//        {
            // Must do loading events.  For child sprites this is
            // done by the dlist, but root movies don't get added
            // to a dlist, so we do it here.
//            m_on_event_load_called = true;
//            m_movie->on_event_load();
//        }

#if 0
    // Must check the socket connection for data
    if (m_on_event_xmlsocket_ondata_called == true) {
        m_movie->on_event_xmlsocket_ondata();
    }
			
    if (m_on_event_xmlsocket_onxml_called == true) {
        m_movie->on_event_xmlsocket_onxml();
    }


    // Must check the progress of the MovieClip being loaded
    if (m_on_event_load_progress_called == true) {
        m_movie->on_event_load_progress();				
    }
#endif
    if (m_interval_timers.size() > 0) {
        for (unsigned int i=0; i<m_interval_timers.size(); i++) {
            if (m_interval_timers[i]->expired()) {
                // printf("FIXME: Interval Timer Expired!\n");
                //m_movie->on_event_interval_timer();
                m_movie->do_something(m_interval_timers[i]);
                // clear_interval_timer(m_interval_timers[i]->getIntervalID()); // FIXME: we shouldn't really disable the timer here
            }
        }
    }
			
// m_movie->advance(delta_time);

		// Vitaly:
		// onload event for root movieclip is executed after frame 1 actions.
		// onload event for child movieclip is executed before frame 1 actions.
		// that's why advance for root movieclip and child movieclip are different.
    m_timer += delta_time;
		sprite_instance* current_root = (sprite_instance*) m_movie.get_ptr();
		assert(current_root);
		current_root->advance_root(delta_time);
}


void
movie_root::display()
{
//  	    GNASH_REPORT_FUNCTION;
    if (m_movie->get_visible() == false)
        {
            // Don't display.
            return;
        }

    const rect& frame_size = m_def->get_frame_size();

    gnash::render::begin_display(
        m_background_color,
        m_viewport_x0, m_viewport_y0,
        m_viewport_width, m_viewport_height,
        frame_size.m_x_min, frame_size.m_x_max,
        frame_size.m_y_min, frame_size.m_y_max);

    m_movie->display();

    gnash::render::end_display();
}

bool
movie_root::goto_labeled_frame(const char* label)
{
    int	target_frame = -1;
    if (m_def->get_labeled_frame(label, &target_frame))
        {
            goto_frame(target_frame);
            return true;
        }
    else
        {
            log_action("ERROR: movie_impl::goto_labeled_frame('%s') unknown label\n", label);
            return false;
        }
}


const char*
movie_root::call_method(const char* method_name,
		const char* method_arg_fmt, ...)
{
	assert(m_movie != NULL);

	va_list	args;
	va_start(args, method_arg_fmt);
	const char* result = m_movie->call_method_args(method_name,
		method_arg_fmt, args);
	va_end(args);

	return result;
}

const
char* movie_root::call_method_args(const char* method_name,
		const char* method_arg_fmt, va_list args)
{
	assert(m_movie != NULL);
	return m_movie->call_method_args(method_name, method_arg_fmt, args);
}

void movie_root::notify_keypress_listeners(key::code k)
{
	for (std::vector< as_object* >::iterator iter = m_keypress_listeners.begin();
			 iter != m_keypress_listeners.end(); ++iter)
	{
		if (*iter == NULL)
		{
				continue;
		}

		smart_ptr<as_object>  listener = *iter; // Hold an owning reference.

		// sprite, button & input_edit_text characters
		character* ch = (character*) listener.get_ptr();
		ch->on_event(event_id(event_id::KEY_PRESS, (key::code) k));
	}
}

void movie_root::add_keypress_listener(as_object* listener)
{
	std::vector< as_object* >::const_iterator end = m_keypress_listeners.end();
	for (std::vector< as_object* >::iterator iter = m_keypress_listeners.begin();
			iter != end; ++iter) 
	{
		if (*iter == NULL)
		{
			// Already in the list.
			return;
		}
	}
	m_keypress_listeners.push_back(listener);
}

void movie_root::remove_keypress_listener(as_object* listener)
{
	for (std::vector< as_object* >::iterator iter = m_keypress_listeners.begin();
			iter != m_keypress_listeners.end(); )
	{
		if (*iter == listener)
		{
			iter = m_keypress_listeners.erase(iter);
			continue;
		}
		iter++;
	}
}

movie* movie_root::get_active_entity()
{
	return m_active_input_text;
}

void movie_root::set_active_entity(movie* ch)
{
	m_active_input_text = ch;
}

} // namespace gnash

