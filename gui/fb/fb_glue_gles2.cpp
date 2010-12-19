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
#include "fb_glue_gles2.h"

namespace gnash {

namespace gui {

FBgles2Glue::FBgles2Glue(int fd)
    : _fd(fd)
{
}

FBgles2Glue::~FBgles2Glue ()
{
}

bool
FBgles2Glue::init(int /*argc*/, char *** /*argv*/)
{
    return false;
}

Renderer*
FBgles2Glue::createRenderHandler()
{
    Renderer *rend = 0;
    return rend;
}

int 
FBgles2Glue::width() {
}

int
FBgles2Glue::height() {
}

void
FBgles2Glue::render() {
}

void 
FBgles2Glue::render_to_pbuffer () {
}

void 
FBgles2Glue::prepare_copy_from_pbuffer () {
}

void 
FBgles2Glue::render_to_display () {
}

} // end of namespace gui
} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
