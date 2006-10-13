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

#include "sdl_cairo_glue.h"
#include "log.h"
#include "render_handler_cairo.h"

using namespace std;


namespace gnash
{

SdlCairoGlue::SdlCairoGlue()
#ifdef FIX_I810_LOD_BIAS
  : _tex_lod_bias(-1.2f)
#endif
{
//    GNASH_REPORT_FUNCTION;
}

SdlCairoGlue::~SdlCairoGlue()
{
//    GNASH_REPORT_FUNCTION;
    cairo_surface_destroy(_cairo_surface);
    cairo_destroy (_cairo_handle);
    SDL_FreeSurface(_sdl_surface);
    SDL_FreeSurface(_screen);
    delete [] _render_image;
}

bool
SdlCairoGlue::init(int argc, char** argv[])
{
//    GNASH_REPORT_FUNCTION;
#ifdef FIX_I810_LOD_BIAS
    int c = getopt (argc, *argv, "m:");
    if (c == 'm') {
      _tex_lod_bias = (float) atof(optarg);
    }
#endif

    return true;
}


render_handler*
SdlCairoGlue::createRenderHandler()
{
//    GNASH_REPORT_FUNCTION;

    return renderer::cairo::create_handler();

}


bool
SdlCairoGlue::prepDrawingArea(int width, int height, int depth, uint32_t sdl_flags)
{
    _screen = SDL_SetVideoMode(width, height, depth, sdl_flags | SDL_SWSURFACE);

    if (!_screen) {
        fprintf(stderr, "SDL_SetVideoMode() failed.\n");
        exit(1);
    }
    
    int stride=width * 4;

    _render_image = new unsigned char[stride * height];
    // XXX is there a need for zeroing out _render_image?

    _cairo_surface =
      cairo_image_surface_create_for_data (_render_image, CAIRO_FORMAT_ARGB32,
                                           width, height, stride);

    _cairo_handle = cairo_create(_cairo_surface);

    renderer::cairo::set_handle(_cairo_handle);

    uint32_t rmask, gmask, bmask, amask;

    rmask = 0x00ff0000;
    gmask = 0x0000ff00;
    bmask = 0x000000ff;
    amask = 0xff000000;

    _sdl_surface = SDL_CreateRGBSurfaceFrom((void *) _render_image, width, height,
                                           depth, stride, rmask, gmask, bmask, amask);
    assert(_sdl_surface);

    return true;
}

void
SdlCairoGlue::render()
{
//    GNASH_REPORT_FUNCTION;
    SDL_BlitSurface(_sdl_surface, NULL, _screen, NULL);
    SDL_UpdateRect (_screen, 0, 0, 0, 0);
}


} // namespace gnash
