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
//

#ifndef __RAWFB_DEVICE_H__
#define __RAWFB_DEVICE_H__ 1

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <linux/vt.h>

#include "GnashDevice.h"

namespace gnash {

namespace renderer {

namespace rawfb {

#define CMAP_SIZE (256*2)

class RawFBDevice : public GnashDevice
{
  public:
    
    RawFBDevice();
    RawFBDevice(int);
    RawFBDevice(int argc, char *argv[]);
    
    // virtual classes should have virtual destructors
    virtual ~RawFBDevice();

    dtype_t getType() { return RAWFB; };

    // Initialize RAWFB Device layer
    bool initDevice(int argc, char *argv[]);

    // Initialize RAWFB Window layer
    bool attachWindow(GnashDevice::native_window_t window);
    
    // Utility methods not in the base class

    // Return a string with the error code as text, instead of a numeric value
    const char *getErrorString(int error);
    
    int getDepth() { return _varinfo.bits_per_pixel; };

    // Accessors for the settings needed by higher level code.
    // Surface accessors
    size_t getWidth() { return _varinfo.xres; };
    size_t getHeight() { return _varinfo.yres; };
    
    bool isSingleBuffered() { return true; }
    
    bool supportsRenderer(GnashDevice::rtype_t /* rtype */) { return false; }
    
    bool isBufferDestroyed() { return false; }
    // bool isBufferDestroyed(IRAWFBSurface surface) {
    //     return false;
    // }
    int getID() { return 0; };

    // Get the size of the pixels
    int getRedSize()   { return _varinfo.red.length; };
    int getGreenSize() { return _varinfo.green.length; };
    int getBlueSize()  { return _varinfo.blue.length; };

#ifdef RENDERER_AGG
    /// These methods are only needed by AGG, which uses these
    /// to calculate the pixel format.
    int getRedOffset()   { return _varinfo.red.offset; };
    int getGreenOffset() { return _varinfo.green.offset; };
    int getBlueOffset()  { return _varinfo.blue.offset; };    
#endif
    
    // Using RAWFB always means a native renderer
    bool isNativeRender() { return true; }

    native_window_t getDrawableWindow() { return 0; };
    
    //
    // Testing Support
    //
    
    // Create an RAWFB window to render in. This is only used by testing
    void createWindow(const char *name, int x, int y, int width, int height);

    // Get the memory from the real framebuffer
    boost::uint8_t *getFBMemory() { return _fbmem; };

    // // Get the memory from an offscreen buffer to support Double Buffering
    boost::uint8_t *getOffscreenBuffer() { return _offscreen_buffer.get(); };

    size_t getStride() { return _fixinfo.line_length; };
    size_t getFBMemSize() { return _fixinfo.smem_len; };
    int getHandle() { return _fd; };
    
    /// Start an RAWFB event loop. This is only used by testing. Note that
    /// calling this function blocks until the specified number of events
    /// have been handled. The first 5 are used up by creating the window.
    ///
    /// @param passes the number of events to process before returning.
    /// @return nothing
    void eventLoop(size_t passes);
    
    /// For 8 bit (palette / LUT) modes, sets a grayscale palette.
    /// This GUI currently does not support palette modes. 
    bool setGrayscaleLUT8();

    bool swapBuffers() {
        if (_fbmem && _offscreen_buffer) {
            std::copy(_fbmem, _fbmem + _fixinfo.smem_len,
                      _offscreen_buffer.get());
            return true;
        }
        return false;
    }
    
protected:
    /// Clear the framebuffer memory
    void clear();

    int                                 _fd;
    std::string                         _filespec;
    struct fb_fix_screeninfo            _fixinfo;
    struct fb_var_screeninfo            _varinfo;
    boost::uint8_t                     *_fbmem;
    
    boost::scoped_ptr<boost::uint8_t>   _offscreen_buffer;
    struct fb_cmap                      _cmap;       // the colormap
};

#ifdef ENABLE_FAKE_FRAMEBUFFER
/// Simulate the ioctls used to get information from the framebuffer driver.
///
/// Since this is an emulator, we have to set these fields to a reasonable default.
int fakefb_ioctl(int fd, int request, void *data);
#endif

typedef void (*init_func)();
typedef void (*reshape_func)(int, int);
typedef void (*draw_func)();
typedef int  (*key_func)(unsigned key);

} // namespace rawFB
} // namespace renderer
} // namespace gnash

#endif  // end of __RAWFB_DEVICE_H__

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
