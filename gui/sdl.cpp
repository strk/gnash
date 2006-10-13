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

/* $Id: sdl.cpp,v 1.37 2006/10/13 15:56:34 strk Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstdio>

#if defined(_WIN32) || defined(WIN32)
	#define basename(x) x
	#include "getopt_win32.h"
#else
	#include <unistd.h>

#ifndef __THROW
# ifndef __GNUC_PREREQ
#  define __GNUC_PREREQ(maj, min) (0)
# endif
# if defined __cplusplus && __GNUC_PREREQ (2,8)
#  define __THROW       throw ()
# else
#  define __THROW
# endif
#endif

	extern int getopt(int, char *const *, const char *) __THROW;
	#include <libgen.h> //For POSIX basename().

#endif // Win32

#include "gnash.h"
#include "log.h"
#include "sdlsup.h"

#ifdef RENDERER_CAIRO
	#include "render_handler_cairo.h"
#endif // RENDERER_CAIRO

#ifdef RENDERER_OPENGL
	#include "tu_opengl_includes.h"
#endif // RENDERER_OPENGL

using namespace std;

namespace gnash 
{

SDLGui::SDLGui(unsigned long xid, float scale, bool loop, unsigned int depth)
 : Gui(xid, scale, loop, depth),
   _timeout(0),
   _core_trap(true)
{

}

SDLGui::~SDLGui()
{
    GNASH_REPORT_FUNCTION;

#ifdef RENDERER_CAIRO
    cairo_surface_destroy(_cairo_surface);
    cairo_destroy (_cairo_handle);
    SDL_FreeSurface(_sdl_surface);
    SDL_FreeSurface(_screen);
    free(_render_image);
#endif
}

bool
SDLGui::run()
{
	GNASH_REPORT_FUNCTION;
	int x_old = -1;
	int y_old = -1;
	int button_state_old = -1;

	SDL_Event	event;
	while (true)
	{

		if (_timeout && SDL_GetTicks() >= _timeout)
		{
			break;
		}

		Uint32 start_tick = SDL_GetTicks();

		while (true)
		{
			if (SDL_PollEvent(&event) == 0)
			{
				break;
			}

			switch (event.type)
			{
          case SDL_MOUSEMOTION:
            // SDL can generate MOUSEMOTION events even without mouse movement
            if (event.motion.x == x_old && event.motion.y == y_old) { break; }
            x_old = event.motion.x;
            y_old = event.motion.y;
            notify_mouse_moved((int) (x_old / _xscale), (int) (y_old / _yscale));
            break;
          case SDL_MOUSEBUTTONDOWN:
          case SDL_MOUSEBUTTONUP:
          {
            int	mask = 1 << (event.button.button - 1);
            if (event.button.state == SDL_PRESSED) {
                // multiple events will be fired while the mouse is held down
                // we are interested only in a change in the mouse state:
                if (event.button.button == button_state_old) { break; }
                notify_mouse_clicked(true, mask);
                button_state_old = event.button.button;
            } else {
                notify_mouse_clicked(false, mask);
                button_state_old = -1;
            }
            break;
          }
          case SDL_KEYDOWN:
					{
						if (event.key.keysym.sym == SDLK_ESCAPE)
						{
							return true;
						}
            key_event(event.key.keysym.sym, true);
						break;
					}
          case SDL_KEYUP:
					{
            SDLKey	key = event.key.keysym.sym;
            key_event(key, false);	     
						break;
					}

          case SDL_VIDEORESIZE:
		resize_event();
		break;

          case SDL_VIDEOEXPOSE:
		expose_event();
		break;

          case SDL_QUIT:
            return true;
          break;
			}
		}

		Gui::advance_movie(this);

		int delay = _interval - (SDL_GetTicks() - start_tick);
		if (delay < 0)
		{
			delay = 0;
		}
		SDL_Delay(delay);
	}
	return false;
}


void
SDLGui::setTimeout(unsigned int timeout)
{
    _timeout = timeout;
}

bool
SDLGui::init(int argc, char **argv[])
{
    GNASH_REPORT_FUNCTION;

	int c;
	while ((c = getopt (argc, *argv, "m:c")) != -1)
	{
		switch (c)
		{
#ifdef FIX_I810_LOD_BIAS
			case 'm':
				_tex_lod_bias = (float) atof(optarg);
				break;
#endif
			case 'c':
				disableCoreTrap();
		}
	}

    if (_xid) {
      char SDL_windowhack[32];
      sprintf (SDL_windowhack,"SDL_WINDOWID=%ld", _xid);
      putenv (SDL_windowhack);
    }

    // Initialize the SDL subsystems we're using. Linux
    // and Darwin use Pthreads for SDL threads, Win32
    // doesn't. Otherwise the SDL event loop just polls.
    if (SDL_Init(SDL_INIT_VIDEO)) {
      fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
      exit(1);
    }

    atexit(SDL_Quit);

    SDL_EnableKeyRepeat(250, 33);

#ifdef RENDERER_CAIRO
    _renderer = renderer::cairo::create_handler();

#elif defined(RENDERER_OPENGL)

    _renderer = create_render_handler_ogl();

    if (_depth == 16) {
      // 16-bit color, surface creation is likely to succeed.
      SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
      SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
      SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
      SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 15);
      SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
      SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
    } else {
      assert(_depth == 32);

      // 32-bit color etc, for getting dest alpha,
      // for MULTIPASS_ANTIALIASING (see
      // render_handler_ogl.cpp).
      SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
      SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
      SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
    }
#endif // RENDERER_OPENGL
#if 0
    // So this is currently unused. We may want to do so.
    _name = basename(*argv[0]);
#endif

    return false;
}

bool
SDLGui::createWindow(const char *title, int width, int height)
{
   bool ret;

   ret = createWindow(width, height);

    // Set the window title
    SDL_WM_SetCaption( title, title);
    return ret;
}

bool
SDLGui::createWindow( int width, int height)
{
    GNASH_REPORT_FUNCTION;
    _width = width;
    _height = height;

    uint32_t sdl_flags;
#ifdef RENDERER_CAIRO
    sdl_flags = SDL_SWSURFACE;
#elif defined(RENDERER_OPENGL)
    sdl_flags = SDL_OPENGL;
#endif
    if (!_core_trap) {
      sdl_flags |= SDL_INIT_NOPARACHUTE;
    }

    if (_xid) {
      sdl_flags |= SDL_NOFRAME;
    }

    _screen = SDL_SetVideoMode(_width, _height, _depth, sdl_flags);

    if (!_screen) {
        fprintf(stderr, "SDL_SetVideoMode() failed.\n");
        exit(1);
    }

#ifdef RENDERER_CAIRO
    int stride=width * 4;

    _render_image = (unsigned char *)calloc(stride*height, 1);

    _cairo_surface =
      cairo_image_surface_create_for_data (_render_image, CAIRO_FORMAT_ARGB32,
                                           _width, _height, stride);

    _cairo_handle = cairo_create(_cairo_surface);

    renderer::cairo::set_handle(_cairo_handle);

#elif defined (RENDERER_OPENGL)
    // Turn on alpha blending.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                     
    // Turn on line smoothing.  Antialiased lines can be used to
    // smooth the outsides of shapes.
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST); // GL_NICEST, GL_FASTEST, GL_DONT_CARE
    glMatrixMode(GL_PROJECTION);

#define OVERSIZE 1.0f
    glOrtho(-OVERSIZE, OVERSIZE, OVERSIZE, -OVERSIZE, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
 
    // We don't need lighting effects
    glDisable(GL_LIGHTING);
 //   glColorPointer(4, GL_UNSIGNED_BYTE, 0, *);
//    glInterleavedArrays(GL_T2F_N3F_V3F, 0, *);
    glPushAttrib (GL_ALL_ATTRIB_BITS);         

#  ifdef FIX_I810_LOD_BIAS
    glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, _tex_lod_bias);
#  endif
#endif
    set_render_handler(_renderer);

    return false;
}

void
SDLGui::disableCoreTrap()
{
  _core_trap = false;
}

void
SDLGui::renderBuffer()
{
    GNASH_REPORT_FUNCTION;

#ifdef RENDERER_CAIRO
    uint32_t rmask, gmask, bmask, amask;

    rmask = 0x00ff0000;
    gmask = 0x0000ff00;
    bmask = 0x000000ff;
    amask = 0xff000000;

    int stride = _width * 4;

    _sdl_surface = SDL_CreateRGBSurfaceFrom((void *) _render_image, _width, _height,
                                           _depth, stride, rmask, gmask, bmask, amask);
    assert(_sdl_surface);
    SDL_BlitSurface(_sdl_surface, NULL, _screen, NULL);
    SDL_UpdateRect (_screen, 0, 0, 0, 0);
#elif defined(RENDERER_OPENGL)
    SDL_GL_SwapBuffers();
#endif
}

void
SDLGui::setInterval(unsigned int interval)
{
    _interval = interval;
}

bool
SDLGui::createMenu()
{
    GNASH_REPORT_FUNCTION;
    return false;
}

bool
SDLGui::setupEvents()
{
    GNASH_REPORT_FUNCTION;
    return false;
}

void SDLGui::key_event(SDLKey key, bool down)
// For forwarding SDL key events.
{
    gnash::key::code	c(gnash::key::INVALID);
    
    if (key >= SDLK_0 && key <= SDLK_9)	{
        c = (gnash::key::code) ((key - SDLK_0) + gnash::key::_0);
	} else if (key >= SDLK_a && key <= SDLK_z) {
        c = (gnash::key::code) ((key - SDLK_a) + gnash::key::A);
    } else if (key >= SDLK_F1 && key <= SDLK_F15)	{
        c = (gnash::key::code) ((key - SDLK_F1) + gnash::key::F1);
    } else if (key >= SDLK_KP0 && key <= SDLK_KP9) {
        c = (gnash::key::code) ((key - SDLK_KP0) + gnash::key::KP_0);
    } else {
        // many keys don't correlate, so just use a look-up table.
        struct {
            SDLKey	sdlk;
            gnash::key::code	gs;
        } table[] = {
            { SDLK_SPACE, gnash::key::SPACE },
            { SDLK_PAGEDOWN, gnash::key::PGDN },
            { SDLK_PAGEUP, gnash::key::PGUP },
            { SDLK_HOME, gnash::key::HOME },
            { SDLK_END, gnash::key::END },
            { SDLK_INSERT, gnash::key::INSERT },
            { SDLK_DELETE, gnash::key::DELETEKEY },
            { SDLK_BACKSPACE, gnash::key::BACKSPACE },
            { SDLK_TAB, gnash::key::TAB },
            { SDLK_RETURN, gnash::key::ENTER },
            { SDLK_ESCAPE, gnash::key::ESCAPE },
            { SDLK_LEFT, gnash::key::LEFT },
            { SDLK_UP, gnash::key::UP },
            { SDLK_RIGHT, gnash::key::RIGHT },
            { SDLK_DOWN, gnash::key::DOWN },
            // @@ TODO fill this out some more
            { SDLK_UNKNOWN, gnash::key::INVALID }
        };
        
        for (int i = 0; table[i].sdlk != SDLK_UNKNOWN; i++) {
            if (key == table[i].sdlk) {
                c = table[i].gs;
                break;
            }
        }
    }
    
    if (c != gnash::key::INVALID) {
        gnash::notify_key_event(c, down);
    }
}

void
SDLGui::resize_event()
{
	log_msg("got resize_event ");
}

void
SDLGui::expose_event()
{
	// TODO: implement and use set_invalidated_region instead?
	renderBuffer();
}


} // namespace gnash


