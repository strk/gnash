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

#include "sprite_instance.h"
#include "gui.h"
#include "render.h"  // debug
#include "render_handler.h"
#include "movie_root.h"
#include "VM.h"

#include <cstdio>
#include <cstring>

/// Define this to have updated regions enclosed in a red rectangle
/// In the future, enabling this might actually use a runtime flag
/// as an additional conditional.
///
//#define ENABLE_REGION_UPDATES_DEBUGGING 1

/// Define this if you want to debug the *detection* of region updates only.
/// This will disable region updates for the backend (GUI+renderer) completely 
/// so that only the last region (red frame) will be visible. However, this 
/// slows down rendering as each frame is fully re-rendered. If you want to 
/// debug the GUI part, however (see if blitting the region works), then you 
/// probably won't define this. 
#define REGION_UPDATES_DEBUGGING_FULL_REDRAW 1 

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
    _width(1),
    _height(1),
    _xscale(1.0f),
    _yscale(1.0f),
    _depth(16),
    _interval(0),
    _renderer(NULL),
    _redraw_flag(true)
{
//    GNASH_REPORT_FUNCTION;
}

Gui::Gui(unsigned long xid, float scale, bool loop, unsigned int depth)
	:
    _loop(loop),
    _xid(xid),
    _width(1),
    _height(1),
    _xscale(scale),
    _yscale(scale),
    _depth(depth),
    _interval(0),
    _renderer(NULL),
    _redraw_flag(true)
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
//	GNASH_REPORT_FUNCTION;

	assert(width>0);
	assert(height>0);

	if ( VM::isInitialized() )
	{

		movie_root& m = VM::get().getRoot();

		movie_definition* md = m.get_movie_definition();

		float swfwidth = md->get_width_pixels();
		float swfheight = md->get_height_pixels();

		m.set_display_viewport(0, 0, width, height);

		// set new scale value
		_xscale = width / swfwidth;
		_yscale = height / swfheight;
		
		// always scale proportionally
		if (_xscale < _yscale) _yscale = _xscale;
		if (_yscale < _xscale) _xscale = _yscale;
		
		_renderer->set_scale(_xscale, _yscale);

	}
	else
	{
		log_warning("Resize request received while there's still"
			" no movie loaded, can't correctly set movie scale");
	}
	
	// trigger redraw
	_redraw_flag |= (_width!=width) || (_height!=height);

	// set new size ?
	_width = width;
	_height = height;
	_validbounds.setTo(0, 0, _width, _height);
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
    get_current_root()->set_play_state(gnash::sprite_instance::PLAY);
}

void
Gui::menu_pause()
{
//    GNASH_REPORT_FUNCTION;

    movie_root* m = get_current_root();
    if (m->get_play_state() == gnash::sprite_instance::STOP) {
      m->set_play_state(gnash::sprite_instance::PLAY);
    } else {
      m->set_play_state(gnash::sprite_instance::STOP);
    }

}

void
Gui::menu_stop()
{
//    GNASH_REPORT_FUNCTION;
    get_current_root()->set_play_state(gnash::sprite_instance::STOP);
}

void
Gui::menu_step_forward()
{
//    GNASH_REPORT_FUNCTION;
	movie_root* m = get_current_root();
	m->goto_frame(m->get_current_frame()+1);
}

void
Gui::menu_step_backward()
{
//    GNASH_REPORT_FUNCTION;

	movie_root* m = get_current_root();
	m->goto_frame(m->get_current_frame()-1);
}

void
Gui::menu_jump_forward()
{
//    GNASH_REPORT_FUNCTION;

	movie_root* m = get_current_root();
	m->goto_frame(m->get_current_frame()+10);
}

void
Gui::menu_jump_backward()
{
//    GNASH_REPORT_FUNCTION;

	movie_root* m = get_current_root();
	m->goto_frame(m->get_current_frame()-10);
}

void
Gui::menu_toggle_sound()
{
//    GNASH_REPORT_FUNCTION;
    sound_handler* snd_handler = get_sound_handler();

    if (!snd_handler)
       return;

    if (snd_handler->is_muted()) {
       snd_handler->unmute();
    } else {
       snd_handler->mute();
    }
}


void
Gui::notify_mouse_moved(int x, int y) 
{
	movie_root* m = get_current_root();

	//log_msg("mouse @ %d,%d", x, y);
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
	movie_root* m = get_current_root();
	assert(m);

	if ( m->notify_mouse_clicked(mouse_pressed, mask) )
	{
		// any action triggered by the
		// event required screen refresh
		display(m);
	}
}

bool
Gui::display(movie_root* m)
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
	//
	// TODO: is it needed to call get_invalidated_bounds even when redraw_flag is true ??
	//
	m->get_invalidated_bounds(&changed_bounds, false);

	if (redraw_flag)     // TODO: Remove this and want_redraw to avoid confusion!?
	{
		changed_bounds.set_world();
	}
  
	// Avoid drawing of stopped movies

	if ( ! changed_bounds.is_null() ) // use 'else'?
	{
		// Tell the GUI(!) that we only need to update this
		// region. Note the GUI can do whatever it wants with
		// this information. It may simply ignore the bounds
		// (which will normally lead into a complete redraw),
		// or it may extend or shrink the bounds as it likes. So,
		// by calling set_invalidated_bounds we have no guarantee
		// that only this part of the stage is rendered again.
#ifdef REGION_UPDATES_DEBUGGING_FULL_REDRAW
		// redraw the full screen so that only the
		// *new* invalidated region is visible
		// (helps debugging)
		rect worldregion; worldregion.set_world();
		setInvalidatedRegion(worldregion);
#else
		setInvalidatedRegion(changed_bounds);
#endif

		// render the frame.
		// It's up to the GUI/renderer combination
		// to do any clipping, if desired.     
		m->display();
  
		// show invalidated region using a red rectangle
		// (Flash debug style)
		IF_DEBUG_REGION_UPDATES (
		if ( ! changed_bounds.is_world() )
		{
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
		}
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

	gnash::movie_root* m = gnash::get_current_root();

// Define REVIEW_ALL_FRAMES to have *all* frames
// consequencially displaied. Useful for debugging.
//#define REVIEW_ALL_FRAMES 1

#ifndef REVIEW_ALL_FRAMES
	// Advance movie by one frame
	m->advance(1.0);
#else
	size_t cur_frame = m->get_root_movie()->get_current_frame();
	size_t tot_frames = m->get_root_movie()->get_frame_count();
	m->advance(1.0);
	m->get_movie_definition()->ensure_frame_loaded(tot_frames);
	m->goto_frame(cur_frame+1);
    	m->set_play_state(gnash::sprite_instance::PLAY);
	log_msg("Frame %d", m->get_current_frame());
#endif

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

bool
Gui::want_redraw()
{
    return false;
}

float
Gui::getXScale()
{
    return _xscale;
}

float
Gui::getYScale()
{
    return _yscale;
}

bool
Gui::loops()
{
    return _loop;
}

void
Gui::setInvalidatedRegion (const rect& bounds)
{
}

// end of namespace
}

