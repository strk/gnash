// AudioDecoderGst.h: Audio decoding using Gstreamer.
// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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

#ifndef GNASH_AUDIODECODERGST_H
#define GNASH_AUDIODECODERGST_H

#include "log.h"
#include "AudioDecoder.h"

#include <gst/gst.h>
#include "GnashImage.h"

#include "swfdec_codec_gst.h"

// Forward declarations
namespace gnash {
    namespace media {
        class AudioInfo;
        class SoundInfo;
    }
}

namespace gnash {
namespace media {
namespace gst {

/// GST based AudioDecoder
class DSOEXPORT AudioDecoderGst : public AudioDecoder {
	
public:
    AudioDecoderGst(const AudioInfo& info);
    AudioDecoderGst(SoundInfo& info);

    ~AudioDecoderGst();

    boost::uint8_t* decode(const boost::uint8_t* input, boost::uint32_t inputSize,
                           boost::uint32_t& outputSize, boost::uint32_t& decodedData,
                           bool /*parse*/);
    boost::uint8_t* decode(const EncodedAudioFrame& ef, boost::uint32_t& outputSize);

private:

    boost::uint8_t* pullBuffers(boost::uint32_t&  outputSize);

    void setup(GstCaps* caps);

    SwfdecGstDecoder _decoder;

};

} // gnash.media.gst namespace
} // media namespace
} // gnash namespace

#endif // __AUDIODECODERGST_H__

