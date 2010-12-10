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

#ifndef GNASH_FB_GLUE_AGG_H
#define GNASH_FB_GLUE_AGG_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>

#include "Renderer.h"
#include "fbsup.h"
#include "fb_glue.h"

namespace gnash {

namespace gui {

class FBAggGlue: public FBGlue
{
public:
    FBAggGlue();

    // All of these virtuals are all defined in the base FBGlue class
    ~FBAggGlue();

    /// \brief
    ///  Initialise the Framebuffer GUI and the AGG renderer.
    /// 
    /// @param argc The commandline argument count.
    /// @param argv The commandline arguments.
    /// @return True on success; false on failure.
    bool init(int argc, char ***argv);    
    
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

    /// For 8 bit (palette / LUT) modes, sets a grayscale palette.
    //
    /// This GUI currently does not support palette modes.
    bool set_grayscale_lut8();

protected:
    /// This is the file descriptor for the framebuffer memory
    int                      _fd;
    struct fb_var_screeninfo _var_screeninfo;
    struct fb_fix_screeninfo _fix_screeninfo;
    struct fb_cmap           _cmap;
    
    boost::shared_ptr<boost::uint8_t> _fbmem;  // framebuffer memory
#ifdef ENABLE_DOUBLE_BUFFERING
    boost::shared_ptr<boost::uint8_t> _buffer; // offscreen buffer
#endif
    boost::uint32_t             _rowsize;
    std::vector< geometry::Range2d<int> > _drawbounds;

    boost::shared_ptr<Renderer> _renderer;
};

} // end of namespace gui
} // end of gnash namespace

#endif  // GNASH_FB_GLUE_AGG_H

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
