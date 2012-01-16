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
//

#ifndef __EGL_DEVICE_H__
#define __EGL_DEVICE_H__ 1

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>

#ifdef HAVE_X11_X_H
#include "x11/X11Device.h"
#endif
#include "GnashDevice.h"

#ifdef HAVE_EGL_EGL_H
# include <EGL/egl.h>
#else
# error "This file needs EGL, which is part of OpenGL-ES"
#endif

#include "Point2d.h"

namespace gnash {

namespace renderer {

struct eglVertex {
    eglVertex(float x, float y)
        : _x(x), _y(y) { }
  
    eglVertex(const point& p)
        : _x(p.x), _y(p.y) { }
    float _x;
    float _y;
};

class EGLDevice : public GnashDevice
{
  public:
    typedef enum {LOW, MEDIUM, HIGH} quality_e;
    EGLDevice();
    EGLDevice(int argc, char *argv[]);
    EGLDevice(GnashDevice::rtype_t rtype);

    virtual ~EGLDevice();

    dtype_t getType() { return EGL; };

    // Initialize EGL Device layer
    bool initDevice(int argc, char *argv[]);

    // Initialize EGL Window layer
    bool attachWindow(GnashDevice::native_window_t window);
    
    // Utility methods not in the base class
    /// Return a string with the error code as text, instead of a numeric value
    const char *getErrorString(int error);

    size_t getStride() {
        return getDepth() * getWidth();
    };

    // Accessors for the settings needed by higher level code.
    // Surface accessors
    size_t getWidth() {
        return getWidth(_eglSurface);
    };
    
    size_t getHeight() {
        return getHeight(_eglSurface);
    }
    
    EGLint getDepth() {
        EGLint value;
        eglGetConfigAttrib(_eglDisplay, _eglConfig, EGL_DEPTH_SIZE, &value);
        return value;
    }

    int getRedSize() {
        EGLint value;
        if (_eglConfig && _eglDisplay) {
            eglGetConfigAttrib(_eglDisplay, _eglConfig, EGL_RED_SIZE, &value);
        }
        return static_cast<int>(value);
    };
    int getGreenSize() {
        EGLint value;
        if (_eglConfig && _eglDisplay) {
            eglGetConfigAttrib(_eglDisplay, _eglConfig, EGL_GREEN_SIZE, &value);
        }
        return static_cast<int>(value);
    };
    int getBlueSize() {
        EGLint value;
        if (_eglConfig && _eglDisplay) {
            eglGetConfigAttrib(_eglDisplay, _eglConfig, EGL_BLUE_SIZE, &value);
        }
        return static_cast<int>(value);
    };
    
    bool isSingleBuffered() {
        EGLint value;
        if (_eglSurface && _eglDisplay) {
            eglQuerySurface(_eglDisplay, _eglSurface, EGL_RENDER_BUFFER, &value);
        }
        if (value == EGL_SINGLE_BUFFER) {
            return true;
        }
        return false;
    }
    bool isBufferDestroyed() {
        return isBufferDestroyed(_eglSurface);
    }
    
    int getID() {
        return static_cast<int>(getSurfaceID());
    }
    
    bool supportsRenderer(GnashDevice::rtype_t rtype);
    
    bool isNativeRender() {
        EGLint value;
        eglGetConfigAttrib(_eglDisplay, _eglConfig, EGL_NATIVE_RENDERABLE, &value);
        return value;
    }

    // Overload some of the base class methods to deal with Device specific
    // data types.
    bool bindClient(GnashDevice::rtype_t rtype);
    
    size_t getWidth(EGLSurface surface) {
        EGLint value;
        if (surface && _eglDisplay) {
            eglQuerySurface(_eglDisplay, surface, EGL_WIDTH, &value);
        }
        return static_cast<size_t>(value);
    };
    size_t getHeight(EGLSurface surface) {
        EGLint value;
        if (surface && _eglDisplay) {
            eglQuerySurface(_eglDisplay, surface, EGL_HEIGHT, &value);
        }
        return static_cast<size_t>(value);
    }
    bool isBufferDestroyed(EGLSurface surface) {
        EGLint value;
        eglQuerySurface(_eglDisplay, surface, EGL_SWAP_BEHAVIOR, &value);
        if (value == EGL_BUFFER_DESTROYED) {
            return true;
        }
        return false;
    }
#ifdef BUILD_X11_DEVICE
    EGLint getNativeVisual();
#endif
    
