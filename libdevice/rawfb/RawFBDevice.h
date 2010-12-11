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

#include "GnashDevice.h"

namespace gnash {

namespace renderer {

namespace rawfb {

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
    
    int getDepth() {GNASH_REPORT_FUNCTION; };

    // Accessors for the settings needed by higher level code.
    // Surface accessors
    size_t getWidth() {GNASH_REPORT_FUNCTION; };
    size_t getHeight() {GNASH_REPORT_FUNCTION; };
    
    bool isSingleBuffered() { return true; }
    
    bool supportsRenderer(GnashDevice::rtype_t /* rtype */) { return false; }
    
    bool isBufferDestroyed() { return false; }
    // bool isBufferDestroyed(IRAWFBSurface surface) {
    //     return false;
    // }
    int getID() {GNASH_REPORT_FUNCTION; };

    // Get the size of the pixels, for RAWFB it's always 8 as far as I can tell
    int getRedSize() {GNASH_REPORT_FUNCTION; };
    int getGreenSize(){GNASH_REPORT_FUNCTION; };
    int getBlueSize() {GNASH_REPORT_FUNCTION; };
    
    // Using RAWFB always means a native renderer
    bool isNativeRender() { return true; }

    native_window_t getDrawableWindow() {GNASH_REPORT_FUNCTION; };
    
    //
    // Testing Support
    //
    
    // Create an RAWFB window to render in. This is only used by testing
    void createWindow(const char *name, int x, int y, int width, int height);

    /// Start an RAWFB event loop. This is only used by testing. Note that
    /// calling this function blocks until the specified number of events
    /// have been handled. The first 5 are used up by creating the window.
    ///
    /// @param passes the number of events to process before returning.
    /// @return nothing
    void eventLoop(size_t passes);

protected:
    std::string _filespec;
};

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
