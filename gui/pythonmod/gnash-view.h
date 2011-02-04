// gnash-view.h: Gtk view widget for gnash
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

#ifndef __GNASH_VIEW_H__
#define __GNASH_VIEW_H__

#include <gtk/gtkbin.h>

G_BEGIN_DECLS

typedef struct _GnashView            GnashView;
typedef struct _GnashViewClass       GnashViewClass;

#define GNASH_TYPE_VIEW              (gnash_view_get_type())
#define GNASH_VIEW(object)           (G_TYPE_CHECK_INSTANCE_CAST((object), GNASH_TYPE_VIEW, GnashView))
#define GNASH_VIEW_CLASS(klass)	     (G_TYPE_CHECK_CLASS_CAST((klass), GNASH_TYPE_VIEW, GnashViewClass))
#define GNASH_IS_VIEW(object)        (G_TYPE_CHECK_INSTANCE_TYPE((object), GNASH_TYPE_VIEW))
#define GNASH_IS_VIEW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE((klass), GNASH_TYPE_VIEW))
#define GNASH_VIEW_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS((object), GNASH_TYPE_VIEW, GnashViewClass))

struct _GnashViewClass {
    GtkBinClass base_class;
};

GType        gnash_view_get_type    (void);
GtkWidget   *gnash_view_new         (void);
const gchar *gnash_view_call        (GnashView *view, const gchar *func_name, const gchar *input_data);

G_END_DECLS

#endif

