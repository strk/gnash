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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <cstring>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>

#include "Renderer.h"
#include "Renderer_agg.h"
#include "fbsup.h"
#include "fb_glue_agg.h"
#include "RunResources.h"
#include "log.h"

namespace gnash {

FBAggGlue::FBAggGlue(int fd)
    : _fd(fd),
      _rowsize(0)
{
    GNASH_REPORT_FUNCTION;

    memset(&_var_screeninfo, 0, sizeof(struct fb_var_screeninfo));
    memset(&_fix_screeninfo, 0, sizeof(struct fb_fix_screeninfo));
}

FBAggGlue::~FBAggGlue()
{
    GNASH_REPORT_FUNCTION;    

    // Close the memory
    if (_fd) {
        ::close(_fd);
    }
}

void
FBAggGlue::setInvalidatedRegions(const InvalidatedRanges &ranges)
{
    GNASH_REPORT_FUNCTION;

}

bool
FBAggGlue::init (int argc, char ***argv)
{
    GNASH_REPORT_FUNCTION;

    // Load framebuffer properties
#ifdef ENABLE_FAKE_FRAMEBUFFER
    fakefb_ioctl(_fd, FBIOGET_VSCREENINFO, &_var_screeninfo);
    fakefb_ioctl(_fd, FBIOGET_FSCREENINFO, &_fix_screeninfo);
#else  // ENABLE_FAKE_FRAMEBUFFER
    ioctl(_fd, FBIOGET_VSCREENINFO, &_var_screeninfo);
    ioctl(_fd, FBIOGET_FSCREENINFO, &_fix_screeninfo);
#endif
    
    log_debug(_("Framebuffer device uses %d bytes of memory."),
	      _fix_screeninfo.smem_len);
    log_debug(_("Video mode: %dx%d with %d bits per pixel."),
	      _var_screeninfo.xres, _var_screeninfo.yres,
              _var_screeninfo.bits_per_pixel);
    
    // map framebuffer into memory
    _fbmem.reset(static_cast<boost::uint8_t *>(mmap(0, _fix_screeninfo.smem_len,
                                          PROT_READ|PROT_WRITE, MAP_SHARED,
                                          _fd, 0)));
    
#ifdef ENABLE_DOUBLE_BUFFERING
    // allocate offscreen buffer
    _buffer.reset(new boost::uint8_t[_fix_screeninfo.smem_len]);
    memset(_buffer.get(), 0, _fix_screeninfo.smem_len);
#endif  
    
#ifdef PIXELFORMAT_LUT8
    // Set grayscale for 8 bit modes
    if (_var_screeninfo.bits_per_pixel == 8) {
	if (!set_grayscale_lut8())
	    return false;
    }
#endif

    return true;
}

#define TO_16BIT(x) (x | (x<<8))

bool
FBAggGlue::set_grayscale_lut8()
{
    GNASH_REPORT_FUNCTION;
    
    struct fb_cmap cmap;
    int i;
    
    log_debug(_("LUT8: Setting up colormap"));
    
    cmap.start = 0;
    cmap.len = 256;
    cmap.red = new boost::uint16_t[CMAP_SIZE];
    cmap.green = new boost::uint16_t[CMAP_SIZE];
    cmap.blue = new boost::uint16_t[CMAP_SIZE];
    cmap.transp = NULL;
    
    for (i=0; i<256; i++) {
	
	int r = i;
	int g = i;
	int b = i;
	
	cmap.red[i] = TO_16BIT(r);
	cmap.green[i] = TO_16BIT(g);
	cmap.blue[i] = TO_16BIT(b);
	
    }
    
#ifdef ENABLE_FAKE_FRAMEBUFFER
    if (fakefb_ioctl(_fd, FBIOPUTCMAP, &cmap))
#else
    if (ioctl(_fd, FBIOPUTCMAP, &cmap))
#endif
    {
	log_error(_("LUT8: Error setting colormap: %s"), strerror(errno));
	return false;
    }
    
    return true;
     
#undef TO_16BIT
}    

Renderer *
FBAggGlue::createRenderHandler()
{
    GNASH_REPORT_FUNCTION;

    const int width     = _var_screeninfo.xres;
    const int height    = _var_screeninfo.yres;
    const int bpp       = _var_screeninfo.bits_per_pixel;
    const int size      = _fix_screeninfo.smem_len; 
    
    // TODO: should recalculate!  
    boost::uint8_t       *mem;
    Renderer_agg_base   *agg_handler;

    agg_handler = NULL;

    // _validbounds.setTo(0, 0, width - 1, height - 1);
    
#ifdef ENABLE_DOUBLE_BUFFERING
    log_debug(_("Double buffering enabled"));
    mem = _buffer;
#else
    log_debug(_("Double buffering disabled"));
    mem = _fbmem.get();
#endif
    
    agg_handler = NULL;
  
    // choose apropriate pixel format
    
    log_debug(_("red channel: %d / %d"), _var_screeninfo.red.offset, 
	      _var_screeninfo.red.length);
    log_debug(_("green channel: %d / %d"), _var_screeninfo.green.offset, 
	      _var_screeninfo.green.length);
    log_debug(_("blue channel: %d / %d"), _var_screeninfo.blue.offset, 
	      _var_screeninfo.blue.length);
    log_debug(_("Total bits per pixel: %d"), _var_screeninfo.bits_per_pixel);
    
    const char* pixelformat = agg_detect_pixel_format(
        _var_screeninfo.red.offset, _var_screeninfo.red.length,
        _var_screeninfo.green.offset, _var_screeninfo.green.length,
        _var_screeninfo.blue.offset, _var_screeninfo.blue.length,
        bpp
        );
    
    if (pixelformat) {    
	agg_handler = create_Renderer_agg(pixelformat);      
    } else {
	log_error("The pixel format of your framebuffer could not be detected.");
	return false;
    }
    
    assert(agg_handler != NULL);

    _rowsize = _var_screeninfo.xres_virtual*((bpp+7)/8);
    
    agg_handler->init_buffer(mem, size, width, height, _rowsize);
    
    return (Renderer *)agg_handler;
}    

void
FBAggGlue::render()
{
    // GNASH_REPORT_FUNCTION;

    if ( _drawbounds.size() == 0 ) {
        return; // nothing to do..
    }
    
#ifdef ENABLE_DOUBLE_BUFFERING
    // Size of a pixel in bytes
    // NOTE: +7 to support 15 bpp
    const unsigned int pixel_size = (var_screeninfo.bits_per_pixel+7)/8;
    
    for (unsigned int bno=0; bno < _drawbounds.size(); bno++) {
	geometry::Range2d<int>& bounds = _drawbounds[bno];
        
	assert ( ! bounds.isWorld() );
	
	// Size, in bytes, of a row that has to be copied
	const unsigned int row_size = (bounds.width()+1) * pixel_size;
	
	// copy each row
	const int minx = bounds.getMinX();
	const int maxy = bounds.getMaxY();
	
	for (int y=bounds.getMinY(); y<=maxy; ++y) {    
	    const unsigned int pixel_index = y * _rowsize + minx*pixel_size;
	    memcpy(&fbmem[pixel_index], &buffer[pixel_index], row_size);
	    
	}
    }  
    
#endif
    
#ifdef DEBUG_SHOW_FPS
    profile();
#endif
}    

int
FBAggGlue::width()
{
    return _var_screeninfo.xres_virtual;
}

int
FBAggGlue::height()
{
    return _var_screeninfo.yres_virtual;
}

} // end of namespace gnash
    
// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
