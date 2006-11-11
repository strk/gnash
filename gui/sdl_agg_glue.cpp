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
//

#include "sdl_agg_glue.h"
#include "log.h"
#include "render_handler.h"
#include "render_handler_agg.h"
#include <errno.h>
#include <ostream>

using namespace std;

namespace {
gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
}

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



} // namespace gnash
