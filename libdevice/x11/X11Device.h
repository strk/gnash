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

#ifndef __X11_DEVICE_H__
#define __X11_DEVICE_H__ 1

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>

#ifdef HAVE_X11_X_H
# include <X11/X.h>
# include <X11/Xlib.h>
# include <X11/Xutil.h>
#else
# error "This file needs X11"
#endif

#include "GnashDevice.h"

namespace gnash {

namespace renderer {

namespace x11 {

class X11Device : public GnashDevice
{
  public:
    
    X11Device();
    X11Device(int);
    X11Device(int argc, char *argv[]);
    
    // virtual classes should have virtual destructors
    virtual ~X11Device();

    dtype_t getType() { return X11; };

    // Initialize X11 Device layer
    bool initDevice(int argc, char *argv[]);

    // Initialize X11 Window layer
    bool attachWindow(GnashDevice::native_window_t window);
    
    // Utility methods not in the base class

    // Return a string with the error code as text, instead of a numeric value
    const char *getErrorString(int error);
    
    int getDepth() { return DefaultDepth(_display, _screennum); }

    // Accessors for the settings needed by higher level code.
    // Surface accessors
    size_t getWidth()  { if (_screen) { return XWidthOfScreen(_screen); } return 0; }
    size_t getHeight() { if (_screen) { return XWidthOfScreen(_screen); } return 0; }
    
    bool isSingleBuffered() { return true; }
    
    bool supportsRenderer(GnashDevice::rtype_t /* rtype */) { return false; }
    
    bool isBufferDestroyed() { return false; }
    // bool isBufferDestroyed(IX11Surface surface) {
    //     return false;
    // }
    int getID() { return static_cast<int>(_window); }

    // Get the size of the pixels, for X11 it's always 8 as far as I can tell
    int getRedSize() { return (_vinfo) ? _vinfo[0].bits_per_rgb : 0; };
    int getGreenSize() { return getRedSize(); };
    int getBlueSize() { return getRedSize(); };
    
    // Using X11 always means a native renderer
    bool isNativeRender() { return true; }

    int getHandle() { return _window; };
    
    //
    // Testing Support
    //
    
    // Create an X11 window to render in. This is only used by testing
    void createWindow(const char *name, int x, int y, int width, int height);

    /// Start an X11 event loop. This is only used by testing. Note that
    /// calling this function blocks until the specified number of events
    /// have been handled. The first 5 are used up by creating the window.
    ///
    /// @param passes the number of events to process before returning.
    /// @return nothing
    void eventLoop(size_t passes);

protected:
    Display    *_display;
    int         _screennum;
    Window      _root;
    Window      _window;
    Colormap    _colormap;
    Visual     *_visual;
    Screen     *_screen;
    int         _depth;
    XVisualInfo *_vinfo;
    int         _vid;
};

typedef void (*init_func)();
typedef void (*reshape_func)(int, int);
typedef void (*draw_func)();
typedef int  (*key_func)(unsigned key);

} // namespace x11
} // namespace renderer
} // namespace gnash

#endif  // end of __X11_DEVICE_H__

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
