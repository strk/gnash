//
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#include "Kde4GlueAgg.h"
#include "render_handler.h"
#include "render_handler_agg.h"
#include <QtGui/QImage>

namespace gnash
{

Kde4AggGlue::Kde4AggGlue()
:
  _width(0),
  _height(0),
   _renderer(0)
{
}

Kde4AggGlue::~Kde4AggGlue()
{
}

bool
Kde4AggGlue::init(int /* argc */, char *** /* argv */)
{
    return true;
}


void
Kde4AggGlue::prepDrawingArea(QWidget *drawing_area)
{
    _drawing_area = drawing_area;
}


void
Kde4AggGlue::initBuffer(int width, int height)
{
    if (!_renderer) return;

    _width = width;
    _height = height;

    int _bpp = 32;
    int depth_bytes = _bpp / 8;

    assert(_bpp % 8 == 0);

#define CHUNK_SIZE (100 * 100 * depth_bytes)

    int bufsize = (width * height * depth_bytes / CHUNK_SIZE + 1) * CHUNK_SIZE;

    _offscreenbuf.reset(new unsigned char[bufsize]);

    render_handler_agg_base * renderer =
      static_cast<render_handler_agg_base *>(_renderer);

    renderer->init_buffer(_offscreenbuf.get(), bufsize, _width, _height,
      width*((_bpp+7)/8));


    _validbounds.setTo(0, 0, _width, _height);
    _drawbounds.push_back(_validbounds);
    
    _image.reset(new QImage(_offscreenbuf.get(), _width, _height, QImage::Format_RGB32));
}

void
Kde4AggGlue::render()
{
    render(0, 0, _width, _height);
}

void
Kde4AggGlue::render(int minX, int minY, int maxX, int maxY)
{

    QRectF rectangle(minX, minY, maxX, maxY);

    QPainter p(_drawing_area);
   
    p.drawImage(rectangle, *_image);
    p.end();

}


void
Kde4AggGlue::setInvalidatedRegions(const InvalidatedRanges& ranges)
{
    _renderer->set_invalidated_regions(ranges);

    _drawbounds.clear();

    for (size_t rno=0; rno<ranges.size(); rno++) {

      geometry::Range2d<int> bounds = Intersection(
      _renderer->world_to_pixel(ranges.getRange(rno)),
      _validbounds);

      // it may happen that a particular range is out of the screen, which 
      // will lead to bounds==null. 
      if (bounds.isNull()) continue;

      assert(bounds.isFinite());

      _drawbounds.push_back(bounds);

    }
}


render_handler*
Kde4AggGlue::createRenderHandler()
{
    _renderer = create_render_handler_agg("BGRA32");

    if ( ! _renderer )
    {
        throw GnashException("Could not create AGG renderer with pixelformat ABGR32");
    }
    return _renderer;
}

void
Kde4AggGlue::resize(int width, int height)
{
    initBuffer(width, height);
}

// end of namespace gnash
}