    /// Check the requested EGl configuration against the current one
    bool checkEGLConfig(EGLConfig config);
    
    /// Query the system for all supported configs
    int queryEGLConfig() { return queryEGLConfig(_eglDisplay); };
    int queryEGLConfig(EGLDisplay display);

    // Debugging utilities
    void printEGLConfig() { return printEGLConfig(_eglConfig); };
    void printEGLConfig(EGLConfig config);
    void printEGLContext() { return printEGLContext(_eglContext); };
    void printEGLContext(EGLContext context);
    void printEGLSurface() { return printEGLSurface(_eglSurface); };
    void printEGLSurface(EGLSurface surface);
    void printEGLAttribs(const EGLint *attrib);
    
    // Create Pbuffers for offscreen rendering
    EGLSurface createPbuffer(int width, int height);
    EGLSurface createPbuffer(int width, int height, EGLClientBuffer buf, EGLenum type);
    EGLSurface createPixmap(int width, int height, NativePixmapType buf);
    size_t totalPbuffers() { return _pbuffers.size(); };
    EGLSurface &operator[](int index) { if (!_pbuffers.empty()) { return _pbuffers[index]; }; };

    // Swapping Buffers makes the specified surface active on the display if
    // EGL_RENDER_BUFFER is set to EGL_BACK_BUFFER. If it's set to
    // EGL_SINGLE_BUFFER then this has no effect, as the display was drawn to
    // directly.
    // Swap to the default surface
    bool swapBuffers() {
        // GNASH_REPORT_FUNCTION;
        if (!isSingleBuffered()) {
            return eglSwapBuffers(_eglDisplay, _eglSurface);
        }
        return true;
    }
    bool copyPbuffers(size_t x) {
        GNASH_REPORT_FUNCTION;
        if (x < _pbuffers.size()) {
            NativePixmapType pix;
            if (!eglCopyBuffers(_eglDisplay, _pbuffers[x], pix)) {
                log_error( "eglCopyBuffers() failed (error 0x%x)", eglGetError());
                return false;
            }
            return true;
        }
        return false;
    }
    // Make one of the pbuffers the current one to draw into
    bool makePbufferCurrent() {
        if ((_eglDisplay != EGL_NO_DISPLAY) && (_eglSurface != EGL_NO_SURFACE)) {
            if (!eglMakeCurrent(_eglDisplay, _eglSurface, _eglSurface, _eglContext)) {
                log_error( "eglMakeCurrent() failed (error 0x%x)", eglGetError());
                return false;
            }
        }
        return false;
    }
    
    bool makePbufferCurrent(size_t x) {
        if (x < _pbuffers.size()) {
            if ((_eglDisplay != EGL_NO_DISPLAY) && (_eglContext != EGL_NO_CONTEXT)) {
                if (!eglMakeCurrent(_eglDisplay, _pbuffers[x], _pbuffers[x], _eglContext)) {
                    log_error( "eglMakeCurrent() failed (error 0x%x)", eglGetError());
                    return false;
                }
                return true;
            }
        }
        return false;        
    }
    
    size_t getVerticalRes() {
        EGLint value = 0;
        if ((_eglDisplay != EGL_NO_DISPLAY) && (_eglSurface != EGL_NO_SURFACE)) {
            eglQuerySurface(_eglDisplay, _eglSurface, EGL_VERTICAL_RESOLUTION, &value);
        }
        return static_cast<size_t>(value);
    }
    size_t getHorzRes() {
        EGLint value = 0;
        if ((_eglDisplay != EGL_NO_DISPLAY) && (_eglSurface != EGL_NO_SURFACE)) {
            eglQuerySurface(_eglDisplay, _eglSurface, EGL_HORIZONTAL_RESOLUTION, &value);
        }
        return static_cast<size_t>(value);
    }
    bool isBackBuffered() {
        EGLint value;
        if ((_eglDisplay != EGL_NO_DISPLAY) && (_eglSurface != EGL_NO_SURFACE)) {
            eglQuerySurface(_eglDisplay, _eglSurface, EGL_RENDER_BUFFER, &value);
            if (value == EGL_BACK_BUFFER) {
                return true;
            }
            return false;
        }
        return false;
    }

