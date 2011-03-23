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

#include "Qt4Gui.h"
#include "Qt4GlueCairo.h"
#include "Renderer.h"
#include "Renderer_cairo.h"
#include <QWidget>
#include <QRect>

namespace gnash
{

Qt4CairoGlue::Qt4CairoGlue()
:
  _width(0),
  _height(0),
  _offscreenbuf(0),
  _renderer(0),
  _cairo_handle(0),
  _cairo_surface(0)
{
}

Qt4CairoGlue::~Qt4CairoGlue()
{
    if (_cairo_surface)
        cairo_surface_destroy(_cairo_surface);
}

bool
Qt4CairoGlue::init(int /* argc */, char *** /* argv */)
{
    return true;
}

void
Qt4CairoGlue::initBuffer(int width, int height)
{
    if (! _drawing_area)
        return;

    _width = width;
    _height = height;

    cairo_format_t cairoFormat = CAIRO_FORMAT_ARGB32;
    QImage::Format qtFormat = QImage::Format_ARGB32;
    switch (_drawing_area->depth()) {
        case 24:
            cairoFormat = CAIRO_FORMAT_RGB24;
            qtFormat = QImage::Format_RGB32;
            break;
        case 32:
            cairoFormat = CAIRO_FORMAT_ARGB32;
            qtFormat = QImage::Format_ARGB32;
            break;
    }

    // Cairo uses 4 bits per pixel even for 24 bit color depth
    int bufsize = _width * _height * 4;
    _offscreenbuf.reset(new unsigned char[bufsize]);
    _image.reset(new QImage(_offscreenbuf.get(),
                            _width, _height, qtFormat));

    if (_cairo_surface)
        cairo_surface_destroy(_cairo_surface);
    if (_cairo_handle)
        cairo_destroy(_cairo_handle);

    _cairo_surface = cairo_image_surface_create_for_data(
                        _offscreenbuf.get(), cairoFormat, _width, _height,
                        cairo_format_stride_for_width(cairoFormat, _width));

    _cairo_handle = cairo_create(_cairo_surface);
    cairo_set_source_surface(_cairo_handle,
                             cairo_get_target(_cairo_handle), 0, 0);
    renderer::cairo::set_context(_renderer, _cairo_handle);
}

void
Qt4CairoGlue::prepDrawingArea(DrawingWidget *drawing_area)
{
    assert(drawing_area);
    _drawing_area = drawing_area;
}


void
Qt4CairoGlue::render()
{
    QRect r(0, 0, _width, _height);
    render(r);
}


void
Qt4CairoGlue::render(const QRect& updateRect)
{
    assert(_drawing_area);

    if (_cairo_handle) {

        cairo_paint(_cairo_handle);

        QPainter p(_drawing_area);

        p.drawImage(updateRect, *_image, updateRect);
        p.end();
    }
}

void
Qt4CairoGlue::resize(int width, int height)
{
    initBuffer(width, height);
}



Renderer*
Qt4CairoGlue::createRenderHandler()
{
    _renderer = renderer::cairo::create_handler();

    return _renderer;
}

// end of namespace gnash
}
