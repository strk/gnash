//    Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software  Foundation, Inc
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


#include "aqua_ogl_glue.h"
#include "log.h"

#define OVERSIZE 1.0f

using namespace std;

namespace gnash
{

AquaOglGlue::AquaOglGlue()
: _context(NULL)
#ifdef FIX_I810_LOD_BIAS
  ,_tex_lod_bias(-1.2f)
#endif
   
{
//    GNASH_REPORT_FUNCTION;
}

AquaOglGlue::~AquaOglGlue()
{
//    GNASH_REPORT_FUNCTION;
  aglDestroyContext (_context);
}


bool AquaOglGlue::init(int argc, char **argv[])
{
//    GNASH_REPORT_FUNCTION;
#ifdef FIX_I810_LOD_BIAS
    int c = getopt (argc, *argv, "m:");
    if (c == 'm') {
      _tex_lod_bias = (float) strtof(optarg, NULL);
    }
#endif

    const GLint glattribs[] = { AGL_RGBA, AGL_ACCELERATED,
                                AGL_DEPTH_SIZE, 32,
                                AGL_ACCUM_RED_SIZE, 8,
                                AGL_ACCUM_GREEN_SIZE, 8,
                                AGL_ACCUM_RED_SIZE, 8,
                                AGL_ACCUM_ALPHA_SIZE, 8,
                                AGL_DOUBLEBUFFER, AGL_NONE };
                              
    AGLPixelFormat pixfmt = aglChoosePixelFormat ( NULL, 0, glattribs);
        

    _context = aglCreateContext (pixfmt, NULL);
    if (!_context) {
      aglDestroyPixelFormat(pixfmt);
      return false;
    }

    bool ret = aglSetCurrentContext(_context);
    aglDestroyPixelFormat(pixfmt);

    return ret; 
}

Renderer* AquaOglGlue::createRenderHandler()
{
//    GNASH_REPORT_FUNCTION;
    Renderer* renderer = create_Renderer_ogl();
#ifdef FIX_I810_LOD_BIAS
    glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, _tex_lod_bias);
#endif
    return renderer;
}

bool AquaOglGlue::prepDrawingArea(int width, int height, AGLDrawable drawable)
{
	GNASH_REPORT_FUNCTION;
    bool ret = aglSetDrawable(_context, drawable);
    
    return ret;
}

void AquaOglGlue::render()
{
    GNASH_REPORT_FUNCTION;
    assert(aglSetCurrentContext(_context));
    aglUpdateContext(_context);

    aglSwapBuffers(_context);
}

} // namespace gnash
