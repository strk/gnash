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

#ifndef __EGL_DEVICE_H__
#define __EGL_DEVICE_H__ 1

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/scoped_array.hpp>

#ifdef HAVE_GTK2
#include "X11/Xlib.h"
#include "X11/Xutil.h"
#endif
#ifdef HAVE_EGL_EGL_H
# include <EGL/egl.h>
#else
# error "This file needs EGL, which is part of OpenGL-ES"
#endif

namespace gnash
{

namespace renderer {

class EGLDevice
{
  public:
    // the list of supported renders that use EGL
    typedef enum {OPENVG, OPENGLES1, OPENGLES2} rtype_t;
    
    EGLDevice();
    ~EGLDevice();

    // Initialize EGL Device layer
    bool initDevice(rtype_t);

    // Initialize EGL Window layer
    bool initEGL(EGLNativeWindowType window);
    
    // Utility methods not in the base class
    /// Return a string with the error code as text, instead of a numeric value
    const char *getErrorString(int error);
    
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
    
    // Create Pbuffers for offscreen rendering
    EGLSurface createPbuffer(int width, int height);
    EGLSurface createPbuffer(int width, int height, EGLClientBuffer buf, EGLenum type);
    EGLSurface createPixmap(int width, int height, NativePixmapType buf);
    size_t totalPbuffers() { return _pbuffers.size(); };
    EGLSurface &operator[](int index) { return _pbuffers[index]; };

    // Swapping Buffers makes the specified surface active on the display if
    // EGL_RENDER_BUFFER is set to EGL_BACK_BUFFER. If it's set to
    // EGL_SINGLE_BUFFER then this has no effect, as the display was drawn to
    // directly.
    // Swap to the default surface
    bool swapPbuffers() {
        return eglSwapBuffers(_eglDisplay, _eglSurface);
    }
    bool copyPbuffers(int x) {
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
        if (!eglMakeCurrent(_eglDisplay, _eglSurface, _eglSurface, _eglContext)) {
            log_error( "eglMakeCurrent() failed (error 0x%x)", eglGetError());
            return false;
        }
    }
    
    bool makePbufferCurrent(int x) {
        if (x < _pbuffers.size()) {
            if (!eglMakeCurrent(_eglDisplay, _pbuffers[x], _pbuffers[x], _eglContext)) {
                log_error( "eglMakeCurrent() failed (error 0x%x)", eglGetError());
                return false;
            }
            return true;
        }
        return false;
    }
    
    // Accessors for the settings needed by higher level code.
    // Surface accessors
    EGLint getWidth() {
        return getWidth(_eglSurface);
    }
    
    EGLint getWidth(EGLSurface surface) {
        EGLint value;
        eglQuerySurface(_eglDisplay, surface, EGL_WIDTH, &value);
        return value;
    };
    EGLint getHeigth() {
        return getHeigth(_eglSurface);
    }
    
    EGLint getHeigth(EGLSurface surface) {
        EGLint value;
        eglQuerySurface(_eglDisplay, surface, EGL_HEIGHT, &value);
        return value;
    }
    EGLint getVerticalRes() {
        EGLint value;
        eglQuerySurface(_eglDisplay, _eglSurface, EGL_VERTICAL_RESOLUTION, &value);
        return value;
    }
    EGLint getHorzRes() {
        EGLint value;
        eglQuerySurface(_eglDisplay, _eglSurface, EGL_HORIZONTAL_RESOLUTION, &value);
        return value;
    }
    bool isSurfaceSingleBuffered() {
        return isSurfaceSingleBuffered(_eglSurface);
    }
    
    bool isSurfaceSingleBuffered(EGLSurface surface) {
        EGLint value;
        eglQuerySurface(_eglDisplay, surface, EGL_RENDER_BUFFER, &value);
        if (value == EGL_SINGLE_BUFFER) {
            return true;
        }
        return false;
    }
    
    bool isSurfaceBackBuffered() {
        return isSurfaceBackBuffered(_eglSurface);
    }
    bool isSurfaceBackBuffered(EGLSurface surface) {
        EGLint value;
        eglQuerySurface(_eglDisplay, surface, EGL_RENDER_BUFFER, &value);
        if (value == EGL_BACK_BUFFER) {
            return true;
        }
        return false;
    }

