// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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
// 
// Based on the filesrc and fdsrc element in Gstreamer-core.
//

/* $Id: gstgnashsrc.c,v 1.5 2007/07/01 10:54:08 bjacques Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstgnashsrc.h"

static GstStaticPadTemplate srctemplate = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY);


GST_DEBUG_CATEGORY_STATIC (gst_gnash_src_debug);
#define GST_CAT_DEFAULT gst_gnash_src_debug

static const GstElementDetails gst_gnash_src_details =
GST_ELEMENT_DETAILS ("Gnash source",
    "Gnash",
    "Use callback to read from Gnash",
    "Gnash team");

/* GnashSrc signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

#define DEFAULT_BLOCKSIZE      4*1024
#define DEFAULT_DATA           NULL
#define DEFAULT_CALLBACKS      NULL

enum
{
  ARG_0,
  ARG_DATA,
  ARG_CALLBACKS
};

static void gst_gnash_src_finalize (GObject * object);

static void gst_gnash_src_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_gnash_src_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_gnash_src_start (GstBaseSrc * basesrc);
static gboolean gst_gnash_src_stop (GstBaseSrc * basesrc);

static gboolean gst_gnash_src_is_seekable (GstBaseSrc * src);
static gboolean gst_gnash_src_get_size (GstBaseSrc * src, guint64 * size);
static GstFlowReturn gst_gnash_src_create (GstPushSrc * src, GstBuffer ** buffer);
static gboolean gst_gnash_src_do_seek (GstBaseSrc * src, GstSegment * s);

static void
_do_init (GType gnashsrc_type)
{
	UNUSEDPAR(gnashsrc_type);
/*  static const GInterfaceInfo urihandler_info = {
    gst_gnash_src_uri_handler_init,
    NULL,
    NULL
  };*/

  GST_DEBUG_CATEGORY_INIT (gst_gnash_src_debug, "gnashsrc", 0, "gnashsrc element");
}

GST_BOILERPLATE_FULL (GstGnashSrc, gst_gnash_src, GstElement, GST_TYPE_PUSH_SRC, _do_init);

static void
gst_gnash_src_base_init (gpointer g_class)
{
  GstElementClass *gstelement_class = GST_ELEMENT_CLASS (g_class);

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&srctemplate));

  gst_element_class_set_details (gstelement_class, &gst_gnash_src_details);
}

