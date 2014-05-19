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

#if GNASH_QT_VERSION == 4
# include <QtGui/QImage>
#else
# include <qimage.h>
#endif

#include "kde_glue.h"
#include <vector>
#include <memory>
#include <boost/scoped_array.hpp>


namespace gnash
{

class KdeAggGlue : public KdeGlue
{
  public:
    KdeAggGlue();
    ~KdeAggGlue();
    
    bool init(int argc, char **argv[]);
    void prepDrawingArea(QWidget *drawing_area);
    Renderer* createRenderHandler();
    void initBuffer(int width, int height);
    void resize(int width, int height);
    void render();
    void setInvalidatedRegions(const InvalidatedRanges& ranges);

  private:
    int _width;
    int _height;
    boost::scoped_array<unsigned char> _offscreenbuf;
    Renderer* _renderer; // We don't own this pointer.
    geometry::Range2d<int> _validbounds;
    std::vector< geometry::Range2d<int> > _drawbounds;
    std::unique_ptr<QImage> _qimage;
};




}
