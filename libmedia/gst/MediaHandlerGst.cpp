// MediaHandlerGst.cpp: GST media handler, for Gnash
// 
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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


#include "MediaHandlerGst.h"
#include "VideoDecoderGst.h"
#include "AudioDecoderGst.h"
#include "MediaParserGst.h"
#include "FLVParser.h"

#ifdef DECODING_SPEEX
#include "AudioDecoderSpeex.h"
#endif

#include "IOChannel.h" // for visibility of destructor
#include "MediaParser.h" // for visibility of destructor

#include "MediaParserGst.h"


namespace gnash { 
namespace media {
namespace gst {

std::auto_ptr<MediaParser>
MediaHandlerGst::createMediaParser(std::auto_ptr<IOChannel> stream)
{
	std::auto_ptr<MediaParser> parser;

	if ( isFLV(*stream) )
	{
		parser.reset( new FLVParser(stream) );
	}
	else
	{
		try
		{
			parser.reset(new MediaParserGst(stream));
		}
		catch (GnashException& ex)
		{
			log_error("Could not create Gstreamer based media parser for "
                    "input stream: %s", ex.what());
			assert(!parser.get());
		}
	}

	return parser;
}

std::auto_ptr<VideoDecoder>
MediaHandlerGst::createVideoDecoder(const VideoInfo& info)
{
	if ( info.type != FLASH )
	{
		ExtraInfoGst* extrainfo = dynamic_cast<ExtraInfoGst*>(info.extra.get());
		if (!extrainfo) {
			log_error(_("Wrong arguments given to GST VideoDecoder"));
			return std::auto_ptr<VideoDecoder>(0);
		}
		return std::auto_ptr<VideoDecoder>(new VideoDecoderGst(extrainfo->caps));
	}
	videoCodecType format = static_cast<videoCodecType>(info.codec);
	int width = info.width;
	int height = info.height;

	boost::uint8_t* extradata = 0;
	size_t datasize = 0;

	ExtraVideoInfoFlv* extrainfo = dynamic_cast<ExtraVideoInfoFlv*>(info.extra.get());
	if (extrainfo) {
		extradata = extrainfo->data.get();
                datasize = extrainfo->size;
	}

	std::auto_ptr<VideoDecoder> ret( new VideoDecoderGst(format, width, height, extradata, datasize) );
	return ret;
}

std::auto_ptr<AudioDecoder>
MediaHandlerGst::createAudioDecoder(const AudioInfo& info)
{
        AudioDecoder* decoder = 0;
#ifdef DECODING_SPEEX
	if (info.codec == AUDIO_CODEC_SPEEX) {
		assert(info.type == FLASH);
		decoder = new AudioDecoderSpeex;
	} else
#endif
	{
		decoder = new AudioDecoderGst(info);
	}

	return std::auto_ptr<AudioDecoder>(decoder);
}

} // gnash.media.gst namespace
} // gnash.media namespace 
} // gnash namespace
