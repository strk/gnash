// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010
//              Free Software Foundation, Inc.
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

#ifndef FB_GLUE_GLES1_H
#define FB_GLUE_GLES1_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

// gles-1.0c for Linux
#ifdef HAVE_GLES1_GL_H
# include <GLES/gl.h>
# endif
#ifdef HAVE_GLES1_EGL_H
#include <GLES/egl.h>
#endif
#if 0
// Mali Developer Tools for ARM 1.x
#ifdef HAVE_EGL_EGL_H
# include <EGL/egl.h>
# include <EGL/eglext.h>
#endif
// Mali Developer Tools for ARM 2.x and Android 2.1
#ifdef HAVE_GLES2_GL2_H
# include <GLES2/gl2.h>
# include <GLES2/gl2ext.h>
#endif
#endif

#include "fbsup.h"
#include "fb_glue.h"

#ifdef BUILD_EGL_DEVICE
# include "egl/eglDevice.h"
#endif

#include <memory>

#include "fbsup.h"

namespace gnash {

namespace gui {

class render_handler;

class FBgles1Glue: public FBGlue
{
public:
    FBgles1Glue(int fd);
    ~FBgles1Glue();
    bool init(int argc, char ***);
    
    Renderer* createRenderHandler();
    void setInvalidatedRegions(const InvalidatedRanges& /* ranges */) {}
    
    int width ();
    int height ();
    void render ();
    
    void render_to_pbuffer ();
    void prepare_copy_from_pbuffer ();
    void render_to_display ();
       /// \brief
    ///  Hand off a handle to the native drawing area to the renderer
    void prepDrawingArea(void *drawing_area);
    
    void initBuffer(int width, int height);
    void resize(int width, int height);
    // void render(geometry::Range2d<int>& bounds);
    
protected:
    int         _fd;

private:
    std::unique_ptr<Renderer> _renderer;
    EGLDisplay  _display;
    EGLConfig   _config;
    EGLContext  _context;
    EGLSurface  _surface;
    EGLConfig   _pbuffer_config;
    EGLSurface  _pbuffer;
};

} // end of namespace gui
} // namespace gnash

#endif // FB_GLUE_GLES1_H

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
