// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#ifndef GNASH_FLTKSUP_H
#define GNASH_FLTKSUP_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif


#include "log.h"
#include "gui.h"

#include <vector>
#include <fltk/Window.h>
#include <fltk/PopupMenu.h>

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
    FltkGui(unsigned long xid, float scale, bool loop, RunResources& r);

    virtual ~FltkGui();
    virtual bool init(int argc, char **argv[]);

    virtual void setInterval(unsigned int);
    virtual void setTimeout(unsigned int) { }
    virtual bool createWindow(const char *title, int width, int height,
                              int xPosition = 0, int yPosition = 0);
    virtual void renderBuffer();

    virtual bool run();
    virtual bool createMenu();
    virtual void setCursor(gnash_cursor_type newcursor);
    virtual bool setupEvents() { return true;}

    void setInvalidatedRegions(const InvalidatedRanges& ranges);

    void create();
    int handle(int event);
    void layout();
    void addMenuItems();
 private:
    void handleKey(unsigned key);

    fltk::PopupMenu  *_popup_menu;
    float _interval;
    unsigned int _menu_height;

    std::vector< geometry::Range2d<int> > _drawbounds_vec;

#ifdef RENDERER_AGG
    FltkAggGlue *_glue;
#elif defined(RENDERER_CAIRO)
    FltkCairoGlue* _glue;
#endif
};

} // end of namespace gnash 

#endif // end of __FLTKSUP_H__
