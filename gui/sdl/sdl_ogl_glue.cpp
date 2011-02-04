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

#include "sdl_ogl_glue.h"
#include "Renderer_ogl.h"
#include "tu_opengl_includes.h"
#include "SDL.h"

#include "log.h"

#define OVERSIZE 1.0f

using namespace std;


namespace gnash
{

SdlOglGlue::SdlOglGlue()
#ifdef FIX_I810_LOD_BIAS
  : _tex_lod_bias(-1.2f)
#endif
{
//    GNASH_REPORT_FUNCTION;
}

SdlOglGlue::~SdlOglGlue()
{
//    GNASH_REPORT_FUNCTION;

}

bool SdlOglGlue::init(int /*argc*/, char ** /*argv*/ [])
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


Renderer*
SdlOglGlue::createRenderHandler(int depth)
{
//    GNASH_REPORT_FUNCTION;

    _bpp = depth;

    Renderer* renderer = create_Renderer_ogl();

#ifdef FIX_I810_LOD_BIAS
    glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, _tex_lod_bias);
#endif

    return renderer;
}

/// Not implemented, Fixme
void
SdlOglGlue::setInvalidatedRegions(const InvalidatedRanges& /*ranges*/)
{
}

bool
SdlOglGlue::prepDrawingArea(int width, int height, boost::uint32_t sdl_flags)
{
    if (_bpp == 16) {
      // 16-bit color, surface creation is likely to succeed.
      SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
      SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
      SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
      SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 15);
      SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
      SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
    } else {
      assert(_bpp == 32);

      // 32-bit color etc, for getting dest alpha,
      // for MULTIPASS_ANTIALIASING (see
      // Renderer_ogl.cpp).
      SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
      SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
      SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
    }

    SDL_SetVideoMode(width, height, _bpp, sdl_flags | SDL_OPENGL);

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

#  ifdef FIX_I810_LOD_BIAS
    glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, _tex_lod_bias);
#  endif

    return true;
}

void
SdlOglGlue::render()
{
//    GNASH_REPORT_FUNCTION;
    SDL_GL_SwapBuffers();
}



} // namespace gnash
