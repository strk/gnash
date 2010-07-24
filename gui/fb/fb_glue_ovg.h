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

#ifndef FB_GLUE_OVG_H
#define FB_GLUE_OVG_H 1

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/cstdint.hpp>

#include "fbsup.h"

#ifdef HAVE_VG_OPENVG_H
#include <VG/openvg.h>
#endif

namespace gnash {

class render_handler;

class FBOvgGlue : public FBGlue
{
public:
    FBOvgGlue(int fd);
    FBOvgGlue(int x, int y, int width, int height);
    ~FBOvgGlue();
    
    virtual bool init(int argc, char ***argv);
    virtual void render();
    
    // resize(int width, int height);
    void draw();
    // Renderer* createRenderHandler();
    void initBuffer(int width, int height);
    void resize(int width, int height);
    // void render(geometry::Range2d<int>& bounds);
    
private:
    int         _width;
    int         _height;
    int         _stride;
    boost::uint8_t *_offscreenbuf;
    // Renderer *_renderer;
    
    //Rectangle _bounds;
    geometry::Range2d<int> _drawbounds;
    geometry::Range2d<int> _validbounds;
};
    
} // end of namespace gnash

#endif  // end of FB_GLUE_OVG_H

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
