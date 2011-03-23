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

#ifndef FLTK_GLUE_AGG_H
#define FLTK_GLUE_AGG_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <fltk/Widget.h>

#include "log.h"
#include "gui.h"

#include "Renderer.h"
#include "Renderer_agg.h"

using namespace fltk;

namespace gnash {

class FltkAggGlue : public fltk::Widget
{
    public:
      FltkAggGlue(int x, int y, int width, int height);
      ~FltkAggGlue();
     // resize(int width, int height);
      void draw();
      Renderer* createRenderHandler();
      void initBuffer(int width, int height);
      void resize(int width, int height);
      void render(geometry::Range2d<int>& bounds);


    private:
      int _width;
      int _height;
      int _stride;
      unsigned char* _offscreenbuf;
      Renderer* _renderer;
      //Rectangle _bounds;
      geometry::Range2d<int> _drawbounds;
      geometry::Range2d<int> _validbounds;
};

} // namespace gnash

#endif //FLTK_GLUE_AGG_H
