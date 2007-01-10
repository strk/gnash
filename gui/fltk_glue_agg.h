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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
//
//

#ifndef FLTK_GLUE_AGG_H
#define FLTK_GLUE_AGG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
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
#include "gnash.h"
#include "log.h"
#include "gui.h"

#include "render_handler.h"
#include "render_handler_agg.h"

using namespace std;
using namespace fltk;

namespace gnash {

class FltkAggGlue
{
    public:
      FltkAggGlue();
      ~FltkAggGlue();
     // resize(int width, int height);
      void draw();
      render_handler* createRenderHandler();
      void initBuffer(int width, int height);
      void resize(int width, int height);
      void invalidateRegion(const rect& bounds);

    private:
      int _width;
      int _height;
      int _stride;
      unsigned char* _offscreenbuf;
      render_handler* _renderer;
      //Rectangle _bounds;
      geometry::Range2d<int> _drawbounds;
      geometry::Range2d<int> _validbounds;
};

} // namespace gnash

#endif //FLTK_GLUE_AGG_H
