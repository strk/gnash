/* Swfdec
 * Copyright (C) 2007 Benjamin Otte <otte@gnome.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <string.h>

#include "swfdec_codec_gst.h"

#define SWFDEC_ERROR printf

#if 0
/*** BUFFER ***/

GstBuffer *
swfdec_gst_buffer_new (SwfdecBuffer *buffer)
{
  /* FIXME: make this a zero-copy operation */
  GstBuffer *ret;

  g_return_val_if_fail (buffer != NULL , NULL);
  
  ret = gst_buffer_new_and_alloc (buffer->length);
  memcpy (GST_BUFFER_DATA (ret), buffer->data, buffer->length);
  swfdec_buffer_unref (buffer);

  return ret;
}
#endif

/*** TYPEFINDING ***/

/* NB: try to mirror decodebin behavior */
static gboolean
swfdec_gst_feature_filter (GstPluginFeature *feature, gpointer caps, const gchar* klassname, gboolean autoplugonly)
{
  const GList *walk;
  const gchar *klass;

  /* we only care about element factories */
  if (!GST_IS_ELEMENT_FACTORY (feature))
    return FALSE;


  /* only decoders are interesting */
  klass = gst_element_factory_get_klass (GST_ELEMENT_FACTORY (feature));
  if (strstr (klass, klassname) == NULL)
    return FALSE;


  /* only select elements with autoplugging rank */
  if (autoplugonly && gst_plugin_feature_get_rank (feature) < GST_RANK_MARGINAL)
    return FALSE;

  /* only care about the right sink caps */
  for (walk = gst_element_factory_get_static_pad_templates (GST_ELEMENT_FACTORY (feature));
       walk; walk = walk->next) {
    GstStaticPadTemplate *template = walk->data;
    GstCaps *intersect;
    GstCaps *template_caps;

    if (template->direction != GST_PAD_SINK)
      continue;

    template_caps = gst_static_caps_get (&template->static_caps);
    intersect = gst_caps_intersect (caps, template_caps);
    
    gst_caps_unref (template_caps);
    if (gst_caps_is_empty (intersect)) {
      gst_caps_unref (intersect);
    } else {
      gst_caps_unref (intersect);
      return TRUE;
    }
  }
  return FALSE;
}

static gboolean
swfdec_gst_feature_filter_decoder (GstPluginFeature *feature, gpointer caps)
{
    return swfdec_gst_feature_filter (feature, caps, "Decoder", TRUE);
}

static gboolean
swfdec_gst_feature_filter_demuxer (GstPluginFeature *feature, gpointer caps)
{
    return swfdec_gst_feature_filter (feature, caps, "Demuxer", TRUE);
}

