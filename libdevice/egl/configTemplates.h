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

#ifndef __CONFIG_TEMPLATES_H__
#define __CONFIG_TEMPLATES_H__ 1

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#ifdef HAVE_EGL_EGL_H
# include <EGL/egl.h>
#else
# error "This file needs EGL, which is part of OpenGL-ES"
#endif

#include "eglDevice.h"

namespace gnash {
namespace renderer {

// From the EGL 1.4 spec:
//
// EGL defines several types of drawing surfaces collectively referred
// to as EGLSurfaces. These include windows, used for onscreen
// rendering; pbuffers, used for offscreen rendering; and pixmaps,
// used for offscreen rendering into buffers that may be accessed
// through native APIs. EGL windows and pixmaps are tied to native
// window system windows and pixmaps.
//
// depth, multisample, and stencil buffers are currently used only by
// OpenGL-ES.

// EGL and OpenGL ES supports two rendering models: back buffered and
// single buffered. Back buffered rendering is used by window and
// pbuffer surfaces. Memory for the color buffer used during rendering
// is allocated and owned by EGL. When the client is finished drawing
// a frame, the back buffer may be copied to a visible window using
// eglSwapBuffers. Pbuffer surfaces have a back buffer but no
// associated window, so the back buffer need not be copied.
//
// Single buffered rendering is used by pixmap surfaces. Memory for
// the color buffer is specified at surface creation time in the form
// of a native pixmap, and client APIs are required to use that memory
// during rendering. When the client is finished drawing a frame, the
// native pixmap contains the final image. Pixmap surfaces typically
// do not support multisampling, since the native pixmap used as the
// color buffer is unlikely to provide space to store multisample
// information. Some client APIs , such as OpenGL and OpenVG , also
// support single buffered rendering to window surfaces. This behavior
// can be selected when creating the window surface, as defined in
// section 3.5.1. When mixing use of client APIs which do not support
// single buffered rendering into windows, like OpenGL ES , with
// client APIs which do support it, back color buffers and visible
// window contents must be kept consistent when binding window
// surfaces to contexts for each API type. Both back and single
// buffered surfaces may also be copied to a specified native pixmap
// using eglCopyBuffers.

// Native rendering will always be supported by pixmap surfaces (to
// the extent that native rendering APIs can draw to native
// pixmaps). Pixmap surfaces are typically used when mixing native and
// client API rendering is desirable, since there is no need to move
// data between the back buffer visible to the client APIs and the
// native pixmap visible to native rendering APIs. However, pixmap
// surfaces may, for the same reason, have restricted capabilities and
// performance relative to window and pbuffer surfaces.

// NOTE: Single Buffering appears not to work on X11, you get no visual.
//    EGL_RENDER_BUFFER, EGL_SINGLE_BUFFER,

// These are the attributes for a 24bpp, 32bit display
static const EGLint attrib32_low[] = {
    EGL_RED_SIZE,        8,
    EGL_GREEN_SIZE,      8,
    EGL_BLUE_SIZE,       8,
    EGL_ALPHA_SIZE,      0,
    EGL_DEPTH_SIZE,      24,
    EGL_RENDERABLE_TYPE, EGLDevice::getRenderableTypes(),
    EGL_SURFACE_TYPE,    EGL_WINDOW_BIT|EGL_PBUFFER_BIT|EGL_PIXMAP_BIT,
    EGL_SAMPLES,         0,
    EGL_SAMPLE_BUFFERS,  0,  
    EGL_NONE
};

static const EGLint attrib32_medium[] = {
    EGL_RED_SIZE,        8,
    EGL_GREEN_SIZE,      8,
    EGL_BLUE_SIZE,       8,
    EGL_ALPHA_SIZE,      0,
    EGL_DEPTH_SIZE,      24,
    EGL_RENDERABLE_TYPE, EGLDevice::getRenderableTypes(),
    EGL_SURFACE_TYPE,    EGL_WINDOW_BIT|EGL_PBUFFER_BIT|EGL_PIXMAP_BIT,
    EGL_SAMPLES,         2,
    EGL_SAMPLE_BUFFERS,  1,  
    EGL_NONE
};

static const EGLint attrib32_high[] = {
    EGL_RED_SIZE,        8,
    EGL_GREEN_SIZE,      8,
    EGL_BLUE_SIZE,       8,
    EGL_DEPTH_SIZE,      24,
    EGL_ALPHA_SIZE,      0,
    EGL_RENDERABLE_TYPE, EGLDevice::getRenderableTypes(),
    EGL_SURFACE_TYPE,    EGL_WINDOW_BIT|EGL_PBUFFER_BIT|EGL_PIXMAP_BIT,
    EGL_SAMPLES,         4,
    EGL_SAMPLE_BUFFERS,  1,  
    EGL_NONE
};

// These are the attributes for a 24bpp, 32bit display
static EGLint const attrib16_low[] = {
    EGL_RED_SIZE,        5,
    EGL_GREEN_SIZE,      6,
    EGL_BLUE_SIZE,       5,
    EGL_ALPHA_SIZE,      0,
    EGL_RENDERABLE_TYPE, EGLDevice::getRenderableTypes(),
//  EGL_LUMINANCE_SIZE,  EGL_DONT_CARE,
//  EGL_SURFACE_TYPE,    EGL_VG_COLORSPACE_LINEAR_BIT,
    EGL_SAMPLES,         0,
    EGL_DEPTH_SIZE,      16,
    EGL_SAMPLES,         0,
    EGL_SAMPLE_BUFFERS,  0,  
    EGL_NONE
};

static EGLint const attrib16_medium[] = {
    EGL_RED_SIZE,        5,
    EGL_GREEN_SIZE,      6,
    EGL_BLUE_SIZE,       5,
    EGL_ALPHA_SIZE,      0,
    EGL_RENDERABLE_TYPE, EGLDevice::getRenderableTypes(),
    EGL_LUMINANCE_SIZE,  EGL_DONT_CARE,
    EGL_SURFACE_TYPE,    EGL_VG_COLORSPACE_LINEAR_BIT,
    EGL_SAMPLES,         0,
    EGL_DEPTH_SIZE,      16,
    EGL_SAMPLES,         2,
    EGL_SAMPLE_BUFFERS,  1,  
    EGL_NONE
};

static EGLint const attrib16_high[] = {
    EGL_RED_SIZE,        5,
    EGL_GREEN_SIZE,      6,
    EGL_BLUE_SIZE,       5,
    EGL_ALPHA_SIZE,      0,
    EGL_RENDERABLE_TYPE, EGLDevice::getRenderableTypes(),
    EGL_LUMINANCE_SIZE,  EGL_DONT_CARE,
    EGL_SURFACE_TYPE,    EGL_VG_COLORSPACE_LINEAR_BIT,
    EGL_SAMPLES,         0,
    EGL_DEPTH_SIZE,      16,
    EGL_SAMPLES,         4,
    EGL_SAMPLE_BUFFERS,  1,  
    EGL_NONE
};

// These are the same EGL config settings as used by the Mesa
// examples, which run on X11.
static const EGLint attrib1_list[] = {
    EGL_RED_SIZE, 1,
    EGL_GREEN_SIZE, 1,
    EGL_BLUE_SIZE, 1,
    EGL_RENDERABLE_TYPE, EGL_OPENVG_BIT,
    EGL_NONE
};

const EGLint surface_attributes[] = {
    // Back buffering is used for window and pbuffer surfaces. Windows
    // require eglSwapBuffers() to become visible, and pbuffers don't.   
    // EGL_SINGLE_BUFFER is by pixmap surfaces. With OpenVG, windows
    // can also be single buffered. eglCopyBuffers() can be used to copy
    // both back and single buffered surfaces to a pixmap.
    EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
    EGL_VG_COLORSPACE, EGL_VG_COLORSPACE_sRGB,
    EGL_NONE
};

#endif  // end of __CONFIG_TEMPLATES_H__

} // namespace renderer
} // namespace gnash

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
