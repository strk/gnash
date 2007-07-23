// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
//

/* $Id: aqua_ogl_glue.cpp,v 1.13 2007/07/23 01:01:31 nihilus Exp $ */


#include "aqua_ogl_glue.h"
#include <AGL/agl.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#include "log.h"

#define OVERSIZE 1.0f

using namespace std;

namespace gnash
{

AquaOglGlue::AquaOglGlue()
#ifdef FIX_I810_LOD_BIAS
  : _tex_lod_bias(-1.2f)
#endif
{
//    GNASH_REPORT_FUNCTION;
}

AquaOglGlue::~AquaOglGlue()
{
//    GNASH_REPORT_FUNCTION;

}


bool
#ifdef FIX_I810_LOD_BIAS
AquaOglGlue::init(int argc, char** argv[])
#else
AquaOglGlue::init(int, char***)
#endif
{
//    GNASH_REPORT_FUNCTION;
#ifdef FIX_I810_LOD_BIAS
    int c = getopt (argc, *argv, "m:");
    if (c == 'm') {
      _tex_lod_bias = (float) atof(optarg);
    }
#endif
    return true;
}

render_handler* AquaOglGlue::createRenderHandler()
{
    GNASH_REPORT_FUNCTION;
    render_handler* renderer = create_render_handler_ogl();
#ifdef FIX_I810_LOD_BIAS
    glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, _tex_lod_bias);
#endif
    return renderer;
}

bool AquaOglGlue::prepDrawingArea(int width, int height)
{
    //SDL_SetVideoMode(width, height, _bpp, sdl_flags | SDL_OPENGL);

     // Turn on alpha blending.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                     
    // Turn on line smoothing.  Antialiased lines can be used to
    // smooth the outsides of shapes.
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST); // GL_NICEST, GL_FASTEST, GL_DONT_CARE
    glMatrixMode(GL_PROJECTION);


    glOrtho(-OVERSIZE, OVERSIZE, OVERSIZE, -OVERSIZE, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
 
    // We don't need lighting effects
    glDisable(GL_LIGHTING);
    glPushAttrib (GL_ALL_ATTRIB_BITS);         

#ifdef FIX_I810_LOD_BIAS
    glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, _tex_lod_bias);
#endif
    return true;
}

void AquaOglGlue::render()
{
    GNASH_REPORT_FUNCTION;
    //SDL_GL_SwapBuffers();
    glFlush();
}

} // namespace gnash