    bool isMultiSample() {
        EGLint value;
        if ((_eglDisplay != EGL_NO_DISPLAY) && (_eglSurface != EGL_NO_SURFACE)) {
            eglQuerySurface(_eglDisplay, _eglSurface, EGL_MULTISAMPLE_RESOLVE, &value);
            if (value == EGL_MULTISAMPLE_RESOLVE_BOX) {
                return true;
            }
            return false;
        }
        return false;
    }
    
    EGLint getSurfaceID() {
        EGLint value = -1;
        if ((_eglDisplay != EGL_NO_DISPLAY) && (_eglSurface != EGL_NO_SURFACE)) {
            eglQuerySurface(_eglDisplay, _eglSurface, EGL_CONFIG_ID, &value);
        }
        return value;
    }

    // Context accessors
    EGLint getContextID() {
        EGLint value = -1;
        if ((_eglDisplay != EGL_NO_DISPLAY) && (_eglContext != EGL_NO_CONTEXT)) {
            eglQueryContext(_eglDisplay, _eglContext, EGL_CONFIG_ID, &value);
        }
        return value;
    }
    bool isContextSingleBuffered() {
        EGLint value;
        if ((_eglDisplay != EGL_NO_DISPLAY) && (_eglContext != EGL_NO_CONTEXT)) {
            eglQueryContext(_eglDisplay, _eglContext, EGL_RENDER_BUFFER, &value);
            if (value == EGL_SINGLE_BUFFER) {
                return true;
            }
            return false;
        }
        return false;
    }
    bool isContextBackBuffered() {
        EGLint value;
        if ((_eglDisplay != EGL_NO_DISPLAY) && (_eglContext != EGL_NO_CONTEXT)) {
            eglQueryContext(_eglDisplay, _eglContext, EGL_RENDER_BUFFER, &value);
            if (value == EGL_BACK_BUFFER) {
                return true;
            }
            return false;
        }
        return false;
    }

    // Config accessors
    EGLint getSamples() {
        EGLint value = -1;
        if ((_eglDisplay != EGL_NO_DISPLAY) && (_eglConfig != 0)) {
            eglGetConfigAttrib(_eglDisplay, _eglConfig, EGL_SAMPLES, &value);
        }
        return value;
    }
    EGLint getSampleBuffers() {
        EGLint value = -1; 
        if ((_eglDisplay != EGL_NO_DISPLAY) && (_eglConfig != 0)) {
            eglGetConfigAttrib(_eglDisplay, _eglConfig, EGL_SAMPLE_BUFFERS, &value);
        }
        return value;
    }
    EGLint getMaxSwapInterval() {
        EGLint value = -1;
        if ((_eglDisplay != EGL_NO_DISPLAY) && (_eglConfig != 0)) {
            eglGetConfigAttrib(_eglDisplay, _eglConfig, EGL_MAX_SWAP_INTERVAL, &value);
        }
        return value;
    }
    EGLint getMinSwapInterval() {
        EGLint value = -1;
        if ((_eglDisplay != EGL_NO_DISPLAY) && (_eglConfig != 0)) {
            eglGetConfigAttrib(_eglDisplay, _eglConfig, EGL_MIN_SWAP_INTERVAL, &value);
        }
        return value;
    }
    
    void setAttrib(int bpp);
    static EGLint getRenderableTypes();
protected:
    EGLConfig           _eglConfig;
    EGLContext          _eglContext;
    EGLSurface          _eglSurface;
    EGLDisplay          _eglDisplay;
    EGLNativeWindowType _nativeWindow;
    EGLNativePixmapType _nativePixmap;
    quality_e           _quality;
    const EGLint       *_attrib;
    unsigned int        _bpp;
    std::vector<EGLSurface> _pbuffers;
};

#define DUMP_CURRENT_SURFACE printEGLSurface(eglGetCurrentSurface(EGL_DRAW))
#define DUMP_CURRENT_CONTEXT printEGLContext(eglGetCurrentContext())

} // namespace renderer
} // namespace gnash

#endif  // end of __EGL_DEVICE_H__

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
