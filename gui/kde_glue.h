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


/* $Id: kde_glue.h,v 1.6 2006/10/29 18:34:10 rsavoye Exp $ */

#include "gnash.h"

#include <qwidget.h>

namespace gnash
{

class KdeGlue
{
  public:
    virtual ~KdeGlue() { delete _drawing_area; }
    virtual bool init(int argc, char **argv[]) = 0;

    virtual void prepDrawingArea(QWidget *drawing_area) = 0;
    virtual render_handler* createRenderHandler() = 0;
    virtual void render() = 0;
  protected:
    QWidget     *_drawing_area;
};

} // namespace gnash
