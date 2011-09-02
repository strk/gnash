// gnash-canvas.h: Gtk canvas widget for gnash
// 
//   Copyright (C) 2009, 2010, 2011 Free Software Foundation, Inc.
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

#ifndef GNASH_GTK_CANVAS_H
#define GNASH_GTK_CANVAS_H

#include <string>
#include <gtk/gtkdrawingarea.h>
#include <boost/shared_ptr.hpp>

// Forward declarations.
namespace gnash {
    class Renderer;
    class movie_root;
}

G_BEGIN_DECLS

typedef struct _GnashCanvas            GnashCanvas;
typedef struct _GnashCanvasClass       GnashCanvasClass;

#define GNASH_TYPE_CANVAS              (gnash_canvas_get_type())
#define GNASH_CANVAS(object)           (G_TYPE_CHECK_INSTANCE_CAST((object), GNASH_TYPE_CANVAS, GnashCanvas))
#define GNASH_CANVAS_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GNASH_TYPE_CANVAS, GnashCanvasClass))
#define GNASH_IS_CANVAS(object)        (G_TYPE_CHECK_INSTANCE_TYPE((object), GNASH_TYPE_CANVAS))
#define GNASH_IS_CANVAS_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE((klass), GNASH_TYPE_CANVAS))
#define GNASH_CANVAS_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS((object), GNASH_TYPE_CANVAS, GnashCanvasClass))

struct _GnashCanvasClass {
    GtkDrawingAreaClass base_class;
};

GType gnash_canvas_get_type();

/// allocate memory for canvas to draw in
GtkWidget *gnash_canvas_new();

/// Select renderer and hwaccel, prep canvas for drawing
void gnash_canvas_setup (GnashCanvas *canvas, std::string &hwaccel,
                         std::string &renderer, int argc, char **argv[]);

void gnash_canvas_before_rendering (GnashCanvas *canvas, gnash::movie_root* stage);

/// Get the Renderer for this canvas
boost::shared_ptr<gnash::Renderer> gnash_canvas_get_renderer(GnashCanvas *canvas);

G_END_DECLS

#endif

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:

