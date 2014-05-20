//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Free Software
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

#include "GnashDevice.h"
#include "DeviceGlue.h"

#ifdef BUILD_EGL_DEVICE
#include "egl/eglDevice.h"
#endif
#ifdef BUILD_RAWFB_DEVICE
#include "rawfb/RawFBDevice.h"
#endif
#ifdef BUILD_DIRECTFB_DEVICE
#include "directfb/DirectFBDevice.h"
#endif
#ifdef BUILD_X11_DEVICE
#include "x11/X11Device.h"
#endif

namespace gnash {
    class movie_root;
}

namespace gnash {

namespace gui {
    
typedef void FbWidget;

/// \class FBGlue
/// This class is the base class for the glue layer between the GUI toolkit
/// and the renderer.
class FBGlue : public DeviceGlue
{
public:
    FBGlue() {};
    virtual ~FBGlue() {};

    /// Initialize the glue layer. This also initializes the Renderer
    /// and display device.
    virtual bool init(int argc, char **argv[]) = 0;

    // Prepare the drawing area for the renderer
    virtual void prepDrawingArea(FbWidget *drawing_area) = 0;
    virtual Renderer* createRenderHandler() = 0;
    virtual void setRenderHandlerSize(int /*width*/, int /*height*/) {}
    virtual void setInvalidatedRegions(const InvalidatedRanges &/* ranges */) {};
    
    virtual void render() = 0;
    
    virtual int width() = 0;
    virtual int height() = 0;
    
    virtual void render(void* const /* region */) {};

    virtual void beforeRendering(movie_root *) {};

protected:
    std::unique_ptr<Renderer> _renderer;
};

} // end of namespace gui
} // end of namespace gnash

// end of GNASH_FB_GLUE_H
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