static void
gst_gnash_src_class_init (GstGnashSrcClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseSrcClass *gstbasesrc_class;
  GstPushSrcClass *gstpush_src_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gstelement_class = GST_ELEMENT_CLASS (klass);
  gstbasesrc_class = GST_BASE_SRC_CLASS (klass);
  gstpush_src_class = GST_PUSH_SRC_CLASS (klass);

  gobject_class->set_property = gst_gnash_src_set_property;
  gobject_class->get_property = gst_gnash_src_get_property;

  g_object_class_install_property (gobject_class, ARG_DATA,
      g_param_spec_pointer ("data", NULL, NULL, (GParamFlags)G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, ARG_CALLBACKS,
      g_param_spec_pointer ("callbacks", NULL, NULL, (GParamFlags)G_PARAM_READWRITE));

  gobject_class->finalize = GST_DEBUG_FUNCPTR (gst_gnash_src_finalize);

  gstbasesrc_class->start = GST_DEBUG_FUNCPTR (gst_gnash_src_start);
  gstbasesrc_class->stop = GST_DEBUG_FUNCPTR (gst_gnash_src_stop);
  gstbasesrc_class->is_seekable = GST_DEBUG_FUNCPTR (gst_gnash_src_is_seekable);
  gstbasesrc_class->get_size = GST_DEBUG_FUNCPTR (gst_gnash_src_get_size);
  gstbasesrc_class->do_seek = GST_DEBUG_FUNCPTR (gst_gnash_src_do_seek);

  gstpush_src_class->create = GST_DEBUG_FUNCPTR (gst_gnash_src_create);
}

static void
gst_gnash_src_init (GstGnashSrc * src, GstGnashSrcClass * g_class)
{
  UNUSEDPAR(g_class);
  src->data = NULL;
  src->callbacks = NULL;
  src->read_position = 0;
}

static void
gst_gnash_src_finalize (GObject * object)
{
  GstGnashSrc *src;

  src = GST_GNASH_SRC (object);

  free (src->callbacks);
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_gnash_src_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstGnashSrc *src;

  g_return_if_fail (GST_IS_GNASH_SRC (object));

  src = GST_GNASH_SRC (object);

  switch (prop_id) {
    case ARG_DATA:
      src->data = g_value_get_pointer (value);
      break;
    case ARG_CALLBACKS:
      src->callbacks = g_value_get_pointer (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_gnash_src_get_property (GObject * object, guint prop_id, GValue * value,
    GParamSpec * pspec)
{
  GstGnashSrc *src;
  UNUSEDPAR(value);
  g_return_if_fail (GST_IS_GNASH_SRC (object));

  src = GST_GNASH_SRC (object);

  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

//
// Below is the functions used get make this plugin work.
//

// Used for seeking
static gboolean
gst_gnash_src_do_seek (GstBaseSrc * basesrc, GstSegment * seg)
{

  GstGnashSrc *src;
  off_t res;
  src = GST_GNASH_SRC (basesrc);

  if (seg->format != GST_FORMAT_BYTES) return FALSE;

  struct gnashsrc_callback *gc;
  gc = src->callbacks;
  res = gc->seek (src->data, seg->start, SEEK_CUR);
  return TRUE;

}

// Output the next buffer
static GstFlowReturn
gst_gnash_src_create (GstPushSrc * psrc, GstBuffer ** outbuf)
{
  GstGnashSrc *src;
  src = GST_GNASH_SRC (psrc);
  int ret, blocksize;
  GstBuffer *buf;

  struct gnashsrc_callback *gc;
  gc = src->callbacks;

  blocksize = GST_BASE_SRC (src)->blocksize;
  buf = gst_buffer_new_and_alloc (blocksize);

  GST_LOG_OBJECT (src, "Reading %d bytes", blocksize);

  ret = gc->read(src->data, (void*)GST_BUFFER_DATA(buf), blocksize);
	
  if (G_UNLIKELY (ret < 0)) goto could_not_read;

  /* other files should eos if they read 0 and more was requested */
  if (G_UNLIKELY (ret == 0 && blocksize > 0))
    goto eos;

  blocksize = ret;

  GST_BUFFER_SIZE (buf) = blocksize;
  GST_BUFFER_OFFSET (buf) = src->read_position;
  GST_BUFFER_OFFSET_END (buf) = src->read_position + blocksize;

  *outbuf = buf;

  src->read_position += blocksize;

  return GST_FLOW_OK;

  /* ERROR */
could_not_read:
  {
    GST_ELEMENT_ERROR (src, RESOURCE, READ, (NULL), GST_ERROR_SYSTEM);
    gst_buffer_unref (buf);
    return GST_FLOW_ERROR;
  }
eos:
  {
    GST_DEBUG ("non-regular file hits EOS");
    gst_buffer_unref (buf);
    return GST_FLOW_UNEXPECTED;
  }
}

static gboolean
gst_gnash_src_is_seekable (GstBaseSrc * basesrc)
{
  GstGnashSrc *src = GST_GNASH_SRC (basesrc);

  return src->seekable;
}

// Get size of the file. Not sure how we shall handles this...
static gboolean
gst_gnash_src_get_size (GstBaseSrc * basesrc, guint64 * size)
{
  GstGnashSrc *src;

  src = GST_GNASH_SRC (basesrc);

  if (!src->seekable) {
    /* If it isn't seekable, we won't know the length (but fstat will still
     * succeed, and wrongly say our length is zero. */
    return FALSE;
  }
  return FALSE;

  // Since it's a streamed video file we probably don't know the length, so we
  // tell it's 50000. Maybe we should just return FALSE?
  *size = 500000;
  return TRUE;

}

/* open the file and mmap it, necessary to go to READY state */
static gboolean
gst_gnash_src_start (GstBaseSrc * basesrc)
{
  GstGnashSrc *src = GST_GNASH_SRC (basesrc);

  if (src->data == NULL || src->callbacks == NULL) {
    GST_ELEMENT_ERROR (src, RESOURCE, NOT_FOUND,(("No data or callback struct supplied.")), (NULL));
    return FALSE;
  }

  src->read_position = 0;
  GST_INFO_OBJECT (src, "Ready for reading using callbacks");

  // TODO: set seekable to false when real streaming
  src->seekable = TRUE;

  return TRUE;

}

/* stop and free */
static gboolean
gst_gnash_src_stop (GstBaseSrc * basesrc)
{
  GstGnashSrc *src = GST_GNASH_SRC (basesrc);
  UNUSEDPAR(src);
  return TRUE;
}

