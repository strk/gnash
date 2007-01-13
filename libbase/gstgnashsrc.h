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
// 
// Based on the filesrc and fdsrc element in Gstreamer-core
//

/* $id: */

#ifndef __GST_GNASH_SRC_H__
#define __GST_GNASH_SRC_H__

#include <gst/gst.h>
#include <gst/base/gstpushsrc.h>

// Struct to contain the callback functions
struct gnashsrc_callback {
	int (*read)(void* data, char* buf, int buf_size);
	int (*seek)(void* data, int offset, int whence);
};

G_BEGIN_DECLS

#define GST_TYPE_GNASH_SRC \
  (gst_gnash_src_get_type())
#define GST_GNASH_SRC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_GNASH_SRC,GstGnashSrc))
#define GST_GNASH_SRC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_GNASH_SRC,GstGnashSrcClass))
#define GST_IS_GNASH_SRC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_GNASH_SRC))
#define GST_IS_GNASH_SRC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_GNASH_SRC))

typedef struct _GstGnashSrc GstGnashSrc;
typedef struct _GstGnashSrcClass GstGnashSrcClass;

/**
 * GstGnashSrc:
 *
 * Opaque #GstGnashSrc structure.
 */
struct _GstGnashSrc {
  GstPushSrc element;

  /*< private >*/

  guint64 read_position;	// position in the stream

  gpointer data;			// data passes with the callbacks
  gpointer callbacks;		// struct with the callbacks

  gboolean seekable;		// seekable or not

};

struct _GstGnashSrcClass {
  GstPushSrcClass parent_class;
};

GType gst_gnash_src_get_type (void);

G_END_DECLS

static gboolean
register_elements (GstPlugin *plugin)
{
  return gst_element_register (plugin, "gnashsrc", GST_RANK_NONE, GST_TYPE_GNASH_SRC);
}

static GstPluginDesc gnash_plugin_desc = {
  0, // GST_VERSION_MAJOR
  10, // GST_VERSION_MINOR
  "gnashsrc",
  "Use gnash as source via callbacks",
  register_elements,
  "0.0.1",
  "LGPL",
  "gnash",
  "gnash",
  "http://www.gnu.org/software/gnash/",
  GST_PADDING_INIT
};

#endif /* __GST_GNASH_SRC_H__ */
