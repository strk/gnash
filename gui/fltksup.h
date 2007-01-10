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

#ifndef __FLTKSUP_H__
#define __FLTKSUP_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "log.h"
#include "gui.h"

#if 0
#include <FL/Fl.H>
#include <FL/gl.h>
#include <FL/x.H>
#include <FL/Fl_Window.H>
#endif

#include <fltk/Item.h>
#include <fltk/Menu.h>
#include <fltk/Window.h>
#include <fltk/PopupMenu.h>
#include <fltk/Widget.h>
#include <fltk/gl.h>
#include <fltk/visual.h>
#include <fltk/GlWindow.h>

#ifdef RENDERER_AGG
# include "fltk_glue_agg.h"
#elif defined(RENDERER_CAIRO)
# include "fltk_glue_cairo.h"
#endif

namespace gnash
{

class FltkGui : public fltk::Window, public Gui
{
public:
    FltkGui(unsigned long xid, float scale, bool loop, unsigned int depth);

    virtual ~FltkGui();
    virtual bool init(int argc, char **argv[]);

    virtual void setInterval(unsigned int);
    virtual void setTimeout(unsigned int) { }
    virtual bool createWindow(const char* title, int width, int height);
    virtual void renderBuffer();

    virtual bool run();
    virtual bool createMenu();
    virtual void setCursor(gnash_cursor_type newcursor);
    virtual bool setupEvents() { return true;}

    void set_invalidated_region(const rect& bounds);

    void create();
    void draw();
    int handle(int event);
    void layout();
 private:
    void handleKey(unsigned key);

    fltk::PopupMenu  *_popup_menu;
    float _interval;
#ifdef RENDERER_AGG
    FltkAggGlue _glue;
#elif defined(RENDERER_CAIRO)
    FltkCairoGlue _glue;
#endif
};
 
// end of namespace gnash 
}

// end of __FLTKSUP_H__
#endif
