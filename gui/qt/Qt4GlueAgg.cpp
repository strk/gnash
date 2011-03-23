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


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "Qt4GlueAgg.h"
#include "Renderer.h"
#include "Renderer_agg.h"
#include "GnashException.h"
#include <QWidget>
#include <QImage>
#include <QRect>

namespace gnash
{

Qt4AggGlue::Qt4AggGlue()
:
  _width(0),
  _height(0),
  _renderer(0)
{
}

Qt4AggGlue::~Qt4AggGlue()
{
}

bool
Qt4AggGlue::init(int /* argc */, char *** /* argv */)
{
    return true;
}


void
Qt4AggGlue::prepDrawingArea(DrawingWidget *drawing_area)
{
    _drawing_area = drawing_area;
}


void
Qt4AggGlue::initBuffer(int width, int height)
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

    Renderer_agg_base * renderer =
      static_cast<Renderer_agg_base *>(_renderer);

    renderer->init_buffer(_offscreenbuf.get(), bufsize, _width, _height,
      width*((_bpp+7)/8));
    
    _image.reset(new QImage(_offscreenbuf.get(), _width, _height, QImage::Format_RGB32));
}


void
Qt4AggGlue::render()
{
    QRect r(0, 0, _width, _height);
    render(r);
}


void
Qt4AggGlue::render(const QRect& updateRect)
{
    QPainter p(_drawing_area);
   
    p.drawImage(updateRect, *_image, updateRect);
    p.end();
}


Renderer*
Qt4AggGlue::createRenderHandler()
{
    _renderer = create_Renderer_agg("BGRA32");

    if ( ! _renderer )
    {
        throw GnashException(_("Could not create AGG renderer with pixelformat ABGR32"));
    }
    return _renderer;
}

void
Qt4AggGlue::resize(int width, int height)
{
    initBuffer(width, height);
}

// end of namespace gnash
}
