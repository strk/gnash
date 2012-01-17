// MediaHandler.cpp:  Default MediaHandler implementation, for Gnash.
//
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
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
//


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "MediaHandler.h"
#include "FLVParser.h"
#include "IOChannel.h"
#include "AudioDecoderSimple.h"
#include "log.h"

#ifdef DECODING_SPEEX
# include "AudioDecoderSpeex.h"
#endif

namespace gnash {
namespace media {

bool
MediaHandler::isFLV(IOChannel& stream)
{
    char head[4] = {0, 0, 0, 0};
    stream.seek(0);
    size_t actuallyRead = stream.read(head, 3);
    stream.seek(0);

    if (actuallyRead < 3)
    {
        throw IOException(_("MediaHandler::isFLV: Could not read 3 bytes "
                    "from input stream"));
    }

    if (!std::equal(head, head + 3, "FLV")) return false;
    return true;
}

std::auto_ptr<MediaParser>
MediaHandler::createMediaParser(std::auto_ptr<IOChannel> stream)
{
    std::auto_ptr<MediaParser> parser;

    try {
        if (!isFLV(*stream))
        {
            log_error(_("MediaHandler::createMediaParser: only FLV input is "
                        "supported by this MediaHandler"));
            return parser;
        }
    }
    catch (IOException& m) {
        log_error(_("Exception while reading from stream: %s"), m.what());
        return parser;
    }

    parser.reset( new FLVParser(stream) );
    assert(!stream.get()); // TODO: when ownership will be transferred...

    return parser;
}

std::auto_ptr<AudioDecoder>
MediaHandler::createFlashAudioDecoder(const AudioInfo& info)
{
    assert (info.type == CODEC_TYPE_FLASH );

    audioCodecType codec = static_cast<audioCodecType>(info.codec);
    switch (codec)
    {
        case media::AUDIO_CODEC_ADPCM:
        case media::AUDIO_CODEC_RAW:
        case media::AUDIO_CODEC_UNCOMPRESSED:
        {
            std::auto_ptr<AudioDecoder> ret(new AudioDecoderSimple(info));
            return ret;
        }

#ifdef DECODING_SPEEX
        case AUDIO_CODEC_SPEEX:
        {
            std::auto_ptr<AudioDecoder> ret(new AudioDecoderSpeex);
            return ret;
        }
#endif

        default:
        {
            boost::format err = boost::format(
                _("MediaHandler::createFlashAudioDecoder:"
                  " no available flash decoders for codec %d (%s)")) %
                (int)codec % codec;
            throw MediaException(err.str());
        }
    }
}

} // namespace media 
} // namespace gnash

/// Here follows handler registration code.

#ifdef ENABLE_FFMPEG_MEDIA
#include "ffmpeg/MediaHandlerFfmpeg.h"
#endif
#ifdef ENABLE_GST_MEDIA
#include "gst/MediaHandlerGst.h"
#endif
#ifdef ENABLE_HAIKU_MEDIA
#include "haiku/MediaHandlerHaiku.h"
#endif

namespace gnash {
namespace media {

RegisterAllHandlers::RegisterAllHandlers()
{
#ifdef ENABLE_FFMPEG_MEDIA
    static const MediaFactory::RegisterHandler<ffmpeg::MediaHandlerFfmpeg>
        ffmpeg("ffmpeg");
#endif
#ifdef ENABLE_GST_MEDIA
    static const MediaFactory::RegisterHandler<gst::MediaHandlerGst> gst("gst");
#endif
#ifdef ENABLE_HAIKU_MEDIA
    static const MediaFactory::RegisterHandler<haiku::MediaHandlerHaiku>
        haiku("haiku");
#endif
}

} // namespace media
} // namespace gnash
