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
// Based on the filesrc and fdsrc element in Gstreamer-core
//

/* $Id: gstgnashsrc.h,v 1.8 2007/07/01 10:54:08 bjacques Exp $ */

#ifndef __GST_GNASH_SRC_H__
#define __GST_GNASH_SRC_H__

#define UNUSEDPAR(x)  { x = x; }


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

#endif /* __GST_GNASH_SRC_H__ */
