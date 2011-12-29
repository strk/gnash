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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

// gles-1.0c for Linux
#ifdef HAVE_GLES1_GL_H
# include <GLES/gl.h>
#endif
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

#include "log.h"
#include "fb_glue_gles1.h"

namespace gnash {

namespace gui {

FBgles1Glue::FBgles1Glue(int fd)
    : _fd (fd),
      _surface (EGL_NO_SURFACE),
      _pbuffer (EGL_NO_SURFACE)
{
    GNASH_REPORT_FUNCTION;    
}

FBgles1Glue::~FBgles1Glue ()
{
    GNASH_REPORT_FUNCTION;

    eglMakeCurrent(_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglTerminate(_display);
}

Renderer *
FBgles1Glue::createRenderHandler()
{
    GNASH_REPORT_FUNCTION;

    //_render_handler = create_render_handler_ogl (true, this);
    //        return _render_handler; FIXME: 
    // error: invalid covariant return type for 'virtual gnash::render_handler* gnash::FBglesGlue::createRenderHandler()'
    Renderer *rend = 0;
    return rend;
}

bool 
FBgles1Glue::init(int /*argc*/, char *** /*argv*/)
{
    GNASH_REPORT_FUNCTION;

    EGLint majorVersion, minorVersion;
    EGLint numOfConfigs = 0;
    EGLint result;
    const EGLint main_attrib_list[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BUFFER_SIZE, 32,
        EGL_DEPTH_SIZE, 16,
        EGL_NONE
    };
    
    _display = eglGetDisplay((NativeDisplayType)0);
    if (_display == EGL_NO_DISPLAY) {
        return false;
    }
    log_trace(_("EGL: getDisplay ok"));
    
    result = eglInitialize(_display, &majorVersion, &minorVersion);
    if (result == EGL_FALSE) {
        return false;
    }
    log_trace(_("EGL: initialize ok"));
    
    result = eglChooseConfig(_display, main_attrib_list, &_config, 1,
                             &numOfConfigs);
    if (result == EGL_FALSE || numOfConfigs != 1) {
        return false;
    }
    log_trace(_("EGL: config ok"));
    
    _surface = eglCreateWindowSurface(_display, _config, (NativeWindowType)0,
                                      NULL);
    if (eglGetError () != EGL_SUCCESS) {
        return false;
    }
    log_trace(_("EGL: surface ok"));
    
    _context = eglCreateContext(_display, _config, NULL, NULL);
    if (eglGetError () != EGL_SUCCESS) {
        return false;
    }
    log_trace(_("EGL: context ok"));
    
    eglMakeCurrent(_display, _surface, _surface, _context);
    if (eglGetError () != EGL_SUCCESS) {
        return false;
    }
    log_trace(_("EGL: current ok"));
    
    const EGLint pbuffer_config_list[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_BUFFER_SIZE, 32,
        EGL_DEPTH_SIZE, 0,
        EGL_BIND_TO_TEXTURE_RGBA, EGL_TRUE,
        EGL_CONFIG_CAVEAT, EGL_NONE,
        EGL_NONE
    };
    
    result = eglChooseConfig(_display, pbuffer_config_list, &_pbuffer_config,
                             1, &numOfConfigs);
    if (result == EGL_FALSE || numOfConfigs == 0) {
        return false;
    }
    log_trace(_("EGL: pbuffer config ok"));
    
    const EGLint pbuffer_attrib_list[] = {
        EGL_WIDTH, EGL_MAX_PBUFFER_WIDTH,
        EGL_HEIGHT, EGL_MAX_PBUFFER_HEIGHT,
        EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGBA,
        EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
        EGL_MIPMAP_TEXTURE, EGL_FALSE,
        EGL_NONE
    };
    
    _pbuffer = eglCreatePbufferSurface(_display, _pbuffer_config,
                                       pbuffer_attrib_list);
    if (eglGetError () != EGL_SUCCESS) {
        return false;
    }
    log_trace(_("EGL: pbuffer surface ok"));
    
    return true;
}

int 
FBgles1Glue::width() {
    EGLint result;
    eglQuerySurface (_display, _surface, EGL_WIDTH, &result);
    log_trace(_("EGL: width %d"), result);
    return result;
}

int
FBgles1Glue::height() {
    EGLint result;
    eglQuerySurface (_display, _surface, EGL_HEIGHT, &result);
    log_trace(_("EGL: height %d"), result);
    return result;
}

void
FBgles1Glue::render() {
    eglSwapBuffers(_display, _surface);
}

void 
FBgles1Glue::render_to_pbuffer () {
    if (_pbuffer != EGL_NO_SURFACE)
        eglMakeCurrent(_display, _pbuffer, _pbuffer, _context);
}

void 
FBgles1Glue::prepare_copy_from_pbuffer () {
    if (_pbuffer != EGL_NO_SURFACE)
        eglMakeCurrent(_display, _surface, _pbuffer, _context);
}

void 
FBgles1Glue::render_to_display () {
    if (_pbuffer != EGL_NO_SURFACE)
        eglMakeCurrent(_display, _surface, _surface, _context);
}

} // end of namespace gui
} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
