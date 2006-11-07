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
		
		// always scale proportionally
		if (_xscale < _yscale) _yscale = _xscale;
		if (_yscale < _xscale) _xscale = _yscale;
		
		_renderer->set_scale(_xscale, _yscale);

	} else {
		log_warning("Resize request received while there's still"
			" no movie loaded, can't correctly set movie scale");
	}
	
	// trigger redraw
	_redraw_flag |= (_width!=width) || (_height!=height);

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

	if ( m->notify_mouse_moved(x, y) )
	{
		// any action triggered by the
		// event required screen refresh
		display(m);
	}
    
	if ( m->isMouseOverActiveEntity() ) {
		setCursor(CURSOR_HAND);
	} else {
		setCursor(CURSOR_NORMAL);
	}

}

void
Gui::notify_mouse_clicked(bool mouse_pressed, int mask) 
{
	movie_interface* m = get_current_root();

	if ( m->notify_mouse_clicked(mouse_pressed, mask) )
	{
		// any action triggered by the
		// event required screen refresh
		display(m);
	}
}

bool
Gui::display(gnash::movie_interface* m)
{
	rect changed_bounds;  // area of the stage that must be updated 
	bool redraw_flag;

	// Should the frame be rendered completely, even if it did not change?
	redraw_flag = _redraw_flag || want_redraw();
	
	// reset class member if we do a redraw now
	if (redraw_flag) _redraw_flag=false;

	// Find out the surrounding frame of all characters which
	// have been updated. This just checks what region of the stage has changed
	// due to ActionScript code, the timeline or user events. The GUI can still
  // choose to render a different part of the stage. 
	m->get_invalidated_bounds(&changed_bounds, false);

	if (redraw_flag)     // TODO: Remove this and want_redraw to avoid confusion!?
	{
		// TODO: use more meaningful ordinate values ?
		changed_bounds = rect(-1e10f, -1e10f, +1e10f, +1e10f);
	}
  
  
	// Avoid drawing of stopped movies

	if ( ! changed_bounds.is_null() )	//vv
	{
		// Tell the GUI(!) that we only need to update this region. Note the GUI can
		// do whatever it wants with this information. It may simply ignore the 
		// bounds (which will normally lead into a complete redraw), or it may
		// extend or shrink the bounds as it likes. So, by calling 
    // set_invalidated_bounds we have no guarantee that only this part of the
    // stage is rendered again.
		set_invalidated_region(changed_bounds);

		// render the frame. It's up to the GUI/renderer combination to do any
    // clipping, if desired.     
		m->display();
  
		// show invalidated region using a red rectangle
		// (Flash debug style)
		IF_DEBUG_REGION_UPDATES (
			point corners[4];
			float xmin = changed_bounds.get_x_min();
			float xmax = changed_bounds.get_x_max();
			float ymin = changed_bounds.get_y_min();
			float ymax = changed_bounds.get_y_max();

			corners[0].m_x = xmin;
			corners[0].m_y = ymin;
			corners[1].m_x = xmax;
			corners[1].m_y = ymin;
			corners[2].m_x = xmax;
			corners[2].m_y = ymax;
			corners[3].m_x = xmin;
			corners[3].m_y = ymax;
			matrix dummy;    	
			gnash::render::set_matrix(dummy); // reset matrix
			gnash::render::draw_poly(&corners[0], 4,
				rgba(0,0,0,0), rgba(255,0,0,255));
		);

		// show frame on screen
		renderBuffer();
   	
	};
  
	return true;
}

bool
Gui::advance_movie(Gui* gui)
{
	assert(gui);
  
//	GNASH_REPORT_FUNCTION;

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

void
Gui::setCursor(gnash_cursor_type /*newcursor*/)
{
}

// end of namespace
}

