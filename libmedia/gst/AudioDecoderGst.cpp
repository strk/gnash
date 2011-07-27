// AudioDecoderGst.cpp: Audio decoding using Gstreamer.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include "AudioDecoderGst.h"
#include "MediaParser.h" // for AudioInfo
#include "MediaParserGst.h"
#include "GstUtil.h"
#include "FLVParser.h" // for ExtraAudioInfoFlv
#include "SoundInfo.h"

namespace gnash {
namespace media {
namespace gst {


AudioDecoderGst::AudioDecoderGst(SoundInfo& info)
{
    // init GStreamer. TODO: what about doing this in MediaHandlerGst ctor?
    gst_init (NULL, NULL);

    GstCaps* srccaps = gst_caps_new_simple ("audio/mpeg",
		"mpegversion", G_TYPE_INT, 1,
		"layer", G_TYPE_INT, 3,
		"rate", G_TYPE_INT, info.getSampleRate(),
		"channels", G_TYPE_INT, info.isStereo() ? 2 : 1, NULL);

    setup(srccaps);

    // FIXME: should we handle other types?
}

AudioDecoderGst::AudioDecoderGst(const AudioInfo& info)
{
    // init GStreamer. TODO: what about doing this in MediaHandlerGst ctor?
    gst_init (NULL, NULL);

    GstCaps* srccaps=0;

    if (info.type == CODEC_TYPE_FLASH && info.codec == AUDIO_CODEC_MP3)
    {
        srccaps = gst_caps_new_simple ("audio/mpeg",
		"mpegversion", G_TYPE_INT, 1,
		"layer", G_TYPE_INT, 3,
		"rate", G_TYPE_INT, info.sampleRate,
		"channels", G_TYPE_INT, info.stereo ? 2 : 1, NULL);
        setup(srccaps);
        return;
    }
    
    if (info.type == CODEC_TYPE_FLASH && info.codec == AUDIO_CODEC_NELLYMOSER)
    {
        srccaps = gst_caps_new_simple ("audio/x-nellymoser",
		"rate", G_TYPE_INT, info.sampleRate,
		"channels", G_TYPE_INT, info.stereo ? 2 : 1, NULL);
        setup(srccaps);
        return;
    }

    if (info.type == CODEC_TYPE_FLASH && info.codec == AUDIO_CODEC_ADPCM)
    {
        srccaps = gst_caps_new_simple ("audio/x-adpcm",
                "rate", G_TYPE_INT, info.sampleRate,
                "channels", G_TYPE_INT, info.stereo ? 2 : 1, 
                "layout", G_TYPE_STRING, "swf", NULL);
        setup(srccaps);
        return;
    }

    if (info.type == CODEC_TYPE_FLASH && info.codec == AUDIO_CODEC_AAC)
    {
        srccaps = gst_caps_new_simple ("audio/mpeg",
            "mpegversion", G_TYPE_INT, 4,
            "rate", G_TYPE_INT, 44100,
            "channels", G_TYPE_INT, 2, 
            NULL);

        ExtraAudioInfoFlv* extra = dynamic_cast<ExtraAudioInfoFlv*>(info.extra.get());
        if (extra) {
            GstBuffer* buf = gst_buffer_new_and_alloc(extra->size);
            memcpy(GST_BUFFER_DATA(buf), extra->data.get(), extra->size);
            gst_caps_set_simple (srccaps, "codec_data", GST_TYPE_BUFFER, buf, NULL);

        } else {
            log_error(_("Creating AAC decoder without extra data. This will probably fail!"));
        }

        setup(srccaps);
        return;
    }


    if (info.type == CODEC_TYPE_FLASH) {
		boost::format err = boost::format(
                _("AudioDecoderGst: cannot handle codec %d (%s)")) %
                info.codec %
                (audioCodecType)info.codec;
        throw MediaException(err.str());
    }

    ExtraInfoGst* extraaudioinfo = dynamic_cast<ExtraInfoGst*>(info.extra.get());

    if (!extraaudioinfo) {
		boost::format err = boost::format(
                _("AudioDecoderGst: cannot handle codec %d "
                  "(no ExtraInfoGst attached)")) %
                info.codec;
        throw MediaException(err.str());
    }

    gst_caps_ref(extraaudioinfo->caps);
    setup(extraaudioinfo->caps);
}

AudioDecoderGst::~AudioDecoderGst()
{
    assert(g_queue_is_empty (_decoder.queue));
    swfdec_gst_decoder_push_eos(&_decoder);
    swfdec_gst_decoder_finish(&_decoder);
}

/// Find the best available audio resampler on the system
static std::string 
findResampler()
{
    std::string resampler = "ffaudioresample";

    GstElementFactory* factory = gst_element_factory_find(resampler.c_str());
     
    if (!factory) {
        resampler = "speexresample";
        factory = gst_element_factory_find(resampler.c_str());
        if (!factory) {
            log_error(_("The best available resampler is 'audioresample'."
                      " Please install gstreamer-ffmpeg 0.10.4 or newer, or you"
                      " may experience long delays in audio playback!"));
            resampler = "audioresample";
        }
    }

    if (factory) {
        gst_object_unref(factory);
    }

    return resampler;
}



void AudioDecoderGst::setup(GstCaps* srccaps)
{
    if (!srccaps) {
        throw MediaException(_("AudioDecoderGst: internal error (caps creation failed)"));      
    }

    bool success = GstUtil::check_missing_plugins(srccaps);
    if (!success) {
        GstStructure* sct = gst_caps_get_structure(srccaps, 0);
        std::string type(gst_structure_get_name(sct));
        std::string msg = (boost::format(_("Couldn't find a plugin for "
                    "audio type %s!")) % type).str();

        gst_caps_unref(srccaps);

        throw MediaException(msg);
    }


    GstCaps* sinkcaps = gst_caps_from_string ("audio/x-raw-int, "
            "endianness=byte_order, signed=(boolean)true, width=16, "
            "depth=16, rate=44100, channels=2");
    if (!sinkcaps) {
        throw MediaException(_("AudioDecoderGst: internal error "
                    "(caps creation failed)"));      
    }

    std::string resampler = findResampler();

    success = swfdec_gst_decoder_init (&_decoder, srccaps, sinkcaps, "audioconvert", resampler.c_str(), NULL);
    if (!success) {
        GstStructure* sct = gst_caps_get_structure(srccaps, 0);
        std::string type(gst_structure_get_name(sct));
        std::string msg = (boost::format(
            _("AudioDecoderGst: initialisation failed for audio type %s!"))
            % type).str();
        throw MediaException(msg);
    }

    gst_caps_unref (srccaps);
    gst_caps_unref (sinkcaps);
}

static void
buf_add(gpointer buf, gpointer data)
{
    boost::uint32_t* total = (boost::uint32_t*) data;

    GstBuffer* buffer = (GstBuffer*) buf;
    *total += GST_BUFFER_SIZE(buffer);
}


/* private */
boost::uint8_t* 
AudioDecoderGst::pullBuffers(boost::uint32_t&  outputSize)
{
    outputSize = 0;
    
    g_queue_foreach(_decoder.queue, buf_add, &outputSize);
  
    if (!outputSize) {
        log_debug(_("Pushed data, but there's nothing to pull (yet)"));
        return 0;   
    }
    
    boost::uint8_t* rbuf = new boost::uint8_t[outputSize];
    
    boost::uint8_t* ptr = rbuf;
    
    while (true) {
    
        GstBuffer* buffer = swfdec_gst_decoder_pull (&_decoder);
        if (!buffer) {
            break;
        }
    
        memcpy(ptr, GST_BUFFER_DATA(buffer), GST_BUFFER_SIZE(buffer));
        ptr += GST_BUFFER_SIZE(buffer);
      
        gst_buffer_unref (buffer);
    }

    return rbuf;    
}

boost::uint8_t*
AudioDecoderGst::decode(const boost::uint8_t* input, boost::uint32_t inputSize,
                        boost::uint32_t& outputSize,
                        boost::uint32_t& decodedData)
{
    outputSize = decodedData = 0;

    GstBuffer* gstbuf = gst_buffer_new_and_alloc(inputSize);
    memcpy (GST_BUFFER_DATA (gstbuf), input, inputSize);

    bool success = swfdec_gst_decoder_push(&_decoder, gstbuf);
    if (!success) {
        log_error(_("AudioDecoderGst: buffer push failed."));
        return 0;
    }

    decodedData = inputSize;

    return pullBuffers(outputSize);
}

boost::uint8_t*
AudioDecoderGst::decode(const EncodedAudioFrame& ef, boost::uint32_t& outputSize)
{
    outputSize = 0;
    
    GstBuffer* gstbuf;
    
    EncodedExtraGstData* extradata = dynamic_cast<EncodedExtraGstData*>(ef.extradata.get());
    
    if (extradata) {
        gstbuf = extradata->buffer;
    } else {

        gstbuf = gst_buffer_new_and_alloc(ef.dataSize);
        memcpy (GST_BUFFER_DATA (gstbuf), ef.data.get(), ef.dataSize);
    }

    bool success = swfdec_gst_decoder_push(&_decoder, gstbuf);
    if (!success) {
        log_error(_("AudioDecoderGst: buffer push failed."));
        return 0;
    }

    return pullBuffers(outputSize);
}


} // gnash.media.gst namespace
} // end of media namespace
} // end of gnash namespace


