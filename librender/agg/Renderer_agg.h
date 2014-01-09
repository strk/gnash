// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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


#ifndef BACKEND_RENDER_HANDLER_AGG_H
#define BACKEND_RENDER_HANDLER_AGG_H

#include "dsodefs.h"
#include "Renderer.h"

namespace gnash {

// Base class to shield GUIs from AGG's pixelformat classes 
class Renderer_agg_base : public Renderer
{
private:
    
    unsigned char *_testBuffer; // used by initTestBuffer() for testing
    
public:
    
    Renderer_agg_base() : _testBuffer(0) { }  
    
    // virtual classes should have virtual destructors
    virtual ~Renderer_agg_base() {
        if ( _testBuffer ) free(_testBuffer);
    }
    
    // these methods need to be accessed from outside:
    virtual void init_buffer(unsigned char *mem, int size, int x, int y,
                             int rowstride) = 0;
    
    virtual unsigned int getBytesPerPixel() const = 0;
    
    unsigned int getBitsPerPixel() const { return getBytesPerPixel()*8; }
    
    virtual bool initTestBuffer(unsigned width, unsigned height) {
        int size = width * height * getBytesPerPixel();
        
        unsigned char *tmp = static_cast<unsigned char *>(realloc(_testBuffer, size));
        if (tmp == NULL) {
            log_error(_("Memory reallocation error"));
            return false;
        } else {
            _testBuffer = tmp;
        }

        memset(_testBuffer, 0, size);
        printf("Renderer Test memory at: %p\n", _testBuffer);
        
        init_buffer(_testBuffer, size, width, height, width * getBytesPerPixel());
        
        return true;
    }
};

/// Create a render handler 
//
/// If the given pixelformat is unsupported, or any other error
/// occurs, NULL is returned.
///
DSOEXPORT Renderer_agg_base *create_Renderer_agg(const char *pixelformat);
  
/// Detect pixel format based on bit mask. If the pixel format is unknown,
/// NULL is returned. Note that a successfully detected pixel format does
/// not necessarily mean that the pixel format is available (compiled in).
/// The bit offsets are assumed to be in host byte order!
DSOEXPORT const char *agg_detect_pixel_format(unsigned int rofs,
                                              unsigned int rsize,
                                              unsigned int gofs,
                                              unsigned int gsize,
                                              unsigned int bofs,
                                              unsigned int bsize,
                                              unsigned int bpp);
  

} // namespace gnash

#endif // BACKEND_RENDER_HANDLER_AGG_H

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
