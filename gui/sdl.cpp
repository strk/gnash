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
// Linking Gnash statically or dynamically with other modules is making
// a combined work based on Gnash. Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
// 
// In addition, as a special exception, the copyright holders of Gnash give
// you permission to combine Gnash with free software programs or
// libraries that are released under the GNU LGPL and/or with Mozilla, 
// so long as the linking with Mozilla, or any variant of Mozilla, is
// through its standard plug-in interface. You may copy and distribute
// such a system following the terms of the GNU GPL for Gnash and the
// licenses of the other code concerned, provided that you include the
// source code of that other code when and as the GNU GPL requires
// distribution of source code. 
// 
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is
// their choice whether to do so.  The GNU General Public License gives
// permission to release a modified version without this exception; this
// exception also makes it possible to release a modified version which
// carries forward this exception.
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include "gnash.h"
#include "log.h"
#include "Movie.h"

#include "sdlsup.h"

#ifdef RENDERER_OPENGL
#include <GL/gl.h>
#include <GL/glu.h>
#endif // RENDERER_OPENGL

#define OVERSIZE	1.0f


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

    SDL_Event	event;
    while (true) {
      if (_timeout && SDL_GetTicks() >= _timeout) {
        break;
      }
      _func(this);

      for (unsigned int i=0; i < _interval; i++) {
        SDL_PollEvent(&event);

        switch (event.type) {
          case SDL_MOUSEMOTION:
            _mouse_x = (int) (event.motion.x /*/ s_scale*/); // XXX
            _mouse_y = (int) (event.motion.y /*/ s_scale*/); // XXX
            break;
          case SDL_MOUSEBUTTONDOWN:
          case SDL_MOUSEBUTTONUP:
          {
            int	mask = 1 << (event.button.button - 1);
            if (event.button.state == SDL_PRESSED) {
                _mouse_buttons |= mask;
            } else {
                _mouse_buttons &= ~mask;
            }
            break;
          }
          case SDL_KEYDOWN:
          if (event.key.keysym.sym == SDLK_ESCAPE)
            return true;
          break;
          case SDL_QUIT:
            return true;
          break;
        }
        SDL_Delay(1);
      }
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

#ifdef RENDERER_OPENGL
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
    _name = basename(*argv[0]);

    return false;
}

bool
SDLGui::createWindow(int width, int height)
{
    GNASH_REPORT_FUNCTION;
    _width = width;
    _height = height;

    Uint32 sdl_flags;
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

    // Set the window title
    SDL_WM_SetCaption(_name.c_str(), _name.c_str());

#ifdef RENDERER_CAIRO
    int stride=width * 4;

    _render_image = (unsigned char *)calloc(stride*height, 1);

    _cairo_surface =
      cairo_image_surface_create_for_data (_render_image, CAIRO_FORMAT_ARGB32,
                                           _width, _height, stride);

    _cairo_handle = cairo_create(_cairo_surface);

    _renderer = create_render_handler_cairo((void*)_cairo_handle);

#elif defined (RENDERER_OPENGL)
    _renderer = create_render_handler_ogl();
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
    Uint32 rmask, gmask, bmask, amask;

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
SDLGui::setCallback(callback_t func, unsigned int interval)
{
    _func = func;
    _interval = interval;
}

void
SDLGui::resizeWindow()
{
    GNASH_REPORT_FUNCTION;
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


} // namespace gnash


