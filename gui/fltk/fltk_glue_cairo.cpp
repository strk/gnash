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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <fltk/Item.h>
#include <fltk/ItemGroup.h>
#include <fltk/PopupMenu.h>
#include <fltk/Widget.h>
#include <fltk/ask.h>
#include <fltk/events.h>
#include <fltk/run.h>
#include <fltk/visual.h>
#include <fltk/Window.h>
#include <fltk/draw.h>
#include <fltk/x.h>
#include <fltk/damage.h>
#include <fltk/layout.h>
#include <fltk/Cursor.h>

#include "Renderer.h"
#include "Renderer_cairo.h"

#include "fltksup.h"
#include "fltk_glue_cairo.h"
#include "log.h"
#include "gui.h"

#include "RunResources.h"

using namespace std;
//using namespace fltk;

namespace gnash {

FltkCairoGlue::FltkCairoGlue()
 : _offscreenbuf(NULL)
{
}

FltkCairoGlue::~FltkCairoGlue()
{
    cairo_surface_destroy(_cairo_surface);
    delete [] _offscreenbuf;
}

Renderer*
FltkCairoGlue::createRenderHandler()
{
    _renderer = renderer::cairo::create_handler();
    return _renderer;
}

void
FltkCairoGlue::initBuffer(int width, int height)
{
    static bool firstTime = true;

    int _bpp = 32;
    int depth_bytes = _bpp / 8;

    assert(_bpp % 8 == 0);

    _stride = width * depth_bytes;

#define CHUNK_SIZE (100 * 100 * depth_bytes)

    //int bufsize = static_cast<int>(width * height * depth_bytes / CHUNK_SIZE + 1) * CHUNK_SIZE;

    int bufsize = height * _stride;

    _offscreenbuf = new unsigned char[bufsize];

    // CAIRO_FORMAT_RGB24 actualy means a 32-bit RGB word with the upper 8 bits
    // unused. Therefore we have allocated a 32-bit buffer.

   if (_cairo_surface)
        cairo_surface_destroy(_cairo_surface);
    if (_cairo_handle)
        cairo_destroy(_cairo_handle);

    _cairo_surface =
      cairo_image_surface_create_for_data (_offscreenbuf, CAIRO_FORMAT_RGB24,
                                           width, height, _stride);

    _cairo_handle = cairo_create(_cairo_surface);

    cairo_set_source_surface(_cairo_handle, cairo_get_target(_cairo_handle), 0, 0);
    renderer::cairo::set_context(_renderer, _cairo_handle);

    //renderer::cairo::set_handle(_cairo_handle);

    if (firstTime) {
      //set_Renderer(_renderer);
      _runResources.setRenderer(std::shared_ptr<Renderer>(_renderer));
      firstTime = false;
    }

    _width = width;
    _height = height;
}

void
FltkCairoGlue::draw()
{
    fltk::Rectangle area(0, 0, _width, _height);

    // CAIRO_FORMAT_RGB24 == fltk::RGB32
    fltk::drawimage(_offscreenbuf, fltk::RGB32, area);
}

void
FltkCairoGlue::invalidateRegion(const SWFRect& bounds)
{
    return;
}

void
FltkCairoGlue::resize(int width, int height)
{
    GNASH_REPORT_FUNCTION;
    cairo_surface_destroy(_cairo_surface);
    cairo_destroy (_cairo_handle);
    delete [] _offscreenbuf;
    initBuffer(width, height);
}

} // namespace gnash
