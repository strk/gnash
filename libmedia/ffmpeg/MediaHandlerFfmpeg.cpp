// MediaHandlerFfmpeg.cpp: FFMPEG media handler, for Gnash
// 
//   Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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


#include "MediaHandlerFfmpeg.h"
#include "MediaParserFfmpeg.h"
#include "VideoDecoderFfmpeg.h"
#include "AudioDecoderFfmpeg.h"
#include "GnashException.h"
#include "FLVParser.h"

#include "IOChannel.h" // for visibility of destructor
#include "MediaParser.h" // for visibility of destructor

namespace gnash { 
namespace media {
namespace ffmpeg {

std::auto_ptr<MediaParser>
MediaHandlerFfmpeg::createMediaParser(std::auto_ptr<IOChannel> stream)
{
	std::auto_ptr<MediaParser> parser;

    try {
        if (isFLV(*stream))
        {
            parser.reset(new FLVParser(stream));
        }
        else
        {
			parser.reset(new MediaParserFfmpeg(stream));
		}
    }
    catch (GnashException& ex)
    {
        log_error("Could not create FFMPEG based media parser for "
                "input stream: %s", ex.what());
        assert(!parser.get());
    }

	return parser;
}

std::auto_ptr<VideoDecoder>
MediaHandlerFfmpeg::createVideoDecoder(const VideoInfo& info)
{
	std::auto_ptr<VideoDecoder> ret(new VideoDecoderFfmpeg(info));
	return ret;
}

std::auto_ptr<AudioDecoder>
MediaHandlerFfmpeg::createAudioDecoder(const AudioInfo& info)
{

	std::auto_ptr<AudioDecoder> ret;

    try
    {
        ret.reset(new AudioDecoderFfmpeg(info));
    }
    catch (MediaException& ex)
    {
        if ( info.type != FLASH ) throw ex;

        try
        {
            ret = createFlashAudioDecoder(info);
        } 
        catch (MediaException& ex2)
        {
            boost::format err = boost::format(
                _("MediaHandlerFfmpeg::createAudioDecoder: %s "
                  "-- %s")) %
                ex.what() % ex2.what();
            throw MediaException(err.str());
        }
    }

	return ret;
}

size_t
MediaHandlerFfmpeg::getInputPaddingSize() const
{
    return FF_INPUT_BUFFER_PADDING_SIZE;
}

} // gnash.media.ffmpeg namespace 
} // gnash.media namespace 
} // gnash namespace
