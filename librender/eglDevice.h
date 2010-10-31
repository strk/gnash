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
    void printEGLConfig() { return printEGLConfig(_eglConfig); };
    void printEGLConfig(EGLConfig config);
    void printEGLContext() { return printEGLContext(_eglContext); };
    void printEGLContext(EGLContext context);
    void printEGLSurface() { return printEGLSurface(_eglSurface); };
    void printEGLSurface(EGLSurface surface);

    // Accessors for the setting needed by higher level code
    EGLint getWidth() {
        EGLint value;
        eglQuerySurface(_eglDisplay, _eglSurface, EGL_WIDTH, &value);
        return value;
    };
    EGLint getHeigth() {
        EGLint value;
        eglQuerySurface(_eglDisplay, _eglSurface, EGL_HEIGHT, &value);
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
    bool supportsRenderer(rtype_t rtype) {
        EGLint value;
        eglQuerySurface(_eglDisplay, _eglSurface, EGL_HEIGHT, &value);
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
    bool isSingleBuffered() {
        EGLint value;
        eglQuerySurface(_eglDisplay, _eglSurface, EGL_RENDER_BUFFER, &value);
        if (value == EGL_SINGLE_BUFFER) {
            return true;
        }
        return false;
    }
    
    bool isBackBuffered() {
        EGLint value;
        eglQuerySurface(_eglDisplay, _eglSurface, EGL_RENDER_BUFFER, &value);
        if (value == EGL_BACK_BUFFER) {
            return true;
        }
        return false;
    }

    
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
