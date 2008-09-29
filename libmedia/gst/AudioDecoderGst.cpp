// AudioDecoderGst.cpp: Audio decoding using Gstreamer.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif


#include "AudioDecoderGst.h"
#include "MediaParser.h"

namespace gnash {
namespace media {

AudioDecoderGst::AudioDecoderGst(SoundInfo& info)
    :
        _samplerate(info.getSampleRate()),
        _stereo(info.isStereo())
{
    setup(info);
}

AudioDecoderGst::AudioDecoderGst(AudioInfo& info)
    :
        _samplerate(info.sampleRate),
        _stereo(info.stereo)
{
    setup(info);
}


AudioDecoderGst::~AudioDecoderGst()
{
    swfdec_gst_decoder_push_eos(&_decoder);
    swfdec_gst_decoder_finish(&_decoder);
}

void AudioDecoderGst::setup(AudioInfo& info)
{
	if (info.type != FLASH || info.codec != AUDIO_CODEC_MP3)
	{
	    throw MediaException("AudioDecoderGst: cannot handle this codec!");
	}

	setup();
}

void AudioDecoderGst::setup(SoundInfo& info)
{
	setup();
}


void AudioDecoderGst::setup()
{
    // init GStreamer
    gst_init (NULL, NULL);

    GstCaps* sinkcaps;
 
    GstCaps* srccaps = gst_caps_new_simple ("audio/mpeg",
		"mpegversion", G_TYPE_INT, 1,
		"layer", G_TYPE_INT, 3,
		"rate", G_TYPE_INT, _samplerate,
		"channels", G_TYPE_INT, _stereo ? 2 : 1, NULL);
    if (!srccaps) {
        throw MediaException(_("AudioDecoderGst: internal error (caps creation failed)"));      
    }

    sinkcaps = gst_caps_from_string ("audio/x-raw-int, endianness=byte_order, signed=(boolean)true, width=16, depth=16, rate=44100, channels=2");
    if (!sinkcaps) {
        throw MediaException(_("AudioDecoderGst: internal error (caps creation failed)"));      
    }

    // TODO: we may want to prefer other modules over audioresample, like ffaudioresample, if they are
    // available.
    bool rv = swfdec_gst_decoder_init (&_decoder, srccaps, sinkcaps, "audioconvert", "audioresample", NULL);
    if (!rv) {
        throw MediaException(_("AudioDecoderGst: initialisation failed."));      
    }

    gst_caps_unref (srccaps);
    gst_caps_unref (sinkcaps);
}

boost::uint8_t*
AudioDecoderGst::decode(boost::uint8_t* input, boost::uint32_t inputSize,
                        boost::uint32_t& outputSize,
                        boost::uint32_t& decodedData, bool /*parse*/)
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

    GstBuffer * decodedbuf = swfdec_gst_decoder_pull (&_decoder);

    if (!decodedbuf) {
        outputSize = 0;
        return 0;
    }

    outputSize = GST_BUFFER_SIZE(decodedbuf);
    decodedData = inputSize;

    boost::uint8_t* rbuf = new boost::uint8_t[outputSize];
    memcpy(rbuf, GST_BUFFER_DATA(decodedbuf), outputSize);
    gst_buffer_unref(decodedbuf);

    return rbuf;
}

boost::uint8_t*
AudioDecoderGst::decode(const EncodedAudioFrame& ef, boost::uint32_t& outputSize)
{
    // Docs are not very helpful as to what the difference between these two is.
    boost::uint32_t output_size = 0;    
    boost::uint32_t decoded_data_size = 0;

    uint8_t* rv = decode(ef.data.get(), ef.dataSize, output_size, decoded_data_size, false);

    // my best guess is that outputSize in one method means outputSize in the other...
    outputSize = output_size;

    return rv;
}


} // end of media namespace
} // end of gnash namespace


