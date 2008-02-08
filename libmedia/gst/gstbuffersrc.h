// 
//   Copyright (C) 2008 Free Software Foundation, Inc.
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// This file incorporates work covered by the following copyright and permission
// notice:

/* GStreamer
 * Copyright (C) 2007 David Schleef <ds@schleef.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
 
#ifndef _GNASH_BUFFER_SRC_H_
#define _GNASH_BUFFER_SRC_H_

#define UNUSEDPAR(x)  { x = x; }

#include <gst/gst.h>
#include <gst/base/gstbasesrc.h>

G_BEGIN_DECLS

#define GST_TYPE_BUFFER_SRC \
  (gst_buffer_src_get_type())
#define GST_BUFFER_SRC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_BUFFER_SRC,GstBufferSrc))
#define GST_BUFFER_SRC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_BUFFER_SRC,GstBufferSrcClass))
#define GST_IS_BUFFER_SRC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_BUFFER_SRC))
#define GST_IS_BUFFER_SRC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_BUFFER_SRC))

typedef struct _GstBufferSrc GstBufferSrc;
typedef struct _GstBufferSrcClass GstBufferSrcClass;

struct _GstBufferSrc
{
  GstBaseSrc basesrc;

  /*< private >*/
  GCond *cond;
  GMutex *mutex;
  GQueue *queue;
  GstCaps *caps;
  gboolean end_of_stream;
  gboolean flush;
  guint64 total_size;
};

struct _GstBufferSrcClass
{
  GstBaseSrcClass basesrc_class;
};

GType gst_buffer_src_get_type(void);

GST_DEBUG_CATEGORY_EXTERN (buffer_src_debug);


void gst_buffer_src_push_buffer_unowned (GstBufferSrc *buffersrc, GstBuffer *buffer);
void gst_buffer_src_set_caps (GstBufferSrc *buffersrc, GstCaps *caps);
void gst_buffer_src_end_of_stream (GstBufferSrc *buffersrc);
void gst_buffer_src_flush (GstBufferSrc * buffersrc);

G_END_DECLS

#endif

