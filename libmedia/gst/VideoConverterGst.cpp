//
//   Copyright (C) 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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

#include "VideoConverterGst.h"
#include "MediaParser.h"
#include "log.h"
#include <cassert>

namespace gnash {
namespace media {
namespace gst {

/// Base class for video image space conversion with gst

VideoConverterGst::VideoConverterGst(ImgBuf::Type4CC srcFormat, ImgBuf::Type4CC dstFormat)
    : VideoConverter(srcFormat, dstFormat)
{
    _decoder.bin = nullptr;

    gst_init (nullptr, nullptr);
    
    GstElementFactory* colorspacefactory = gst_element_factory_find ("ffmpegcolorspace");
    if (!colorspacefactory) {
        throw MediaException(_("VideoConverterGst: ffmpegcolorspace element missing"));
    }   

    GstCaps* caps = gst_caps_new_simple("video/x-raw-yuv",
                                        "format", GST_TYPE_FOURCC, _dst_fmt,
                                        NULL); 

    // Verify that ffmpegcolorspace can actually convert to the requested fmt.
    bool found = false;
    for (const GList* walk = gst_element_factory_get_static_pad_templates (colorspacefactory);
         walk; walk = walk->next) {
        GstStaticPadTemplate* templ = (GstStaticPadTemplate*) walk->data;

        if (templ->direction != GST_PAD_SRC) {
            continue;
        }

        GstCaps* template_caps = gst_static_caps_get (&templ->static_caps);
        GstCaps* intersect = gst_caps_intersect (caps, template_caps);   
        gst_caps_unref (template_caps);
    
        found = !gst_caps_is_empty (intersect);    
        gst_caps_unref (intersect);

        if (found) {
            break;
        }
    }

    gst_caps_unref(caps);
    gst_object_unref(colorspacefactory);

    if (!found) {
         throw MediaException(_("VideoConverterGst: can't output requested format"));
    }
}

bool
VideoConverterGst::init(const ImgBuf& src)
{
    // FIXME: mask values are probably wrong.
    GstCaps* srccaps = gst_caps_new_simple("video/x-raw-rgb",
            "bpp", G_TYPE_INT, 24,
            "depth", G_TYPE_INT, 24,
            "width", G_TYPE_INT, src.width,
            "height", G_TYPE_INT, src.height,
            "red_mask", G_TYPE_INT, 16711680,
            "green_mask", G_TYPE_INT, 65280,
            "blue_mask", G_TYPE_INT, 255,
            "endianness",G_TYPE_INT,  4321,
            "framerate", GST_TYPE_FRACTION, 0, 1,
            NULL);

    GstCaps* sinkcaps = gst_caps_new_simple("video/x-raw-yuv",
            "format", GST_TYPE_FOURCC, _dst_fmt,
            "width", G_TYPE_INT, src.width,
            "height", G_TYPE_INT, src.height,
            "framerate", GST_TYPE_FRACTION, 0, 1,
            NULL);

    if (!sinkcaps || !srccaps) {
        log_error(_("VideoConverterGst: internal error "
                    "(caps creation failed)"));    
        return false;  
    }

    bool rv = swfdec_gst_colorspace_init (&_decoder, srccaps, sinkcaps);
    if (!rv) {
        log_error(_("VideoConverterGst: initialization failed."));
        return false;
    }

    gst_caps_unref (srccaps);
    gst_caps_unref (sinkcaps);
    
    return true;
}

VideoConverterGst::~VideoConverterGst()
{
    if (_decoder.bin) {
        swfdec_gst_decoder_push_eos(&_decoder);
        swfdec_gst_decoder_finish(&_decoder);
    }
}

std::unique_ptr<ImgBuf>
VideoConverterGst::convert(const ImgBuf& src)
{
    std::unique_ptr<ImgBuf> ret;
    
    if (!init(src)) {
        return ret;  
    }

    GstBuffer* buffer = gst_buffer_new();

    GST_BUFFER_DATA(buffer) = const_cast<std::uint8_t*>(src.data);
    GST_BUFFER_SIZE(buffer) = src.size;
    GST_BUFFER_FLAG_SET(buffer, GST_BUFFER_FLAG_READONLY);

    bool success = swfdec_gst_decoder_push(&_decoder, buffer);
    if (!success) {
        log_error(_("VideoConverterGst: buffer push failed."));
        return ret;
    }
    
    GstBuffer* retbuffer = swfdec_gst_decoder_pull (&_decoder);

    if (!retbuffer) {
        log_error(_("VideoConverterGst: buffer pull failed."));
        return ret;
    }
  
    ret.reset(new ImgBuf(_dst_fmt, GST_BUFFER_DATA(retbuffer),
                         GST_BUFFER_SIZE(retbuffer), src.width, src.height));

    GST_BUFFER_MALLOCDATA(retbuffer) = nullptr; // don't free
    gst_buffer_unref(retbuffer);
    
    ret->dealloc = g_free;
  
    return ret;
}

} // gnash.media.gst namespace
} // gnash.media namespace 
} // gnash namespace

