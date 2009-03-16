/* Swfdec
 * Copyright (C) 2003-2006 David Schleef <ds@schleef.org>
 *		 2005-2006 Eric Anholt <eric@anholt.net>
 *		 2006-2007 Benjamin Otte <otte@gnome.org>
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

#ifndef __SWFDEC_CODEC_GST_H__
#define __SWFDEC_CODEC_GST_H__

#include <gst/gst.h>

G_BEGIN_DECLS

typedef struct {
  GstElement *		bin;
  GstPad *		src;
  GstPad *		sink;
  GQueue *		queue;		/* all the stored output GstBuffers */
} SwfdecGstDecoder;

gboolean	swfdec_gst_decoder_init		(SwfdecGstDecoder *	dec,
						 GstCaps *		srccaps,
						 GstCaps *		sinkcaps,
						 ...) G_GNUC_NULL_TERMINATED;

gboolean	swfdec_gst_colorspace_init	(SwfdecGstDecoder *	dec,
						 GstCaps *		srccaps,
						 GstCaps *		sinkcaps);
						 
void		swfdec_gst_decoder_push_eos	(SwfdecGstDecoder *	dec);
GstBuffer *	swfdec_gst_decoder_pull		(SwfdecGstDecoder *	dec);
gboolean	swfdec_gst_decoder_push		(SwfdecGstDecoder *	dec,
						 GstBuffer *		buffer);
void		swfdec_gst_decoder_finish	(SwfdecGstDecoder *	dec);

GstElementFactory *
		swfdec_gst_get_element_factory	(GstCaps *		caps);

GstElementFactory *
		swfdec_gst_get_demuxer_factory	(GstCaps *		caps);
GstElementFactory *
		swfdec_gst_get_parser_factory	(GstCaps *		caps);

GstPad *	swfdec_gst_connect_srcpad (GstElement *element, GstCaps *caps);

GstPad *	swfdec_gst_connect_sinkpad (GstElement *element, GstCaps *caps);
GstPad *    swfdec_gst_connect_sinkpad_by_pad (GstPad *srcpad, GstCaps *caps);

G_END_DECLS
#endif

