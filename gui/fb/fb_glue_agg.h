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

#include "fbsup.h"

namespace gnash {

class Renderer;

class FBAggGlue: public FBGlue
{
public:
    FBAggGlue(int fd);

    // All of these virtuals are all defined in the base FBGlue class
    virtual ~FBAggGlue();
    virtual bool init(int argc, char ***argv);    
    virtual Renderer *createRenderHandler();
    virtual void setInvalidatedRegions(const InvalidatedRanges &ranges);
    virtual int width();
    virtual int height();
    virtual void render();

    /// For 8 bit (palette / LUT) modes, sets a grayscale palette.
    //
    /// This GUI currently does not support palette modes. 
    //
    bool set_grayscale_lut8();

protected:
    int                 _fd;
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

} // end of gnash namespace

#endif  // GNASH_FB_GLUE_AGG_H

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