static gboolean
swfdec_gst_feature_filter_parser (GstPluginFeature *feature, gpointer caps)
{
    return swfdec_gst_feature_filter (feature, caps, "Parser", FALSE);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
static int
swfdec_gst_compare_features (gconstpointer a_, gconstpointer b_)
{
  int diff;
  GstPluginFeature *a = (GstPluginFeature *)GST_PLUGIN_FEATURE (a_);
  GstPluginFeature *b = (GstPluginFeature *)GST_PLUGIN_FEATURE (b_);

  diff = gst_plugin_feature_get_rank (b) - gst_plugin_feature_get_rank (a);
  if (diff != 0)
    return diff;

  return strcmp (gst_plugin_feature_get_name (a), gst_plugin_feature_get_name (b));
}
#pragma GCC diagnostic pop

static GstElementFactory *
_swfdec_gst_get_factory (GstCaps *caps, GstPluginFeatureFilter filter)
{
  GstElementFactory *ret;
  GList *list;

  list = gst_registry_feature_filter (gst_registry_get_default (), 
      filter, FALSE, caps);
  if (list == NULL)
    return NULL;

  list = g_list_sort (list, swfdec_gst_compare_features);
  ret = list->data;
  gst_object_ref (ret);
  gst_plugin_feature_list_free (list);
  return ret;
}

GstElementFactory *
swfdec_gst_get_element_factory (GstCaps *caps)
{
  return _swfdec_gst_get_factory (caps, swfdec_gst_feature_filter_decoder);
}

GstElementFactory *
swfdec_gst_get_demuxer_factory (GstCaps *caps)
{
  return  _swfdec_gst_get_factory (caps, swfdec_gst_feature_filter_demuxer);
}

GstElementFactory *
swfdec_gst_get_parser_factory (GstCaps *caps)
{
  return  _swfdec_gst_get_factory (caps, swfdec_gst_feature_filter_parser);
}


/*** PADS ***/

/* static */ GstPad *
swfdec_gst_connect_srcpad (GstElement *element, GstCaps *caps)
{
  GstPadTemplate *tmpl;
  GstPad *srcpad, *sinkpad;

  sinkpad = gst_element_get_pad (element, "sink");
  if (sinkpad == NULL)
    return NULL;
  gst_caps_ref (caps);
  tmpl = gst_pad_template_new ("src", GST_PAD_SRC, GST_PAD_ALWAYS, caps);
  srcpad = gst_pad_new_from_template (tmpl, "src");
  g_object_unref (tmpl);
  if (gst_pad_link (srcpad, sinkpad) != GST_PAD_LINK_OK)
    goto error;
  
  gst_object_unref (sinkpad);
  gst_pad_set_active (srcpad, TRUE);
  return srcpad;

error:
  SWFDEC_ERROR ("failed to create or link srcpad");
  gst_object_unref (sinkpad);
  gst_object_unref (srcpad);
  return NULL;
}

/*static*/ GstPad *
swfdec_gst_connect_sinkpad_by_pad (GstPad *srcpad, GstCaps *caps)
{
  GstPadTemplate *tmpl;
  GstPad *sinkpad;

  gst_caps_ref (caps);
  tmpl = gst_pad_template_new ("sink", GST_PAD_SINK, GST_PAD_ALWAYS, caps);
  sinkpad = gst_pad_new_from_template (tmpl, "sink");
  g_object_unref (tmpl);
  if (gst_pad_link (srcpad, sinkpad) != GST_PAD_LINK_OK)
    goto error;

  gst_pad_set_active (sinkpad, TRUE);
  return sinkpad;

error:
  SWFDEC_ERROR ("failed to create or link sinkpad");
  gst_object_unref (sinkpad);
  return NULL;
}

/*static*/ GstPad *
swfdec_gst_connect_sinkpad (GstElement *element, GstCaps *caps)
{
  GstPad* srcpad;
  srcpad = gst_element_get_pad (element, "src");

  if (srcpad == NULL)
    return NULL;

  GstPad* sinkpad = swfdec_gst_connect_sinkpad_by_pad (srcpad, caps);
  
  gst_object_unref (srcpad);
  
  return sinkpad;
}

/*** DECODER ***/

static GstFlowReturn
swfdec_gst_chain_func (GstPad *pad, GstBuffer *buffer)
{
  GQueue *queue = g_object_get_data (G_OBJECT (pad), "swfdec-queue");

  g_queue_push_tail (queue, buffer);

  return GST_FLOW_OK;
}

gboolean
swfdec_gst_colorspace_init (SwfdecGstDecoder *dec, GstCaps *srccaps, GstCaps *sinkcaps)
{
  GstElement *converter;

  dec->bin = gst_bin_new ("bin");

  converter = gst_element_factory_make ("ffmpegcolorspace", NULL);
  if (converter == NULL) {
    SWFDEC_ERROR ("failed to create converter");
    return FALSE;
  }
  gst_bin_add (GST_BIN (dec->bin), converter);
  dec->src = swfdec_gst_connect_srcpad (converter, srccaps);
  if (dec->src == NULL)
    return FALSE;

  dec->sink = swfdec_gst_connect_sinkpad (converter, sinkcaps);
  if (dec->sink == NULL)
    return FALSE;
  gst_pad_set_chain_function (dec->sink, swfdec_gst_chain_func);
  dec->queue = g_queue_new ();
  g_object_set_data (G_OBJECT (dec->sink), "swfdec-queue", dec->queue);
  if (!gst_element_set_state (dec->bin, GST_STATE_PLAYING) == GST_STATE_CHANGE_SUCCESS) {
    SWFDEC_ERROR ("could not change element state");
    return FALSE;
  }
  return TRUE;
}


gboolean
swfdec_gst_decoder_init (SwfdecGstDecoder *dec, GstCaps *srccaps, GstCaps *sinkcaps, ...)
{
  va_list args;
  GstElementFactory *factory;
  GstElement *decoder;
  const char *name;
  
  /* create decoder */
  factory = swfdec_gst_get_element_factory (srccaps);
  dec->bin = gst_bin_new ("bin");
  if (factory) {
    decoder = gst_element_factory_create (factory, "decoder");
    gst_object_unref (factory);
  } else {
    decoder = NULL;
  }
  if (decoder == NULL) {
    SWFDEC_ERROR ("failed to create decoder");
    return FALSE;
  }
  gst_bin_add (GST_BIN (dec->bin), decoder);
  dec->src = swfdec_gst_connect_srcpad (decoder, srccaps);
  if (dec->src == NULL)
    return FALSE;

  /* plug transform elements */
  va_start (args, sinkcaps);
  while ((name = va_arg (args, const char *))) {
    GstElement *next = gst_element_factory_make (name, NULL);
    if (next == NULL) {
      SWFDEC_ERROR ("failed to create '%s' element", name);
      va_end (args);
      return FALSE;
    }
    gst_bin_add (GST_BIN (dec->bin), next);
    if (!gst_element_link (decoder, next)) {
      SWFDEC_ERROR ("failed to link '%s' element to decoder", name);
      va_end (args);
      return FALSE;
    }
    decoder = next;
  }
  va_end (args);
  dec->sink = swfdec_gst_connect_sinkpad (decoder, sinkcaps);
  if (dec->sink == NULL)
    return FALSE;
  gst_pad_set_chain_function (dec->sink, swfdec_gst_chain_func);
  dec->queue = g_queue_new ();
  g_object_set_data (G_OBJECT (dec->sink), "swfdec-queue", dec->queue);
  if (!gst_element_set_state (dec->bin, GST_STATE_PLAYING) == GST_STATE_CHANGE_SUCCESS) {
    SWFDEC_ERROR ("could not change element state");
    return FALSE;
  }
  return TRUE;
}

void
swfdec_gst_decoder_finish (SwfdecGstDecoder *dec)
{
  if (dec->bin) {
    gst_element_set_state (dec->bin, GST_STATE_NULL);
    g_object_unref (dec->bin);
    dec->bin = NULL;
  }
  if (dec->src) {
    g_object_unref (dec->src);
    dec->src = NULL;
  }
  if (dec->sink) {
    g_object_unref (dec->sink);
    dec->sink = NULL;
  }
  if (dec->queue) {
    GstBuffer *buffer;
    while ((buffer = g_queue_pop_head (dec->queue)) != NULL) {
      gst_buffer_unref (buffer);
    }
    g_queue_free (dec->queue);
    dec->queue = NULL;
  }
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
gboolean
swfdec_gst_decoder_push (SwfdecGstDecoder *dec, GstBuffer *buffer)
{
  GstFlowReturn ret;
  GstCaps *caps;

  /* set caps if none set yet */
  caps = gst_buffer_get_caps (buffer);
  if (caps) {
    gst_caps_unref (caps);
  } else {
    caps = GST_PAD_CAPS (dec->src);
    if (caps == NULL) {
      caps = (GstCaps *) gst_pad_get_pad_template_caps (dec->src);
      g_assert (gst_caps_is_fixed (caps));
      gst_pad_set_caps (dec->src, caps);
    }
    gst_buffer_set_caps (buffer, GST_PAD_CAPS (dec->src));
  }

  ret = gst_pad_push (dec->src, buffer);
  if (GST_FLOW_IS_SUCCESS (ret))
    return TRUE;
  SWFDEC_ERROR ("error %d pushing data", (int) ret);
  return FALSE;
}
#pragma GCC diagnostic pop

void
swfdec_gst_decoder_push_eos (SwfdecGstDecoder *dec)
{
  gst_pad_push_event (dec->src, gst_event_new_eos ());
}

GstBuffer *
swfdec_gst_decoder_pull (SwfdecGstDecoder *dec)
{
  return g_queue_pop_head (dec->queue);
}

