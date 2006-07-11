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

#include "gtk_glue_gtkglext.h"
#include "log.h"

const float OVERSIZE = 1.0f;

using namespace std;


namespace gnash
{

GtkGlExtGlue::GtkGlExtGlue()
#ifdef FIX_I810_LOD_BIAS
  : tex_lod_bias(-1.2f)
#endif
{
//    GNASH_REPORT_FUNCTION;
}

GtkGlExtGlue::~GtkGlExtGlue()
{
//    GNASH_REPORT_FUNCTION;
    if (_glconfig) {
        g_object_unref (G_OBJECT (_glconfig));
        _glconfig = NULL;
    }
    
    GdkGLContext *glcontext = gtk_widget_get_gl_context (_drawing_area);
    if (glcontext) {
       g_object_unref (G_OBJECT (glcontext));
       glcontext = NULL;
    }
}

bool
GtkGlExtGlue::init(int argc, char **argv[])
{
//    GNASH_REPORT_FUNCTION;
#ifdef FIX_I810_LOD_BIAS
    int c = getopt (argc, argv, "m:");
    if (c == 'm') {
      _tex_lod_bias = (float) atof(optarg);
    }
#endif

    gtk_gl_init (&argc, argv);

    gint major, minor;
    gdk_gl_query_version (&major, &minor);
    dbglogfile << "OpenGL extension version - "
              << (int)major << "." << (int)minor << endl;

    GdkGLConfigMode glcmode = (GdkGLConfigMode)(GDK_GL_MODE_RGB |
                                                GDK_GL_MODE_DEPTH |
                                                GDK_GL_MODE_DOUBLE);
    _glconfig = gdk_gl_config_new_by_mode (glcmode);

    if (!_glconfig) {
      dbglogfile << "Cannot find the double-buffered visual." << endl;
      dbglogfile << "Trying single-buffered visual." << endl;

      glcmode = (GdkGLConfigMode)(GDK_GL_MODE_RGB | GDK_GL_MODE_DEPTH);
      _glconfig = gdk_gl_config_new_by_mode (glcmode);
      if (!_glconfig) {
        dbglogfile << "No appropriate OpenGL-capable visual found." << endl;
        gtk_main_quit(); // XXX
      } else {
        dbglogfile << "Got single-buffered visual." << endl;
      }
    } else {
      dbglogfile << "Got double-buffered visual." << endl;
    }
    
    return true;
}


void
GtkGlExtGlue::prepDrawingArea(GtkWidget *drawing_area)
{
//    GNASH_REPORT_FUNCTION;
    _drawing_area = drawing_area;
    gtk_widget_set_gl_capability(_drawing_area, _glconfig,
                                 NULL, TRUE, GDK_GL_RGBA_TYPE);
}

render_handler*
GtkGlExtGlue::createRenderHandler()
{
//    GNASH_REPORT_FUNCTION;
    GdkGLContext *glcontext = gtk_widget_get_gl_context (_drawing_area);
    GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (_drawing_area);

    // Attach our OpenGL context to the drawing_area.
    gdk_gl_drawable_make_current(gldrawable, glcontext);

    render_handler* renderer = create_render_handler_ogl();

#ifdef FIX_I810_LOD_BIAS
    glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, _tex_lod_bias);
#endif

    return renderer;
}

void
GtkGlExtGlue::render()
{
//    GNASH_REPORT_FUNCTION;
    GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (_drawing_area);
    if (gdk_gl_drawable_is_double_buffered (gldrawable)) {
        gdk_gl_drawable_swap_buffers (gldrawable);
    } else {
      glFlush();
    }
}

void
GtkGlExtGlue::configure(GtkWidget *const widget, GdkEventConfigure *const event)
{
    GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
    GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);
    if (gdk_gl_drawable_make_current(gldrawable, glcontext)) {
        glViewport (event->x, event->y, event->width, event->height);
    }
}


} // namespace gnash
