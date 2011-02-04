//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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


#include "kde_glue_opengl.h"
#include "tu_opengl_includes.h"

using namespace std;

namespace gnash
{

KdeOpenGLGlue::KdeOpenGLGlue()
#ifdef FIX_I810_LOD_BIAS
  : _tex_lod_bias(-1.2f)
#endif
{
}

KdeOpenGLGlue::~KdeOpenGLGlue()
{
}

bool
KdeOpenGLGlue::init(int argc, char **argv[])
{
//    GNASH_REPORT_FUNCTION;

#ifdef FIX_I810_LOD_BIAS
    int c = getopt (argc, *argv, "m:");
    if (c == 'm') {
      _tex_lod_bias = (float) strtof(optarg, NULL);
    }
#endif
    return true;
}


void
KdeOpenGLGlue::prepDrawingArea(QWidget *drawing_area)
{
//    GNASH_REPORT_FUNCTION;
    static_cast<QGLWidget*>(drawing_area)->makeCurrent();
    _drawing_area = drawing_area;
}

Renderer*
KdeOpenGLGlue::createRenderHandler()
{
//    GNASH_REPORT_FUNCTION;
    Renderer* renderer = create_Renderer_ogl();

#ifdef FIX_I810_LOD_BIAS
    glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, _tex_lod_bias);
#endif
    return renderer;
}

void
KdeOpenGLGlue::render()
{
//    GNASH_REPORT_FUNCTION;
    
    static_cast<QGLWidget*>(_drawing_area)->swapBuffers();
}

// end of namespace gnash
}
