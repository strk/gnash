// 
//   Copyright (C) 2010 Free Software Foundation, Inc
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


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

// gles-1.0c for Linux
#ifdef HAVE_GLES1_GL_H
# include <GLES/gl.h>
#endif
#ifdef HAVE_GLES1_EGL_H
# include <GLES/egl.h>
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

#include <cstring>
#include <cmath>

#include <smart_ptr.h>
#include "swf/ShapeRecord.h"
#include "RGBA.h"
#include "GnashImage.h"
#include "GnashTexture.h"
#include "GnashNumeric.h"
#include "GnashImage.h"
#include "log.h"
#include "utility.h"
#include "Range2d.h"
//#include "cxform.h"

#include "Renderer_gles1.h"

#include <boost/utility.hpp>
#include <boost/bind.hpp>

// Defined to 1 to disable (slow) anti-aliasing with the accumulation buffer
#define NO_ANTIALIASING 1

/// \file Renderer_gles.cpp
/// \brief The OpenGL-ES renderer and related code.
///

// TODO:
// - Profiling!
// - Optimize code:
// * Use display lists
// * Use better suited standard containers
// * convert to double at a later stage (oglVertex)
// * keep data for less time
// * implement hardware accelerated gradients. Most likely this will require
//   the use of fragment shader language.

// * The "Programming Tips" in the OpenGL "red book" discusses a coordinate system
// that would give "exact two-dimensional rasterization". AGG uses a similar
// system; consider the benefits and drawbacks of switching.

namespace gnash {

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
