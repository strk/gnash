//
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

//
//


/* $Id: kde_glue.h,v 1.13 2007/07/01 10:54:03 bjacques Exp $ */

#include "gnash.h"

#include <qwidget.h>
#include "snappingrange.h"

namespace gnash
{

class KdeGlue
{
  public:
    KdeGlue() : _drawing_area(NULL) {}
    virtual ~KdeGlue() { }
    virtual bool init(int argc, char **argv[]) = 0;

    virtual void prepDrawingArea(QWidget *drawing_area) = 0;
    virtual render_handler* createRenderHandler() = 0;
    virtual void render() = 0;
    virtual void setInvalidatedRegions(const InvalidatedRanges& /* ranges */) {}
    virtual void resize(int, int) {}
    virtual void initBuffer(int, int) {}
  protected:
    QWidget     *_drawing_area;
};

} // namespace gnash
