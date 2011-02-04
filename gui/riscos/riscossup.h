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


#ifndef __ROSUP_H__
#define __ROSUP_H__

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "gnash.h"

#include "oslib/colourtrans.h"
#include "oslib/wimp.h"

#ifdef RENDERER_AGG
#include "riscos_glue_agg.h"
#endif

#include "gui.h"

namespace gnash
{

class RiscosGui : public Gui
{
 public:
    RiscosGui(unsigned long xid, float scale, bool loop, RunResources& r);
    virtual ~RiscosGui();
    virtual bool init(int argc, char **argv[]);
    virtual bool createWindow(int width, int height);
    virtual bool createWindow(const char *title, int width, int height,
                              int xPosition = 0, int yPosition = 0);
    virtual bool run();
    virtual bool createMenu();
    virtual bool setupEvents();
    virtual void renderBuffer();
    virtual void setInterval(unsigned int interval);
    virtual void setTimeout(unsigned int timeout);
    virtual void setInvalidatedRegion(const SWFRect& bounds);


 private:
    bool create_window();
    int valid_coord(int coord, int max);

    wimp_t _task;
    wimp_w _window;
    bool _quit;
    os_t _timeout;

    int m_draw_minx;
    int m_draw_miny;
    int m_draw_maxx;
    int m_draw_maxy;

    int _screen_height;
    int _screen_width;

#ifdef RENDERER_AGG
    RiscosAggGlue glue;
#endif
};

}

#endif
