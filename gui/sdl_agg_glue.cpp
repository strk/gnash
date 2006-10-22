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

#include "sdl_agg_glue.h"
#include "log.h"
#include "render_handler.h"
#include "render_handler_agg.h"
#include <errno.h>
#include <ostream>

using namespace std;


namespace gnash
{

SdlAggGlue::SdlAggGlue()
{
//    GNASH_REPORT_FUNCTION;
}

SdlAggGlue::~SdlAggGlue()
{
//    GNASH_REPORT_FUNCTION;
    SDL_FreeSurface(_sdl_surface);
    SDL_FreeSurface(_screen);
    delete [] _offscreenbuf;
}

bool
SdlAggGlue::init(int argc, char** argv[])
{
//    GNASH_REPORT_FUNCTION;

    return true;
}


render_handler*
SdlAggGlue::createRenderHandler(int bpp)
{
//    GNASH_REPORT_FUNCTION;

    _bpp = bpp;

    switch (_bpp) {
      case 32:
        _agg_renderer = create_render_handler_agg("RGBA32");
        break;
      case 24:
        _agg_renderer = create_render_handler_agg("RGB24");
        break;
      case 16:
        _agg_renderer = create_render_handler_agg("RGBA16");
        break;
      default:
        dbglogfile << "ERROR: bit depth must be 16, 24 or 32 bits." << std::endl;
        assert(0);
    }
    return _agg_renderer;
}


bool
SdlAggGlue::prepDrawingArea(int width, int height, uint32_t sdl_flags)
{
    _width = width;
    _height = height;
    int depth_bytes = _bpp / 8;

    assert(_bpp % 8 == 0);

    _screen = SDL_SetVideoMode(width, height, _bpp, sdl_flags | SDL_SWSURFACE);

    if (!_screen) {
        fprintf(stderr, "SDL_SetVideoMode() failed.\n");
        exit(1);
    }

    int stride = width * depth_bytes;

    uint32_t rmask, gmask, bmask, amask;

    switch(_bpp) {
      case 32: // RGBA32
        rmask = 0xFF;
        gmask = 0xFF << 8;
        bmask = 0xFF << 16;
        amask = 0xFF << 24;
        break;
      case 24: // RGB24
        rmask = 0xFF;
        gmask = 0xFF << 8;
        bmask = 0xFF << 16;
        amask = 0;
        break;
      case 16: // RGB565: 5 bits for red, 6 bits for green, and 5 bits for blue
        rmask = 0x1F << 11;
        gmask = 0x3F << 5;
        bmask = 0x1F;
        amask = 0;
        break;
      default:
        assert(0);
    }

#define CHUNK_SIZE (100 * 100 * depth_bytes)

    int bufsize = static_cast<int>(width * height * depth_bytes / CHUNK_SIZE + 1) * CHUNK_SIZE;

    _offscreenbuf = new unsigned char[bufsize];

    log_msg("SDL-AGG: %i bytes offscreen buffer allocated\n", bufsize);


    // Only the AGG renderer has the function init_buffer, which is *not* part of
    // the renderer api. It allows us to change the renderers movie size (and buffer
    // address) during run-time.
    render_handler_agg_base * renderer =
      static_cast<render_handler_agg_base *>(_agg_renderer);
    renderer->init_buffer(_offscreenbuf, bufsize, width, height);


    _sdl_surface = SDL_CreateRGBSurfaceFrom((void *) _offscreenbuf, width, height,
                                           _bpp, stride, rmask, gmask, bmask, amask);
    assert(_sdl_surface);

    return true;
}

static int
valid_coord(int coord, int max)
{ 
    if (coord<0) return 0;
    else if (coord>=max) return max;

    return coord;
}


void
SdlAggGlue::render()
{
    int xmin, ymin, xmax, ymax;

    _agg_renderer->get_invalidated_region(xmin, ymin, xmax, ymax);

    // add two pixels because of anti-aliasing...
    xmin = valid_coord(xmin-2, _width);
    ymin = valid_coord(ymin-2, _height);
    xmax = valid_coord(xmax+2, _width);
    ymax = valid_coord(ymax+2, _height);

    // Our invalidated rectangle.
    SDL_Rect area;
    area.w = xmax - xmin;
    area.h = ymax - ymin;
    area.x = xmin;
    area.y = ymin;

    SDL_BlitSurface(_sdl_surface, &area, _screen, &area);

    SDL_UpdateRect (_screen, area.x, area.y, area.w, area.h);
}


} // namespace gnash
