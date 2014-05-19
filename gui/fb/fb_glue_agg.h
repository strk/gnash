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

#ifndef GNASH_FB_GLUE_AGG_H
#define GNASH_FB_GLUE_AGG_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/cstdint.hpp>

#include "Renderer.h"
#include "fbsup.h"
#include "fb_glue.h"

namespace gnash {

namespace gui {

class FBAggGlue: public FBGlue
{
public:
    FBAggGlue();

    // This constructor is not part of the API, as it's AGG and
    // Framebuffer specific
    FBAggGlue(int fd);

    // All of these virtuals are all defined in the base FBGlue class
    ~FBAggGlue();

    /// \brief
    ///  Initialise the Framebuffer GUI and the AGG renderer.
    /// 
    /// @param argc The commandline argument count.
    /// @param argv The commandline arguments.
    /// @return True on success; false on failure.
    bool init(int argc, char ***argv);

    /// Create the renderer handler
    Renderer *createRenderHandler();

    /// \brief
    ///  Hand off a handle to the native drawing area to the renderer
    void prepDrawingArea(void *drawing_area);
    
    /// Gives the GUI a *hint* which region of the stage should be redrawn.
    //
    /// There is *no* restriction what the GUI might do with these coordinates. 
    /// Normally the GUI forwards the information to the renderer so that
    /// it avoids rendering regions that did not change anyway. The GUI can
    /// also alter the bounds before passing them to the renderer and it's
    /// absolutely legal for the GUI to simply ignore the call.
    ///
    /// Coordinates are in TWIPS!
    ///
    /// Note this information is given to the GUI and not directly to the 
    /// renderer because both of them need to support this feature for 
    /// correct results. It is up to the GUI to forward this information to
    /// the renderer.
    ///
    // does not need to be implemented (optional feature),
    // but still needs to be available.
    //
    void setInvalidatedRegion(const SWFRect& bounds);
    void setInvalidatedRegions(const InvalidatedRanges &ranges);

    /// \brief
    ///  The Width of the drawing area, in pixels. For framebuffer
    ///  based devices, this is the size of the display screen.
    int width();
    
    /// Height of the drawing area, in pixels. For framebuffer
    ///  based devices, this is the size of the display screen.
    int height();

    /// Render the current buffer.    
    void render();
    void render(void* const /* region */) { };

    size_t getWidth()  { return (_device) ? _device->getWidth() : 0; };
    size_t getHeight() { return (_device) ? _device->getWidth() : 0; };
    size_t getDepth()  { return (_device) ? _device->getDepth() : 0; };
    
protected:
    /// This is the file descriptor for the framebuffer memory
    int                                 _fd;
    struct fb_fix_screeninfo            _fixinfo;
    struct fb_var_screeninfo            _varinfo;
    
    boost::scoped_ptr<Renderer>         _renderer;

    geometry::Range2d<int>              _validbounds;
    std::vector< geometry::Range2d<int> > _drawbounds;    
};

} // end of namespace gui
} // end of gnash namespace

#endif  // GNASH_FB_GLUE_AGG_H

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
