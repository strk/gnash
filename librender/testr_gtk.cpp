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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <map>
#include <cassert>
#include <regex.h>
#include <boost/assign/list_of.hpp>

#ifdef HAVE_GTK2
# include <gtk/gtk.h>
# include <gdk/gdk.h>
#endif
#ifdef RENDERER_AGG
# include "agg/Renderer_agg.h"
#endif
#ifdef RENDERER_OPENGL
# include "opengl/Renderer_ogl.h"
#endif
#ifdef RENDERER_OPENVG
# include <VG/vgu.h>
# ifdef OPENVG_VERSION_1_1
#  include <VG/ext.h>
# else
#  include <VG/vgext.h>
# endif
# include <VG/openvg.h>
#endif
#ifdef RENDERER_GLES1
# include "opengles1/Renderer_gles1.h"
#endif
# ifdef RENDERER_GLES2
#  include "opengles2/Renderer_gles2.h"
# endif
#ifdef RENDERER_CAIRO
# include "cairo/Renderer_cairo.h"
#endif

#ifdef BUILD_EGL_DEVICE
# include <egl/eglDevice.h>
#endif
#ifdef BUILD_DIRECTFB_DEVICE
# include <directfb/directfb.h>
#endif
#ifdef BUILD_X11_DEVICE
# include <x11/X11Device.h>
#endif

#include "log.h"
#include "SWFMatrix.h"
#include "Renderer.h"
#include "Transform.h"
#include "GnashVaapiImage.h"
#include "GnashVaapiImageProxy.h"

using namespace gnash; 
using namespace std; 

const VGfloat white_color[4] = {1.0, 1.0, 1.0, 1.0};
const VGfloat color[4] = {0.4, 0.1, 1.0, 1.0};

VGPath path;
VGPaint paint;
VGPaint fill;

// The debug log used by all the gnash libraries.
static LogFile& dbglogfile = LogFile::getDefaultInstance();


static void
initVGDemo(void)
{
    VGfloat clearColor[] = {0.0f, 0.0f, 0.0f, 1.0f};/* black color */
    VGfloat greenColor[] = {0.0f, 1.0f, 0.0f, 1.0f};/* green color */
    VGint arcType = VGU_ARC_OPEN;
    VGfloat x, y, w, h, startAngle, angleExtent;

    x = 150;
    y = 150;
    w = 150;
    h = 150;
#if 0
    startAngle  = -540.0f;
    angleExtent = 270.0f;
#else
    startAngle  = 270.0f;
    angleExtent = 90.0f;
#endif
    paint = vgCreatePaint();

    vgSetPaint(paint, VG_STROKE_PATH);
    vgSetParameterfv(paint, VG_PAINT_COLOR, 4, greenColor);
    vgSetParameteri( paint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
    vgSetf(VG_STROKE_LINE_WIDTH, 6.0f);
    vgSeti(VG_RENDERING_QUALITY, VG_RENDERING_QUALITY_NONANTIALIASED);
    vgSetfv(VG_CLEAR_COLOR, 4, clearColor);

    path  = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,
                         1.0f, 0.0f, 0, 0, VG_PATH_CAPABILITY_ALL);

    vguArc(path, x, y, w, h, startAngle, angleExtent, (VGUArcType)arcType);

    vgSeti(VG_STROKE_CAP_STYLE, VG_CAP_BUTT);
    vgSeti(VG_STROKE_JOIN_STYLE, VG_JOIN_BEVEL);
    vgSetf(VG_STROKE_MITER_LIMIT, 4.0f);
}

/* new window size or exposure */
static void
init(void)
{
    static const VGubyte sqrCmds[5] = {VG_MOVE_TO_ABS, VG_HLINE_TO_ABS,
                                       VG_VLINE_TO_ABS, VG_HLINE_TO_ABS, VG_CLOSE_PATH};
    static const VGfloat sqrCoords[5] = {50.0f, 50.0f, 250.0f, 250.0f, 50.0f};
    path = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1, 0, 0, 0,
                        VG_PATH_CAPABILITY_APPEND_TO);
    vgAppendPathData(path, 5, sqrCmds, sqrCoords);
    
    VGPaint fill = vgCreatePaint();
    vgSetParameterfv(fill, VG_PAINT_COLOR, 4, color);
    vgSetPaint(fill, VG_FILL_PATH);
    
    vgSetfv(VG_CLEAR_COLOR, 4, white_color);
    vgSetf(VG_STROKE_LINE_WIDTH, 10);
    vgSeti(VG_STROKE_CAP_STYLE, VG_CAP_BUTT);
    vgSeti(VG_STROKE_JOIN_STYLE, VG_JOIN_ROUND);
    vgSetf(VG_STROKE_MITER_LIMIT, 4.0f);
    
//    printVGParams();
}

static void
reshape(int w, int h)
{
   vgLoadIdentity();
}

static void
draw(void)
{
    vgClear(0, 0, 640, 480);
    vgSeti(VG_MATRIX_MODE, VG_MATRIX_STROKE_PAINT_TO_USER);
    vgLoadIdentity();
    vgScale(2.25, 2.25);
    vgDrawPath(path, VG_STROKE_PATH);
    
    vgFlush();
}

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
