// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
//
// port to OpenGL-Embedded Subset 1.1:
// Copyright (C) 2010 Sennheiser GmbH & Co. KG, Wedemark, Germany
// author: Bernd Kischnick <kisch@gmx.li>
//

#ifndef GNASH_RENDER_HANDLER_OGLES_H
#define GNASH_RENDER_HANDLER_OGLES_H

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
#ifdef HAVE_GLES_EGL_H
# include <GLES/egl.h>
# include <GLES/eglext.h>
#endif
// Mali Developer Tools for ARM 1.x
#ifdef HAVE_GLES2_GL2_H
# include <GLES2/gl2.h>
# include <GLES2/gl2ext.h>
#endif
#endif

namespace gnash {

typedef GLfloat oglCoord;
#define OGL_COORD GL_FLOAT
#define GL_LINE_WIDTH_RANGE GL_ALIASED_LINE_WIDTH_RANGE
#define glOrtho glOrthof
#define GLUCALLBACKTYPE GLvoid (*)()

class OglGlue;
class render_handler;

render_handler* create_render_handler_gles1 (bool init, OglGlue* glue);

} // namespace gnash

#endif // __RENDER_HANDLER_OGLES_H__

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
