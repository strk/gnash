// MediaHandlerHaiku.cpp: Haiku media kit media handler, for Gnash
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

#include "MediaHandlerHaiku.h"
#include "MediaParser.h"

#include "log.h"
#include "MediaParserHaiku.h"
#include "VideoDecoderHaiku.h"
#include "AudioDecoderHaiku.h"
#include "GnashException.h"
#include "FLVParser.h"
#include "VideoConverterHaiku.h"
#include "VideoInputHaiku.h"
#include "AudioInputHaiku.h"

#include "adipe.h"

#include "IOChannel.h" // for visibility of destructor

namespace gnash { 
namespace media {
namespace haiku {


std::unique_ptr<MediaParser>
MediaHandlerHaiku::createMediaParser(std::unique_ptr<IOChannel> stream)
{
	std::unique_ptr<MediaParser> parser;

    try {
        if (isFLV(*stream))
        {
            parser.reset(new FLVParser(stream));
        }
        else
        {
			parser.reset(new MediaParserHaiku(stream));
		}
    }
    catch (GnashException& ex)
    {
        log_error("Could not create Haiku media parser for "
                "input stream: %s", ex.what());
        assert(!parser.get());
    }

	return parser;
}

std::unique_ptr<VideoDecoder>
MediaHandlerHaiku::createVideoDecoder(const VideoInfo& info)
{
	std::unique_ptr<VideoDecoder> ret(new VideoDecoderHaiku(info));
	return ret;
}

std::unique_ptr<VideoConverter>
MediaHandlerHaiku::createVideoConverter(ImgBuf::Type4CC srcFormat,
                                         ImgBuf::Type4CC dstFormat)
{
    std::unique_ptr<VideoConverter> converter;

    try
    {
        converter.reset(new VideoConverterHaiku(srcFormat, dstFormat));
    }
    catch (GnashException& ex)
    {
        log_error("Could not create Haiku based video converter parser for "
                "input format: %s", ex.what());
    }
    
    return converter;
}


std::unique_ptr<AudioDecoder>
MediaHandlerHaiku::createAudioDecoder(const AudioInfo& info)
{
	std::unique_ptr<AudioDecoder> ret;

    try
    {
        ret.reset(new AudioDecoderHaiku(info));
    }
    catch (MediaException& ex)
    {
        if ( info.type != CODEC_TYPE_FLASH ) throw ex;

        try
        {
            ret = createFlashAudioDecoder(info);
        } 
        catch (MediaException& ex2)
        {
            boost::format err = boost::format(
                _("MediaHandlerHaiku::createAudioDecoder: %s "
                  "-- %s")) %
                ex.what() % ex2.what();
            throw MediaException(err.str());
        }
    }

	return ret;
}

AudioInput*
MediaHandlerHaiku::getAudioInput(size_t /*index*/)
{
    return new AudioInputHaiku();
}

VideoInput*
MediaHandlerHaiku::getVideoInput(size_t /*index*/)
{
    return new VideoInputHaiku();
}

void
MediaHandlerHaiku::cameraNames(std::vector<std::string>& /*names*/) const
{
    QQ(2);
    log_unimpl("Haiku: camera names");
}

//size_t
//MediaHandlerHaiku::getInputPaddingSize() const
//{
//    QQ(2);
////    return FF_INPUT_BUFFER_PADDING_SIZE;
//}

#ifdef REGISTER_MEDIA_HANDLERS
namespace {
    MediaFactory::RegisterHandler<MediaHandlerHaiku> reg("haiku");
}
#endif

} // gnash.media.haiku namespace 
} // gnash.media namespace 
} // gnash namespace
