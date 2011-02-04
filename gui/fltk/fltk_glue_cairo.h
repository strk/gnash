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

#ifndef FLTK_GLUE_CAIRO_H
#define FLTK_GLUE_CAIRO_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <fltk/Item.h>
#include <fltk/ItemGroup.h>
#include <fltk/PopupMenu.h>
#include <fltk/Widget.h>
#include <fltk/ask.h>
#include <fltk/events.h>
#include <fltk/run.h>
#include <fltk/visual.h>
#include <fltk/Window.h>
#include <fltk/draw.h>
#include <fltk/x.h>
#include <fltk/damage.h>
#include <fltk/layout.h>
#include <fltk/Cursor.h>




#include "fltksup.h"
#include <cairo.h>

using namespace fltk;

namespace gnash {

class FltkCairoGlue
{
    public:
      FltkCairoGlue();
      ~FltkCairoGlue();
     // resize(int width, int height);
      void draw();
      Renderer* createRenderHandler();
      void initBuffer(int width, int height);
      void resize(int width, int height);
      void invalidateRegion(const SWFRect& bounds);
    private:
      int _width;
      int _height;
      int _stride;
      unsigned char* _offscreenbuf;
      Renderer* _renderer;
      cairo_surface_t *_cairo_surface;
      cairo_t         *_cairo_handle;
};

} // namespace gnash

#endif //FLTK_GLUE_CAIRO_H
