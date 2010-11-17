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

// #ifdef HAVE_X11_X_H
# include <X11/Xlib.h>
#include <X11/Xutil.h>
// #else
// # error "This file needs X11"
// #endif

namespace gnash
{

namespace renderer {

namespace x11 {

class X11Device
{
  public:
    X11Device();
    ~X11Device();

    // Initialize X11 Device layer
    bool initDevice(int argc, char *argv[]);

    // Initialize X11 Window layer
    //    bool initX11(X11NativeWindowType window);
    
    // Utility methods not in the base class
    // Return a string with the error code as text, instead of a numeric value
    const char *getErrorString(int error);
    
    // Accessors for the settings needed by higher level code.
    // Surface accessors
    size_t getWidth() {
        if (_screen) {
            return XWidthOfScreen(_screen);
        }
        return 0;
    }
    
    size_t getWidth(Screen *screen) {
        if (screen) {
            return XWidthOfScreen(screen);
        }
        return 0;
    };
    size_t getHeight() {
        if (_screen) {
            return XWidthOfScreen(_screen);
        }
        return 0;
    }
    
    size_t getHeight(Screen *screen) {
        if (screen) {
            return XHeightOfScreen(screen);
        }
    }
    size_t getVerticalRes() {
        return getHeight();
    }
    size_t getVerticalRes(Screen *screen) {
        return getHeight(screen);
    }
    size_t getHorzRes() {
        return getWidth();
    }
    size_t getHorzRes(Screen *screen) {
        return getWidth(screen);
    }

    bool isSurfaceSingleBuffered() {
        return true;
    }
    
    bool isSurfaceBackBuffered() {
        return false;
    }
    bool isBufferDestroyed() {
        return false;
    }
    // bool isBufferDestroyed(IX11Surface surface) {
    //     return false;
    // }
    bool isMultiSample() {
        return false;
    }
    int getSurfaceID() {
        return 0;
    }

    // Context accessors
    int getContextID() {
        return 0;
    }

    bool isContextSingleBuffered() { 
        return true;
    }
    bool isContextBackBuffered() {
        return false;
    }

    int getDepth() {
        return DefaultDepth(_display, _screennum);
    }

    bool isNativeRender() {
        return 0;
    }
    // Sample Buffers are only used by OpenGLES2
    int getSamples() {
        return 0;
    }
    int getSampleBuffers() {
        return 0;
    }
    int getMaxSwapInterval() {
        return 0;
    }
    int getMinSwapInterval() {
        return 0;
    }

protected:
    Display    *_display;
    int         _screennum;
    Window     *_window;
    Colormap    _colormap;
    Visual     *_visual;
    Screen     *_screen;
    int         _depth;
    
};

} // namespace x11
} // namespace renderer
} // namespace gnash

#endif  // end of __X11_DEVICE_H__

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
