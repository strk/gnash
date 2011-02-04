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

#include <fltk/draw.h>

#include "fltk_glue_agg.h"

#include "Renderer.h"
#include "Renderer_agg.h"
#include "GnashException.h"

using namespace std;

namespace gnash {

FltkAggGlue::FltkAggGlue(int x, int y, int width, int height)
 : Widget(x, y, width, height),
   _offscreenbuf(NULL)
{
}

FltkAggGlue::~FltkAggGlue()
{
}

Renderer*
FltkAggGlue::createRenderHandler()
{
    _renderer = create_Renderer_agg("RGB24");
    if (_renderer == NULL)
        throw GnashException(_("Could not create AGG renderer with pixelformat RGB24"));
    return _renderer;
}

void
FltkAggGlue::initBuffer(int width, int height)
{
    assert(_renderer);

    int _bpp = 24;
    int depth_bytes = _bpp / 8;  // TODO: <Udo> is this correct? Gives 1 for 15 bit modes!

    assert(_bpp % 8 == 0);

    _stride = width * depth_bytes;

#define CHUNK_SIZE (100 * 100 * depth_bytes)

    int bufsize = (width * height * depth_bytes / CHUNK_SIZE + 1) * CHUNK_SIZE;

    _offscreenbuf = new unsigned char[bufsize];

    // Only the AGG renderer has the function init_buffer, which is *not* part of
    // the renderer api. It allows us to change the renderers movie size (and buffer
    // address) during run-time.
    Renderer_agg_base * renderer =
      static_cast<Renderer_agg_base *>(_renderer);
    renderer->init_buffer(_offscreenbuf, bufsize, width, height, 
      width*((_bpp+7)/8));

    _width = width;
    _height = height;

    _validbounds.setTo(0, 0, _width-1, _height-1);
    _drawbounds = _validbounds;

}

void
FltkAggGlue::render(geometry::Range2d<int>& bounds)
{
    _drawbounds = bounds;
    redraw();
}

void
FltkAggGlue::draw()
{
    // Calculate the position of the first pixel within the invalidated
    // rectangle in _offscreenbuf.
    ptrdiff_t offset = _drawbounds.getMinY() * _stride + _drawbounds.getMinX() * 3;

    Rectangle bounds(_drawbounds.getMinX(), _drawbounds.getMinY(), _drawbounds.width(), _drawbounds.height());

    fltk::drawimage(_offscreenbuf + offset, fltk::RGB, bounds, _stride);
}

void
FltkAggGlue::resize(int width, int height)
{
    GNASH_REPORT_FUNCTION;
    if (!_offscreenbuf) {
      // If initialisation has not taken place yet, we don't want to touch this.
      return;
    }

    delete [] _offscreenbuf;
    initBuffer(width, height);
}

} // namespace gnash