    bool isBufferDestroyed() {
        return isBufferDestroyed(_eglSurface);
    }
    bool isBufferDestroyed(EGLSurface surface) {
        EGLint value;
        eglQuerySurface(_eglDisplay, surface, EGL_SWAP_BEHAVIOR, &value);
        if (value == EGL_BUFFER_DESTROYED) {
            return true;
        }
        return false;
    }
    bool isMultiSample() {
        EGLint value;
        eglQuerySurface(_eglDisplay, _eglSurface, EGL_MULTISAMPLE_RESOLVE, &value);
        if (value == EGL_MULTISAMPLE_RESOLVE_BOX) {
            return true;
        }
        return false;
    }
    EGLint getSurfaceID() {
        EGLint value;
        eglQuerySurface(_eglDisplay, _eglSurface, EGL_CONFIG_ID, &value);
        return value;
    }

    // Context accessors
    EGLint getContextID() {
        EGLint value;
        eglQueryContext(_eglDisplay, _eglContext, EGL_CONFIG_ID, &value);
        return value;
    }
    bool supportsClient(rtype_t rtype) {
        EGLint value;
        eglQueryContext(_eglDisplay, _eglContext, EGL_CONTEXT_CLIENT_TYPE, &value);
        std::string str;
        if ((value == EGL_OPENGL_ES_API) && (rtype == EGLDevice::OPENGLES2)) {
            return true;
        }
        if ((value == EGL_OPENGL_ES_API) && (rtype == EGLDevice::OPENGLES1)) {
            return true;
        }
        if ((value == EGL_OPENVG_API) && (rtype == EGLDevice::OPENVG)){
            return true;
        }
        return false;
    }
    bool isContextSingleBuffered() {
        EGLint value;
        eglQueryContext(_eglDisplay, _eglContext, EGL_RENDER_BUFFER, &value);
        if (value == EGL_SINGLE_BUFFER) {
            return true;
        }
        return false;
    }
    bool isContextBackBuffered() {
        EGLint value;
        eglQueryContext(_eglDisplay, _eglContext, EGL_RENDER_BUFFER, &value);
        if (value == EGL_BACK_BUFFER) {
            return true;
        }
        return false;
    }

    // Config accessors
    bool supportsRenderer(rtype_t rtype) {
        EGLint value;
        eglGetConfigAttrib(_eglDisplay, _eglConfig, EGL_RENDERABLE_TYPE, &value);
        
        if (value > 0) {
            std::string str;
            if ((value & EGL_OPENGL_ES2_BIT) && (rtype == EGLDevice::OPENGLES2)) {
                return true;
            }
            if ((value & EGL_OPENGL_ES_BIT) && (rtype == EGLDevice::OPENGLES1)) {
                return true;
            }
            if ((value & EGL_OPENVG_BIT) && (rtype == EGLDevice::OPENVG)){
                return true;
            }
        }
        return false;
    }
    bool isNativeRender() {
        EGLint value;
        eglGetConfigAttrib(_eglDisplay, _eglConfig, EGL_NATIVE_RENDERABLE, &value);
        return value;
    }
    EGLint getSamples() {
        EGLint value;
        eglGetConfigAttrib(_eglDisplay, _eglConfig, EGL_SAMPLES, &value);
        return value;
    }
    EGLint getSampleBuffers() {
        EGLint value; 
        eglGetConfigAttrib(_eglDisplay, _eglConfig, EGL_SAMPLE_BUFFERS, &value);
        return value;
    }
    EGLint getDepth() {
        EGLint value;
        eglGetConfigAttrib(_eglDisplay, _eglConfig, EGL_DEPTH_SIZE, &value);
        return value;
    }
    EGLint getMaxSwapInterval() {
        EGLint value;
        eglGetConfigAttrib(_eglDisplay, _eglConfig, EGL_MAX_SWAP_INTERVAL, &value);
        return value;
    }
    EGLint getMinSwapInterval() {
        EGLint value;
        eglGetConfigAttrib(_eglDisplay, _eglConfig, EGL_MIN_SWAP_INTERVAL, &value);
        return value;
    }
    // bool i() {
    //     EGLint value;
    //     return value;
    // }
    
protected:
    EGLConfig           _eglConfig;
    EGLContext          _eglContext;
    EGLSurface          _eglSurface;
    EGLDisplay          _eglDisplay;
    EGLint              _eglNumOfConfigs;
    EGLNativeWindowType _nativeWindow;
    EGLint              _max_num_config;
    unsigned int        _bpp;
    unsigned int        _width;
    unsigned int        _height;
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
