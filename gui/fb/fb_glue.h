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

#ifndef GNASH_FB_GLUE_H
#define GNASH_FB_GLUE_H

#include <cassert>

namespace gnash {
    class Renderer;
    class movie_root;
}

namespace gnash {

namespace gui {
    
typedef void FbWidget;
  
class FBGlue
{
public:
    FBGlue();
    virtual ~FBGlue();
    
    virtual bool init(int argc, char **argv[]) = 0;
    
    virtual void prepDrawingArea(FbWidget *drawing_area) = 0;
    virtual Renderer* createRenderHandler() = 0;
    virtual void setRenderHandlerSize(int /*width*/, int /*height*/) {}
    virtual void render() = 0;
    
    virtual int width() = 0;
    virtual int height() = 0;
    
    virtual void render(void* const region);

    virtual void beforeRendering(movie_root *) {};

  protected:
    //    FbWidget *_drawing_area;
};

} // end of namespace gui
} // end of namespace gnash

// end of GNASH_FB_GLUE_H
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
