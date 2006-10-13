//
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
//
//

#include "sdl_ogl_glue.h"
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

bool
SdlOglGlue::init(int argc, char** argv[])
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


render_handler*
SdlOglGlue::createRenderHandler()
{
//    GNASH_REPORT_FUNCTION;

// ogl initialisation?

    render_handler* renderer = create_render_handler_ogl();

#ifdef FIX_I810_LOD_BIAS
    glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, _tex_lod_bias);
#endif

    return renderer;
}

bool
SdlOglGlue::prepDrawingArea(int width, int height, int depth, uint32_t sdl_flags)
{
    if (depth == 16) {
      // 16-bit color, surface creation is likely to succeed.
      SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
      SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
      SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
      SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 15);
      SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
      SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
    } else {
      assert(depth == 32);

      // 32-bit color etc, for getting dest alpha,
      // for MULTIPASS_ANTIALIASING (see
      // render_handler_ogl.cpp).
      SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
      SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
      SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
    }

    SDL_SetVideoMode(width, height, depth, sdl_flags | SDL_OPENGL);

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
