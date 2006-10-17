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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstdio>
#include <cstring>

//#include "log.h"
//#include "gnash.h"
//#include "movie_definition.h"
#include "sprite_instance.h"
#include "gui.h"
#include "render.h"  // debug
#include "render_handler.h"

/// Define this to have updated regions enclosed in a red rectangle
/// In the future, enabling this might actually use a runtime flag
/// as an additional conditional.
///
//#define ENABLE_REGION_UPDATES_DEBUGGING 1

#ifdef ENABLE_REGION_UPDATES_DEBUGGING
// a runtime check would make the { x; } block conditionally executed
#define IF_DEBUG_REGION_UPDATES(x) { x; }
#else
#define IF_DEBUG_REGION_UPDATES(x) 
#endif

namespace gnash {

Gui::Gui() :
    _loop(true),
    _xid(0),
    _width(0),
    _height(0),
    _xscale(1.0f),
    _yscale(1.0f),
    _depth(16)
{
//    GNASH_REPORT_FUNCTION;
  _last_invalidated_bounds.m_x_min = 0.0f;
  _last_invalidated_bounds.m_y_min = 0.0f;
  _last_invalidated_bounds.m_x_max = 0.0f;
  _last_invalidated_bounds.m_y_max = 0.0f;
}

Gui::Gui(unsigned long xid, float scale, bool loop, unsigned int depth) :
    _loop(loop),
    _xid(xid),
    _width(0),
    _height(0),
    _xscale(scale),
    _yscale(scale),
    _depth(depth)
{
  _last_invalidated_bounds.m_x_min = 0.0f;
  _last_invalidated_bounds.m_y_min = 0.0f;
  _last_invalidated_bounds.m_x_max = 0.0f;
  _last_invalidated_bounds.m_y_max = 0.0f;
}

Gui::~Gui()
{
//    GNASH_REPORT_FUNCTION;
    delete _renderer;
}

void
Gui::menu_restart()
{
//    GNASH_REPORT_FUNCTION;
    get_current_root()->restart();
}

void
Gui::resize_view(int width, int height)
{
//    GNASH_REPORT_FUNCTION;
	movie_interface* m = get_current_root();
	if (m) {

		movie_definition* md = m->get_movie_definition();
		float swfwidth = md->get_width_pixels();
		float swfheight = md->get_height_pixels();

		m->set_display_viewport(0, 0, width, height);

		// set new scale value
		_xscale = width / swfwidth;
		_yscale = height / swfheight;

	} else {
		log_warning("Resize request received while there's still"
			" no movie loaded, can't correctly set movie scale");
	}

	// set new size ?
	_width = width;
	_height = height;
	//log_msg("new size (in twips) is: %dx%d", _width*20, _height*20);

}

void
Gui::menu_quit()
{
//    GNASH_REPORT_FUNCTION;
    exit(0);
}

void
Gui::menu_play()
{
//    GNASH_REPORT_FUNCTION;
    get_current_root()->set_play_state(gnash::movie_interface::PLAY);
}

void
Gui::menu_pause()
{
//    GNASH_REPORT_FUNCTION;

    movie_interface* m = get_current_root();
    if (m->get_play_state() == gnash::movie_interface::STOP) {
      m->set_play_state(gnash::movie_interface::PLAY);
    } else {
      m->set_play_state(gnash::movie_interface::STOP);
    }

}

void
Gui::menu_stop()
{
//    GNASH_REPORT_FUNCTION;
    get_current_root()->set_play_state(gnash::movie_interface::STOP);
}

void
Gui::menu_step_forward()
{
//    GNASH_REPORT_FUNCTION;
    movie_interface* m = get_current_root();
    m->goto_frame(m->get_current_frame()+1);
}

void
Gui::menu_step_backward()
{
//    GNASH_REPORT_FUNCTION;
    movie_interface* m = get_current_root();
    m->goto_frame(m->get_current_frame()-1);
}

void
Gui::menu_jump_forward()
{
//    GNASH_REPORT_FUNCTION;
    movie_interface* m = get_current_root();
    m->goto_frame(m->get_current_frame()+10);
}

void
Gui::menu_jump_backward()
{
//    GNASH_REPORT_FUNCTION;
    movie_interface* m = get_current_root();
    m->goto_frame(m->get_current_frame()-10);
}

void
Gui::notify_mouse_moved(int x, int y) 
{
	movie_interface* m = get_current_root();
    	m->notify_mouse_moved(x, y);
}

void
Gui::notify_mouse_clicked(bool mouse_pressed, int mask) 
{
    get_current_root()->notify_mouse_clicked(mouse_pressed, mask);
}

bool
Gui::display(gnash::movie_interface* m)
{
	rect changed_bounds;  // new bounds for the current frame
	rect draw_bounds;     // redraw bounds (union of current and previous frame)
	bool redraw_flag;

	// Should the frame be rendered completely, even if it did not change?
	redraw_flag = want_redraw();

	// Find out the surrounding frame of all characters which
	// have been updated.
	m->get_invalidated_bounds(&changed_bounds, false);


	if (redraw_flag)
	{
		draw_bounds.m_x_min = -1e10f;
		draw_bounds.m_y_min = -1e10f;
		draw_bounds.m_x_max = +1e10f;
		draw_bounds.m_y_max = +1e10f;
	}
	else
	{
  
		// Union it with the previous frame (when a character moved,
		// we also need to redraw it's previous position).
		draw_bounds = changed_bounds;
		// TODO: the following condition seems bogus to me...
		//       what about always calling expand_to_rect
		//	 and let rect class take care of any check ?
		if (_last_invalidated_bounds.m_x_min <= _last_invalidated_bounds.m_x_max)  
		{
			draw_bounds.expand_to_rect(_last_invalidated_bounds);
		}
      
	}
  
  
	// Avoid drawing of stopped movies
	// TODO: the following condition seems a bit undeadable to me,
	//	 do we mean to catch an *empty* rect ? what about
	//	 adding a is_empty() method to rect class then ?
	if (draw_bounds.m_x_min <= draw_bounds.m_x_max)
	{
  
		// Tell the GUI that we only need to update this region
		// (it may ignore this information)
		set_invalidated_region(draw_bounds);

		// render the frame      
		m->display();
  
		// show invalidated region using a red rectangle
		// (Flash debug style)
		IF_DEBUG_REGION_UPDATES (
			point corners[4];
			corners[0].m_x = draw_bounds.m_x_min;    	
			corners[0].m_y = draw_bounds.m_y_min;    	
			corners[1].m_x = draw_bounds.m_x_max;    	
			corners[1].m_y = draw_bounds.m_y_min;    	
			corners[2].m_x = draw_bounds.m_x_max;    	
			corners[2].m_y = draw_bounds.m_y_max;    	
			corners[3].m_x = draw_bounds.m_x_min;    	
			corners[3].m_y = draw_bounds.m_y_max;
			matrix dummy;    	
			gnash::render::set_matrix(dummy); // reset matrix
			gnash::render::draw_poly(&corners[0], 4,
				rgba(0,0,0,0), rgba(255,0,0,255));
		);

		// show frame on screen
		renderBuffer();
   	
	}
  
	_last_invalidated_bounds = changed_bounds;
  
}

bool
Gui::advance_movie(Gui* gui)
{
	assert(gui);

  
//    GNASH_REPORT_FUNCTION;
	gnash::movie_interface* m = gnash::get_current_root();

  // Advance movie by one frame
	m->advance(1.0);

	gui->display(m);
	
	if ( ! gui->loops() )
	{
		size_t curframe = m->get_current_frame();
		gnash::sprite_instance* si = m->get_root_movie();
		if (curframe + 1 == si->get_frame_count())
		{
		    exit(0); // TODO: quit in a more gentile fashion.
		}
	}

	return true;
}

// end of namespace
}

