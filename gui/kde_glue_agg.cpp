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

#if GNASH_QT_VERSION == 4
#include <Qt/qpixmap.h>
#include <Qt/qcolor.h>
#include <Qt/qicon.h>
#include <Qt/Qt3Support>
#else
#include <qpixmap.h>
#include <qcolor.h>
#endif
#include "kde_glue_agg.h"
#include "Renderer.h"
#include "Renderer_agg.h"
#include "GnashException.h"

namespace gnash
{

KdeAggGlue::KdeAggGlue()
:
  _width(0),
  _height(0),
   _renderer(0)
{
}

KdeAggGlue::~KdeAggGlue()
{
}

bool
KdeAggGlue::init(int /* argc */, char *** /* argv */)
{
//    GNASH_REPORT_FUNCTION;

    return true;
}


void
KdeAggGlue::prepDrawingArea(QWidget *drawing_area)
{
//    GNASH_REPORT_FUNCTION;
    _drawing_area = drawing_area;
}


void
KdeAggGlue::initBuffer(int width, int height)
{
    if (!_renderer) return;

    int _bpp = 32;
    int depth_bytes = _bpp / 8;  // TODO: <Udo> is this correct? Gives 1 for 15 bit modes!

    assert(_bpp % 8 == 0);

#define CHUNK_SIZE (100 * 100 * depth_bytes)

    int bufsize = (width * height * depth_bytes / CHUNK_SIZE + 1) * CHUNK_SIZE;

    _offscreenbuf.reset(new unsigned char[bufsize]);

    // Only the AGG renderer has the function init_buffer, which is *not* part of
    // the renderer api. It allows us to change the renderers movie size (and buffer
    // address) during run-time.
    Renderer_agg_base * renderer =
      static_cast<Renderer_agg_base *>(_renderer);
    renderer->init_buffer(_offscreenbuf.get(), bufsize, width, height,
      width*((_bpp+7)/8));

    _width = width;
    _height = height;

    _validbounds.setTo(0, 0, _width, _height);
    _drawbounds.push_back(_validbounds);
    
    _qimage.reset(new QImage(_offscreenbuf.get(), _width, _height, 32 /* bits per pixel */,
                             0 , 0, QImage::IgnoreEndian));
}

void
KdeAggGlue::render()
{
    // In order to use our buffer in QT, we must copy it into a pixmap. This is
    // an expensive operation, but, as far as I can see, the only way to do it.
    QPixmap qpixmap(*_qimage);

    for (unsigned bno=0; bno < _drawbounds.size(); bno++) {
       geometry::Range2d<int>& bounds = _drawbounds[bno];

       assert ( bounds.isFinite() );

       QPoint dest_point(bounds.getMinX(), bounds.getMinY()) ;
       QRect src_rect(bounds.getMinX(), bounds.getMinY(), bounds.width(),
                      bounds.height());
       
       bitBlt (_drawing_area, dest_point, &qpixmap, src_rect, Qt::CopyROP,
               true /* ignore mask */ );
    }
}

void
KdeAggGlue::setInvalidatedRegions(const InvalidatedRanges& ranges)
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


Renderer*
KdeAggGlue::createRenderHandler()
{
    // QT requires the use of this pixel format...
    _renderer = create_Renderer_agg("BGRA32");
    if (! _renderer) {
        throw GnashException(_("Could not create AGG renderer with pixelformat BGRA32"));
    }
    return _renderer;
}

void
KdeAggGlue::resize(int width, int height)
{
    initBuffer(width, height);
}

// end of namespace gnash
}
