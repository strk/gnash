// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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


#ifndef SDLSUP_H
#define SDLSUP_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "gui.h"

#include "SDL.h"
#include "SDL_thread.h"

#ifdef RENDERER_AGG
# include "sdl_agg_glue.h"
#elif defined(RENDERER_CAIRO)
# include "sdl_cairo_glue.h"
#elif defined(RENDERER_OPENGL)
# include "sdl_ogl_glue.h"
#endif


#ifdef RENDERER_CAIRO
# include <cairo.h>
#endif

namespace gnash
{

class SDLGui : public Gui
{
public:
    SDLGui(unsigned long xid, float scale, bool loop, RunResources& r);
    virtual ~SDLGui();
    virtual bool init(int argc, char **argv[]);
    virtual bool createWindow(const char *title, int width, int height,
                              int xPosition = 0, int yPosition = 0);
    virtual bool run();
    virtual bool createMenu();
    virtual bool setupEvents();
    virtual void renderBuffer();
    virtual void setInterval(unsigned int interval);
    virtual void disableCoreTrap();
    virtual void setTimeout(unsigned int timeout);

    void key_event(SDL_KeyboardEvent * key, bool down);

    // See gui.h for documentation
    void setInvalidatedRegions(const InvalidatedRanges& ranges);

private:
    unsigned int _timeout;
    bool         _core_trap;

    /// Handle VIDEORESIZE event
    void resize_event();

    /// Handle VIDEOEXPOSE event
    void expose_event();

    static key::code sdl_to_gnash_key(SDL_KeyboardEvent* key);
    static int sdl_to_gnash_modifier(int state);

#ifdef RENDERER_AGG
    SdlAggGlue          _glue;
#elif defined(RENDERER_CAIRO)
    SdlCairoGlue    _glue;
#elif defined(RENDERER_OPENGL)
    SdlOglGlue      _glue;
#endif

};
 
// void xt_event_handler(Widget xtwidget, gpointer instance,
//       XEvent *xevent, Boolean *b);

// end of namespace gnash 
}

// end of __SDLSUP_H__
#endif
