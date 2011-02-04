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
# include <Qt/qwidget.h>
#else
# include <qwidget.h>
#endif
#include "snappingrange.h"

namespace gnash {

class Renderer; 
class KdeGlue
{
  public:
    KdeGlue() : _drawing_area(NULL) {}
    virtual ~KdeGlue() { }
    virtual bool init(int argc, char **argv[]) = 0;

    virtual void prepDrawingArea(QWidget *drawing_area) = 0;
    virtual Renderer* createRenderHandler() = 0;
    virtual void render() = 0;
    virtual void setInvalidatedRegions(const InvalidatedRanges& /* ranges */) {}
    virtual void resize(int, int) {}
    virtual void initBuffer(int, int) {}
  protected:
    QWidget     *_drawing_area;
};

} // namespace gnash
