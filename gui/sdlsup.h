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

#ifndef __SDLSUP_H__
#define __SDLSUP_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h"
#include "gui.h"

#include "SDL.h"
#include "SDL_thread.h"


#ifdef RENDERER_CAIRO
# include <cairo.h>
#endif

namespace gnash
{

class DSOEXPORT SDLGui : public Gui
{
public:
    SDLGui(unsigned long xid, float scale, bool loop, unsigned int depth);
    virtual ~SDLGui();
    virtual bool init(int argc, char **argv[]);
    virtual bool createWindow( int width, int height);
    virtual bool createWindow(const char *title, int width, int height);
    virtual bool run();
    virtual bool createMenu();
    virtual bool setupEvents();
    virtual void renderBuffer();
    virtual void setInterval(unsigned int interval);
    virtual void disableCoreTrap();
    virtual void setTimeout(unsigned int timeout);

		void key_event(SDLKey key, bool down);

private:
    unsigned int    _timeout;
    SDL_Surface     *_screen;

    /// Handle VIDEORESIZE event
    void resize_event();

    /// Handle VIDEOEXPOSE event
    void expose_event();

#ifdef RENDERER_CAIRO
    cairo_surface_t *_cairo_surface;
    cairo_t         *_cairo_handle;
    SDL_Surface     *_sdl_surface;
    unsigned char   *_render_image;

#endif
    bool _core_trap;
#ifdef FIX_I810_LOD_BIAS
    float _tex_lod_bias;
#endif
};
 
// void xt_event_handler(Widget xtwidget, gpointer instance,
// 		 XEvent *xevent, Boolean *b);

// end of namespace gnash 
}

// end of __SDLSUP_H__
#endif
