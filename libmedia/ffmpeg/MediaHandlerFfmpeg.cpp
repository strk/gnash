// MediaHandlerFfmpeg.cpp: FFMPEG media handler, for Gnash
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


#include "MediaHandlerFfmpeg.h"

#include <sstream>

#include "MediaParser.h"
#include "MediaParserFfmpeg.h"
#include "VideoDecoderFfmpeg.h"
#include "AudioDecoderFfmpeg.h"
#include "GnashException.h"
#include "FLVParser.h"
#include "VideoConverterFfmpeg.h"
#include "VideoInputFfmpeg.h"
#include "AudioInputFfmpeg.h"
#include "IOChannel.h" 

namespace gnash { 
namespace media {
namespace ffmpeg {
    
std::string
MediaHandlerFfmpeg::description() const
{
    std::ostringstream ss;
    const boost::uint32_t ver = avcodec_version();
    ss << "FFmpeg (avcodec version: " << (ver >> 16) << "."
                      << ((ver & 0xff00) >> 8)  << "."
                      << (ver & 0xff) << ")";
    return ss.str();
}

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

std::auto_ptr<VideoConverter>
MediaHandlerFfmpeg::createVideoConverter(ImgBuf::Type4CC srcFormat,
                                         ImgBuf::Type4CC dstFormat)
{
    std::auto_ptr<VideoConverter> converter;

    try
    {
        converter.reset(new VideoConverterFfmpeg(srcFormat, dstFormat));
    }
    catch (GnashException& ex)
    {
        log_error("Could not create Ffmpeg based video converter parser for "
                "input format: %s", ex.what());
    }
    
    return converter;
}


std::auto_ptr<AudioDecoder>
MediaHandlerFfmpeg::createAudioDecoder(const AudioInfo& info)
{

	std::auto_ptr<AudioDecoder> ret;

    try {
        ret.reset(new AudioDecoderFfmpeg(info));
    }
    catch (const MediaException& ex) {

        if (info.type != CODEC_TYPE_FLASH) throw;

        try {
            ret = createFlashAudioDecoder(info);
        } 
        catch (const MediaException& ex2) {
            boost::format err = boost::format(
                _("MediaHandlerFfmpeg::createAudioDecoder: %s "
                  "-- %s")) % ex.what() % ex2.what();
            throw MediaException(err.str());
        }
    }

	return ret;
}

AudioInput*
MediaHandlerFfmpeg::getAudioInput(size_t /*index*/)
{
    return new AudioInputFfmpeg();
}

VideoInput*
MediaHandlerFfmpeg::getVideoInput(size_t /*index*/)
{
    return new VideoInputFfmpeg();
}

void
MediaHandlerFfmpeg::cameraNames(std::vector<std::string>& /*names*/) const
{
    log_unimpl("FFmpeg: camera names");
}

size_t
MediaHandlerFfmpeg::getInputPaddingSize() const
{
    return FF_INPUT_BUFFER_PADDING_SIZE;
}

} // gnash.media.ffmpeg namespace 
} // gnash.media namespace 
} // gnash namespace

