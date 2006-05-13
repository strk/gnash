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

#include "log.h"
#include "gnash.h"
#include "movie_definition.h"
#include "gui.h"

namespace gnash {

const char *GNASH = "Gnash";

Gui::Gui() :
    _loop(true),
    _xid(0),
    _width(0),
    _height(0),
    _mouse_x(0),
    _mouse_y(0),
    _scale(0.1f),
    _mouse_buttons(0),
    _xembed(0),
    _depth(16)
#if defined(FIX_I810_LOD_BIAS)
   ,_tex_lod_bias(-1.2f)
#endif
{
//    GNASH_REPORT_FUNCTION;
}

Gui::Gui(unsigned long xid, float scale, bool loop, unsigned int depth) :
    _loop(true),
    _xid(0),
    _width(0),
    _height(0),
    _mouse_x(0),
    _mouse_y(0),
    _scale(0.1f),
    _mouse_buttons(0),
    _xembed(0),
    _depth(16)
#if defined(FIX_I810_LOD_BIAS)
   ,_tex_lod_bias(-1.2f)
#endif
{
}

bool
Gui::init(int xid, int argc, char **argv[])
{
  _xembed = true;
  _xid = xid;
//  return init(argc, argv);
    return false;
}

bool
Gui::createWindow(int xid, int width, int height)
{
//    GNASH_REPORT_FUNCTION;
    _xembed = true;
    _xid = xid;
//  return createWindow(width, height);
    return false;
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
        m->set_display_viewport(0, 0, width, height);
    }
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

bool
Gui::advance_movie(void *data)
{
//    GNASH_REPORT_FUNCTION;
    
    Gui *gui = reinterpret_cast<Gui*> (data);
    gnash::movie_interface* m = gnash::get_current_root();

    m->notify_mouse_state(gui->getMouseX(), gui->getMouseY(), gui->getMouseButtons());

    m->advance(1.0);
    m->display();
    
    gui->renderBuffer();
    
    if (!gui->loops()) {
        if (m->get_current_frame() + 1 ==
            m->get_root_movie()->get_movie_definition()->get_frame_count()) {
            exit(0); // TODO: quit in a more gentile fashion.
        }
    }

    return true;
}

// end of namespace
}

